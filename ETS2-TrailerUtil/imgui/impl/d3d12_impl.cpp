// https://github.com/chadlrnsn/ImGui-DirectX-12-Kiero-Hook/blob/main/dll/src/hooks/d3d12hook.cpp

#include "../../kiero/kiero.h"

#if KIERO_INCLUDE_D3D12

#include "d3d12_impl.h"

#include <d3d12.h>
#include <dxgi1_4.h>

#include "../imgui.h"
#include "../imgui_impl_win32.h"
#include "../imgui_impl_dx12.h"
#include "win32_impl.h"


HWND window;

struct FrameContext
{
    ID3D12CommandAllocator* CommandAllocator;
    UINT64 FenceValue; // In imgui original code // i didn't use it
    ID3D12Resource* g_mainRenderTargetResource = {};
    D3D12_CPU_DESCRIPTOR_HANDLE g_mainRenderTargetDescriptor = {};
};

static int const NUM_FRAMES_IN_FLIGHT = 3;
FrameContext* g_frameContext;
static UINT g_frameIndex = 0;
static UINT g_fenceValue = 0;

static int NUM_BACK_BUFFERS = -1;
static ID3D12CommandQueue* g_pd3dCommandQueue = nullptr;
static ID3D12Fence* g_fence = nullptr;
static ID3D12Device* g_pd3dDevice = nullptr;
static ID3D12DescriptorHeap* g_pd3dSrvDescHeap = nullptr;
static ID3D12DescriptorHeap* g_pd3dRtvDescHeap = nullptr;
static ID3D12GraphicsCommandList* g_pd3dCommandList = nullptr;
static HANDLE g_fenceEvent = nullptr;
static HANDLE g_hSwapChainWaitableObject = nullptr;
static IDXGISwapChain3* g_pSwapChain = nullptr;
static UINT g_ResizeWidth = 0, g_ResizeHeight = 0;

typedef void(__stdcall* ExecuteCommandListsFunc)(ID3D12CommandQueue* pCommandQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists);
ExecuteCommandListsFunc oExecuteCommandLists = nullptr;

typedef HRESULT(__stdcall* SignalFunc)(ID3D12CommandQueue* queue, ID3D12Fence* fence, UINT64 value);
SignalFunc oSignal = nullptr;

typedef HRESULT(__stdcall* PresentFunc)(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
PresentFunc oPresent = nullptr;

typedef HRESULT(__stdcall* ResizeBuffers)(IDXGISwapChain3* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
ResizeBuffers oResizeBuffers;

void CreateRenderTarget()
{
    if (!g_pSwapChain || !g_pd3dDevice || !g_pd3dRtvDescHeap || !g_frameContext || NUM_BACK_BUFFERS <= 0)
        return;

    // We get the size of the RTV descriptor and the initial descriptor
    SIZE_T rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();

    // First, initialize the descriptors
    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
    {
        g_frameContext[i].g_mainRenderTargetDescriptor = rtvHandle;
        rtvHandle.ptr += rtvDescriptorSize;
    }

    // Then we create Render Target Views
    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
    {
        ID3D12Resource* pBackBuffer = nullptr;
        if (SUCCEEDED(g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer))))
        {
            g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, g_frameContext[i].g_mainRenderTargetDescriptor);
            g_frameContext[i].g_mainRenderTargetResource = pBackBuffer;
        }
        else
        {
            if (pBackBuffer)
            {
                pBackBuffer->Release();
                pBackBuffer = nullptr;
            }
        }
    }
}

void WaitForLastSubmittedFrame()
{
    FrameContext* frameCtx = &g_frameContext[g_frameIndex % NUM_FRAMES_IN_FLIGHT];

    UINT64 fenceValue = frameCtx->FenceValue;
    if (fenceValue == 0)
        return; // No fence was signaled

    frameCtx->FenceValue = 0;
    if (g_fence->GetCompletedValue() >= fenceValue)
        return;

    g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
    WaitForSingleObject(g_fenceEvent, INFINITE);
}

void CleanupRenderTarget()
{
    if (g_frameContext && NUM_BACK_BUFFERS > 0)
    {
        for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
        {
            if (g_frameContext[i].g_mainRenderTargetResource)
            {
                g_frameContext[i].g_mainRenderTargetResource->Release();
                g_frameContext[i].g_mainRenderTargetResource = nullptr;
            }
        }
    }
}

void __fastcall hkExecuteCommandLists(ID3D12CommandQueue* pCommandQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists)
{
    if (!g_pd3dCommandQueue)
    {
        g_pd3dCommandQueue = pCommandQueue;
    }

    oExecuteCommandLists(pCommandQueue, NumCommandLists, ppCommandLists);
}

HRESULT __fastcall hkSignal(ID3D12CommandQueue* queue, ID3D12Fence* fence, UINT64 value)
{
    if (g_pd3dCommandQueue != nullptr && queue == g_pd3dCommandQueue)
    {
        g_fence = fence;
        g_fenceValue = value;
    }
    return oSignal(queue, fence, value);
}

HRESULT __fastcall hkPresent(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags)
{
    static bool init = false;

    if (!init)
    {
        // We are waiting for the Commandqueue through Executecommandlists.
        if (!g_pd3dCommandQueue)
            return oPresent(pSwapChain, SyncInterval, Flags);

        if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D12Device), (void**)&g_pd3dDevice)))
        {
            DXGI_SWAP_CHAIN_DESC sdesc;
            pSwapChain->GetDesc(&sdesc);
            window = sdesc.OutputWindow;
            NUM_BACK_BUFFERS = sdesc.BufferCount;

            // SRV Heap
            {
                D3D12_DESCRIPTOR_HEAP_DESC desc = {};
                desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                desc.NumDescriptors = 1;
                desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
                if (FAILED(g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap))))
                    return oPresent(pSwapChain, SyncInterval, Flags);
            }

            // RTV Heap
            {
                D3D12_DESCRIPTOR_HEAP_DESC desc = {};
                desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
                desc.NumDescriptors = NUM_BACK_BUFFERS;
                desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
                desc.NodeMask = 1;
                if (FAILED(g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap))))
                    return oPresent(pSwapChain, SyncInterval, Flags);
            }

            // Command Allocator
            ID3D12CommandAllocator* allocator;
            if (FAILED(g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator))))
                return oPresent(pSwapChain, SyncInterval, Flags);

            // Command List
            if (FAILED(g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, nullptr, IID_PPV_ARGS(&g_pd3dCommandList))))
            {
                allocator->Release();
                return oPresent(pSwapChain, SyncInterval, Flags);
            }
            g_pd3dCommandList->Close();

            // Frame Contexts
            g_frameContext = new FrameContext[NUM_BACK_BUFFERS];
            if (!g_frameContext)
            {
                allocator->Release();
                return oPresent(pSwapChain, SyncInterval, Flags);
            }

            for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
            {
                g_frameContext[i].CommandAllocator = allocator;
                g_frameContext[i].FenceValue = 0;
            }

            // Fence & Events
            if (FAILED(g_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence))))
            {
                allocator->Release();
                return oPresent(pSwapChain, SyncInterval, Flags);
            }

            g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (g_fenceEvent == nullptr)
            {
                allocator->Release();
                return oPresent(pSwapChain, SyncInterval, Flags);
            }

            g_hSwapChainWaitableObject = pSwapChain->GetFrameLatencyWaitableObject();
            g_pSwapChain = pSwapChain;

            // Create RenderTarget
            CreateRenderTarget();

            // Hook window procedure
            impl::win32::init(window);

            ImGui::CreateContext();
            ImGui_ImplWin32_Init(window);

            ImGui_ImplDX12_Init(g_pd3dDevice, NUM_FRAMES_IN_FLIGHT,
                DXGI_FORMAT_R8G8B8A8_UNORM,
                g_pd3dSrvDescHeap,
                g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
                g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

            ImGuiIO& io = ImGui::GetIO();

            ImFont* defaultFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf", 15.0f);
            io.Fonts->Build();
            io.FontDefault = defaultFont;

            init = true;
        }
        return oPresent(pSwapChain, SyncInterval, Flags);
    }

    // We check all the necessary objects
    if (!g_pd3dCommandQueue || !g_pd3dDevice || !g_frameContext || !g_pd3dSrvDescHeap)
        return oPresent(pSwapChain, SyncInterval, Flags);

    // Processing changes size
    if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
    {
        WaitForLastSubmittedFrame();
        CleanupRenderTarget();
        g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
        g_ResizeWidth = g_ResizeHeight = 0;
        CreateRenderTarget();
    }


    // The beginning of the new frame
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    impl::tick();
    impl::render();

    //We get the current Back Buffer
    UINT backBufferIdx = g_pSwapChain->GetCurrentBackBufferIndex();
    FrameContext& frameCtx = g_frameContext[backBufferIdx];

    // Reset Command Allocator
    frameCtx.CommandAllocator->Reset();

    // Preparation for rendering
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = g_frameContext[backBufferIdx].g_mainRenderTargetResource;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    // Execution of rendering commands
    g_pd3dCommandList->Reset(frameCtx.CommandAllocator, nullptr);
    g_pd3dCommandList->ResourceBarrier(1, &barrier);
    g_pd3dCommandList->OMSetRenderTargets(1, &g_frameContext[backBufferIdx].g_mainRenderTargetDescriptor, FALSE, nullptr);
    g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);

    // Rendering imgui
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);

    // Return of the resource to the Pressent state
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    g_pd3dCommandList->ResourceBarrier(1, &barrier);
    g_pd3dCommandList->Close();

    // Permand List
    g_pd3dCommandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList* const*>(&g_pd3dCommandList));

    return oPresent(pSwapChain, SyncInterval, Flags);
}

HRESULT __fastcall hkResizeBuffers(IDXGISwapChain3* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
    // We check the readiness to change the size
    if (!g_pd3dDevice || !g_pSwapChain)
    {
        return oResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
    }

    if (g_pd3dDevice)
    {
        ImGui_ImplDX12_InvalidateDeviceObjects();
    }

    CleanupRenderTarget();

    // We save a new number of buffers
    NUM_BACK_BUFFERS = BufferCount;

    // Call the original function
    HRESULT result = oResizeBuffers(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);

    if (SUCCEEDED(result))
    {
        // We will recharge our resources only if we successfully changed the size
        CreateRenderTarget();
        if (g_pd3dDevice)
        {
            ImGui_ImplDX12_CreateDeviceObjects();
        }
    }

    return result;
}

void impl::d3d12::init()
{
    kiero::bind(54, (void**)&oExecuteCommandLists, hkExecuteCommandLists);
    kiero::bind(58, (void**)&oSignal, hkSignal);
    kiero::bind(140, (void**)&oPresent, hkPresent);
    kiero::bind(145, (void**)&oResizeBuffers, hkResizeBuffers);
}

void impl::d3d12::shutdown()
{
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();

    ImGui::DestroyContext();

    CleanupRenderTarget();

    if (g_pd3dCommandQueue) g_pd3dCommandQueue->Release();
    if (g_pSwapChain) g_pSwapChain->Release();
    if (g_pd3dDevice) g_pd3dDevice->Release();

    kiero::unbind(54);
    kiero::unbind(58);
    kiero::unbind(140);
    kiero::unbind(145);
}

#endif // KIERO_INCLUDE_D3D12
