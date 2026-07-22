#include "../../kiero/kiero.h"

#if KIERO_INCLUDE_OPENGL
#define GLEW_STATIC

#include "opengl_impl.h"
#include <Windows.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <assert.h>

#include "win32_impl.h"

#include "../imgui.h"
#include "../imgui_impl_win32.h"
#include "../imgui_impl_opengl3.h"
#include <unordered_map>

#pragma comment(lib, "opengl32.lib")

typedef BOOL(WINAPI* wglSwapBuffers_t)(HDC hdc);
static wglSwapBuffers_t oWglSwapBuffers = nullptr;

struct GLStateBackup {
    GLint last_program = 0;
    GLint last_texture = 0;
    GLint last_array_buffer = 0;
    GLint last_vertex_array = 0;
    GLint last_active_texture = 0;
    GLint last_viewport[4] = { 0,0,0,0 };
    GLint last_scissor[4] = { 0,0,0,0 };
    GLboolean last_enable_blend = GL_FALSE;
    GLboolean last_enable_depth_test = GL_FALSE;
    GLboolean last_enable_scissor_test = GL_FALSE;
    GLint last_draw_fbo = 0;
};

static void BackupGLState(GLStateBackup& s) {
    glGetIntegerv(GL_CURRENT_PROGRAM, &s.last_program);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &s.last_texture);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &s.last_array_buffer);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &s.last_vertex_array);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &s.last_active_texture);
    glGetIntegerv(GL_VIEWPORT, s.last_viewport);
    glGetIntegerv(GL_SCISSOR_BOX, s.last_scissor);
    s.last_enable_blend = glIsEnabled(GL_BLEND);
    s.last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
    s.last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &s.last_draw_fbo);
}

static void RestoreGLState(const GLStateBackup& s) {
    // restore program, textures, buffers, VAO
    glUseProgram((GLuint)s.last_program);

    glActiveTexture((GLenum)s.last_active_texture);
    glBindTexture(GL_TEXTURE_2D, (GLuint)s.last_texture);

    glBindBuffer(GL_ARRAY_BUFFER, (GLuint)s.last_array_buffer);
    glBindVertexArray((GLuint)s.last_vertex_array);

    if (s.last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    if (s.last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (s.last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);

    glViewport(s.last_viewport[0], s.last_viewport[1], (GLsizei)s.last_viewport[2], (GLsizei)s.last_viewport[3]);
    glScissor(s.last_scissor[0], s.last_scissor[1], s.last_scissor[2], s.last_scissor[3]);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, (GLuint)s.last_draw_fbo);
}

BOOL WINAPI hkWglSwapBuffers(HDC hdc)
{
    static bool init = false;

    if (!init)
    {
        glewInit();

        HWND hwnd = WindowFromDC(wglGetCurrentDC ? (HDC)wglGetCurrentDC() : nullptr);

        impl::win32::init(hwnd);

        ImGui::CreateContext();
        ImGui_ImplWin32_Init(hwnd);
        ImGui_ImplOpenGL3_Init("#version 430 core");
        ImGui_ImplOpenGL3_CreateDeviceObjects();

        ImGuiIO& io = ImGui::GetIO();

        ImFont* defaultFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf", 15.0f);
        io.Fonts->Build();
        io.FontDefault = defaultFont;

        init = true;
    }

    // ensure we're rendering on the current HGLRC used by the game
    HGLRC ctx = wglGetCurrentContext();
    if (!ctx) return oWglSwapBuffers(hdc);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    impl::tick();
    impl::render();

    ImGui::Render();

    GLStateBackup backup;
    BackupGLState(backup);

    // If the game renders to FBOs, ensure we draw to default framebuffer (or adapt to game's FBO)
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    // Render ImGui on top
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // flush to ensure commands are sent before swap
    glFlush();

    // Restore GL state so the game is not affected next frame
    RestoreGLState(backup);

    return oWglSwapBuffers(hdc);
}

void impl::opengl::init()
{
    // Get pointer to wglSwapBuffers
    HMODULE hOpenGL = GetModuleHandleA("opengl32.dll");
    oWglSwapBuffers = (wglSwapBuffers_t)GetProcAddress(hOpenGL, "wglSwapBuffers");

    // hook it
    kiero::bind(oWglSwapBuffers, (void**)&oWglSwapBuffers, (void*)hkWglSwapBuffers);
}

void impl::opengl::shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();

    ImGui::DestroyContext();

    // Get pointer to wglSwapBuffers
    HMODULE hOpenGL = GetModuleHandleA("opengl32.dll");
    oWglSwapBuffers = (wglSwapBuffers_t)GetProcAddress(hOpenGL, "wglSwapBuffers");

    // unhook it
    kiero::unbind(oWglSwapBuffers);
}

#endif // KIERO_INCLUDE_OPENGL
