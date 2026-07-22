#include "dllmain.h"

#include <Windows.h>
#include <algorithm>
#include <fstream>
#include <thread>

#include "scs_sdk/scssdk_telemetry.h"
#include "rendering.h"
#include "ImGui/imgui.h"

#include "globals.h"
#include "bmem.h"
#include "scs_logging.h"
#include "prism/prism.h"

#include "launch_arguments_manager.h"

#include "direct_input_manager.h"
#include <unordered_map>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include <DirectXMath.h>
using namespace DirectX;


#pragma comment(lib, "../Lib/minhook.x64.lib")

using namespace scs_logging;

RenderManager* renderer;

std::vector<prism::game_trailer_actor_u*> locked_trailers;
std::vector<prism::game_trailer_actor_u*> temp_locked_trailers;

uintptr_t game_ctrl_ptr;
uint32_t game_actor_offset;

bool is_fatal_error = false;
std::string error = "";

bool running_truckersmp = false;
bool shown_truckersmp_warning = false;

void SetupImGuiStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();

    style.Alpha = 1.0f;
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.WindowRounding = 2.0f;
    style.WindowBorderSize = 1.0f;
    style.WindowMinSize = ImVec2(32.0f, 32.0f);
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_None;
    style.ChildRounding = 3.0f;
    style.ChildBorderSize = 1.0f;
    style.PopupRounding = 3.0f;
    style.PopupBorderSize = 1.0f;
    style.FramePadding = ImVec2(4.0f, 3.0f);
    style.FrameRounding = 3.0f;
    style.FrameBorderSize = 1.0f;
    style.ItemSpacing = ImVec2(8.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
    style.CellPadding = ImVec2(4.0f, 2.0f);
    style.IndentSpacing = 21.0f;
    style.ColumnsMinSpacing = 6.0f;
    style.ScrollbarSize = 5.599999904632568f;
    style.ScrollbarRounding = 18.0f;
    style.GrabMinSize = 10.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 3.0f;
    style.TabBorderSize = 0.0f;
//    style.TabMinWidthForCloseButton = 0.0f;
    style.ColorButtonPosition = ImGuiDir_Right;
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

    style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.6000000238418579f, 0.6000000238418579f, 0.6000000238418579f, 1.0f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.125490203499794f, 0.125490203499794f, 0.125490203499794f, 1.0f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.125490203499794f, 0.125490203499794f, 0.125490203499794f, 1.0f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.168627455830574f, 0.168627455830574f, 0.168627455830574f, 1.0f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.250980406999588f, 0.250980406999588f, 0.250980406999588f, 1.0f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.168627455830574f, 0.168627455830574f, 0.168627455830574f, 1.0f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2156862765550613f, 0.2156862765550613f, 0.2156862765550613f, 1.0f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.250980406999588f, 0.250980406999588f, 0.250980406999588f, 1.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.125490203499794f, 0.125490203499794f, 0.125490203499794f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.168627455830574f, 0.168627455830574f, 0.168627455830574f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.125490203499794f, 0.125490203499794f, 0.125490203499794f, 1.0f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.168627455830574f, 0.168627455830574f, 0.168627455830574f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.125490203499794f, 0.125490203499794f, 0.125490203499794f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.250980406999588f, 0.250980406999588f, 0.250980406999588f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.3019607961177826f, 0.3019607961177826f, 0.3019607961177826f, 1.0f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.3490196168422699f, 0.3490196168422699f, 0.3490196168422699f, 1.0f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 0.7960784435272217f, 0.6078431606292725f, 1.0f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.0f, 0.7960784435272217f, 0.6078431606292725f, 1.0f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.5922746658325195f, 0.452229231595993f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.168627455830574f, 0.168627455830574f, 0.168627455830574f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.2156862765550613f, 0.2156862765550613f, 0.2156862765550613f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.250980406999588f, 0.250980406999588f, 0.250980406999588f, 1.0f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.2156862765550613f, 0.2156862765550613f, 0.2156862765550613f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.250980406999588f, 0.250980406999588f, 0.250980406999588f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.3019607961177826f, 0.3019607961177826f, 0.3019607961177826f, 1.0f);
    style.Colors[ImGuiCol_Separator] = ImVec4(0.2156862765550613f, 0.2156862765550613f, 0.2156862765550613f, 1.0f);
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.250980406999588f, 0.250980406999588f, 0.250980406999588f, 1.0f);
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.3019607961177826f, 0.3019607961177826f, 0.3019607961177826f, 1.0f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.2156862765550613f, 0.2156862765550613f, 0.2156862765550613f, 1.0f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.250980406999588f, 0.250980406999588f, 0.250980406999588f, 1.0f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.3019607961177826f, 0.3019607961177826f, 0.3019607961177826f, 1.0f);
    style.Colors[ImGuiCol_Tab] = ImVec4(0.168627455830574f, 0.168627455830574f, 0.168627455830574f, 1.0f);
    style.Colors[ImGuiCol_TabHovered] = ImVec4(0.2156862765550613f, 0.2156862765550613f, 0.2156862765550613f, 1.0f);
    style.Colors[ImGuiCol_TabActive] = ImVec4(0.250980406999588f, 0.250980406999588f, 0.250980406999588f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.168627455830574f, 0.168627455830574f, 0.168627455830574f, 1.0f);
    style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.2156862765550613f, 0.2156862765550613f, 0.2156862765550613f, 1.0f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(0.0f, 0.4705882370471954f, 0.843137264251709f, 1.0f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.0f, 0.3294117748737335f, 0.6000000238418579f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.0f, 0.4705882370471954f, 0.843137264251709f, 1.0f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.0f, 0.3294117748737335f, 0.6000000238418579f, 1.0f);
    style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
    style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
    style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
    style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.0f, 0.4705882370471954f, 0.843137264251709f, 1.0f);
    style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.8999999761581421f);
    style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 1.0f);
    style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
    style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
    style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.3499999940395355f);
}
bool isTrailerLocked(prism::game_trailer_actor_u* trailer)
{
    for (prism::game_trailer_actor_u* locked_trailer : locked_trailers) {
        if (locked_trailer == trailer)
            return true;
    }

    return false;
}

bool isTrailerTempLocked(prism::game_trailer_actor_u* trailer)
{
    for (prism::game_trailer_actor_u* temp_locked_trailer : temp_locked_trailers) {
        if (temp_locked_trailer == trailer)
            return true;
    }

    return false;
}


bool isTrailerSteerable(prism::game_trailer_actor_u* trailer)
{
    return trailer->steering_data != nullptr;
}

void render_error(std::string message) {
    ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), message.c_str());
}

void render_fatal(std::string message) {
    render_error("A fatal error has occured within this plugin!");
    ImGui::TextWrapped("Please make sure your game is fully loaded!");

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

    if (ImGui::Button("Get Help!"))
    {
        ShellExecuteA(NULL, "open", "https://discord.gg/insanux", NULL, NULL, SW_SHOWNORMAL);
    }

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Spacing();

#ifdef _DEBUG
    if (ImGui::CollapsingHeader("Developer Information"))
    {
        render_error("Error: " + message);
        ImGui::Separator();

        uint8_t* game_ctrl = *(uint8_t**)game_ctrl_ptr;

        ImGui::TextWrapped("game_ctrl_ptr: %p", game_ctrl_ptr);
        ImGui::TextWrapped("game_ctrl: %p", game_ctrl);

        if (game_ctrl) {
            uint8_t* game_actor = *(uint8_t**)(game_ctrl + game_actor_offset);
            ImGui::TextWrapped("game_actor: %p", game_actor);

            if (game_actor) {
                uint8_t* trailer_actor = *(uint8_t**)(game_actor + 0xc0);
                ImGui::TextWrapped("trailer_actor: %p", trailer_actor);


            }
        }

        ImGui::TextWrapped("game_actor_offset: %d", game_actor_offset);
    }
#endif

    ImGui::EndTabItem();
}

BOOL CALLBACK EnumAxesCallback(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext)
{
    auto* axisList = reinterpret_cast<std::vector<DIDEVICEOBJECTINSTANCE>*>(pContext);

    if (pdidoi->dwType & DIDFT_AXIS)
    {
        axisList->push_back(*pdidoi);
    }

    return DIENUM_CONTINUE;
}

struct input_device
{
    std::string name;
    LPDIRECTINPUTDEVICE8 di8_device;
    std::unordered_map<std::string, float> joystates;
    DIJOYSTATE2 joystate;
};
std::vector<input_device> input_devices;

int axis_device_index = -1; // the index in the array ^ of the device thats being used for steering
int last_axis_device_index = -1;

std::string steering_axis_name = "None";
std::string last_steering_axis_name = "None";

int steer_button_device_index = -1;
int last_steer_button_device_index = -1;

std::string steer_left_button_name = "None";
std::string last_steer_left_button_name = "None";
std::string steer_right_button_name = "None";
std::string last_steer_right_button_name = "None";

int current_controlled_trailer = 0; // the trailer you are currently controlling
int last_current_controlled_trailer = 0;

bool use_axis_steering = false;
bool last_use_axis_steering = false;
bool use_button_steering = false;
bool last_use_button_steering = false;

std::string get_next_button_input(int device_index)
{
    input_device& device = input_devices[device_index];

    for (int i = 0; i < 128; ++i)
        if (device.joystate.rgbButtons[i] & 0x80)
            return std::to_string(i);

    return "";
}

std::string get_button_name_xbox(std::string index)
{
    int i = -1;
    try {
        i = std::stoi(index);
    }
    catch (...) {
        return index;
    }

    switch (i)
    {
        case 0: return "A";
        case 1: return "B";
        case 2: return "X";
        case 3: return "Y";
        case 4: return "Left Shoulder";
        case 5: return "Right Shoulder";
        case 6: return "Back";
        case 7: return "Start";
        case 8: return "Left Stick";
        case 9: return "Right Stick";
        default: return index + " (Unknown Key Name)";
    }
}

std::string get_button_name_ps(std::string index)
{
    int i = -1;
    try {
        i = std::stoi(index);
    }
    catch (...) {
        return index;
    }

    switch (i)
    {
        case 0: return "X";
        case 1: return "O";
        case 2: return "Sqare";
        case 3: return "Y";
        case 4: return "Left Shoulder";
        case 5: return "Right Shoulder";
        case 6: return "Select";
        case 7: return "Start";
        case 8: return "Left Stick";
        case 9: return "Right Stick";
        default: return index + " (Unknown Key Name)";
    }
}

std::string get_button_name_logi(std::string index)
{
    int i = -1;
    try {
        i = std::stoi(index);
    }
    catch (...) {
        return index;
    }

    switch (i)
    {
        case 0:  return "Cross";
        case 1:  return "Square";
        case 2:  return "Circle";
        case 3:  return "Triangle";
        case 4:  return "L1 / Paddle Left";
        case 5:  return "R1 / Paddle Right";
        case 6:  return "L2";
        case 7:  return "R2";
        case 8:  return "Share";
        case 9:  return "Options";
        case 10: return "R3";
        case 11: return "L3";
        case 19: return "+";
        case 20: return "-";
        case 21: return "Knob Right";
        case 22: return "Knob Left";
        case 23: return "Reset Car / Enter";
        case 24: return "PS Button (Home)";
        default: return index + " (Unknown Key Name)";
    }
}

std::string get_button_name(int device_index, std::string key)
{
    input_device& device = input_devices[device_index];

    std::string name = device.name;
    std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return std::tolower(c); });

    if (name.find("xbox") != std::string::npos)
    {
        return get_button_name_xbox(key);
    }

    if (name.find("playstation") != std::string::npos)
    {
        return get_button_name_ps(key);
    }

    if (name.find("g29") != std::string::npos)
    {
        return get_button_name_logi(key);
    }

    return key + (key != "None" ? " (Unknown Key Name)" : "");
}

void render_keybinds()
{
    // Initalize the devices list
    std::vector<const char*> device_items; // No static here, rebuild every frame
    device_items.push_back("None");
    for (input_device& device : input_devices)
        device_items.push_back(device.name.c_str());

    // Device selections from saved values
    int selected_device_axis = (axis_device_index == -1) ? 0 : axis_device_index + 1;
    int selected_device_button = (steer_button_device_index == -1) ? 0 : steer_button_device_index + 1;

    // ==========================
    // Steering Axis
    // ==========================
    ImGui::TextWrapped("Steering Axis:");
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Checkbox("Use Axis Steering?", &use_axis_steering);

    if (use_axis_steering)
    {
        use_button_steering = false;

        // Device combo
        if (ImGui::BeginCombo("Steering Device", device_items[selected_device_axis]))
        {
            for (int n = 0; n < (int)device_items.size(); n++)
            {
                bool is_selected = (selected_device_axis == n);
                if (ImGui::Selectable(device_items[n], is_selected))
                    selected_device_axis = n;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Select the input device to use for steering.");
        axis_device_index = selected_device_axis - 1;

        if (axis_device_index != -1)
        {
            std::vector<const char*> axis_items;
            axis_items.push_back("None");
            for (const auto& [name, value] : input_devices[axis_device_index].joystates)
                axis_items.push_back(name.c_str());

            ImGui::TextWrapped("Using the table below, select which key you want to use for steering");
            ImGui::TextWrapped("The steering value should be: center = 0, full left = -1, full right = 1");

            if (ImGui::BeginTable("JoystickAxes", 2, ImGuiTableFlags_Borders))
            {
                for (const auto& [name, value] : input_devices[axis_device_index].joystates)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextWrapped("%s", name.c_str());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextWrapped("%f", value);
                }
                ImGui::EndTable();
            }

            // Find saved steering axis index
            int selected_axis = 0;
            for (size_t i = 1; i < axis_items.size(); i++)
            {
                if (steering_axis_name == axis_items[i])
                {
                    selected_axis = (int)i;
                    break;
                }
            }

            if (ImGui::BeginCombo("Inputs", axis_items[selected_axis]))
            {
                for (int n = 0; n < (int)axis_items.size(); n++)
                {
                    bool is_selected = (selected_axis == n);
                    if (ImGui::Selectable(axis_items[n], is_selected))
                        selected_axis = n;
                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            steering_axis_name = axis_items[selected_axis];

            if (steering_axis_name != "None")
            {
                ImGui::SliderFloat("Steering Preview",
                    &input_devices[axis_device_index].joystates[steering_axis_name],
                    -1.f, 1.f);

                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("The steering value should be: center = 0, full left = -1, full right = 1");
            }
        }
    }

    // ==========================
    // Button Steering
    // ==========================
    ImGui::Spacing(); ImGui::Spacing();
    ImGui::TextWrapped("Button Steering");
    ImGui::Separator();
    ImGui::Checkbox("Use Button Steering?", &use_button_steering);

    if (use_button_steering)
    {
        use_axis_steering = false;

        if (ImGui::BeginCombo("Steering Device", device_items[selected_device_button]))
        {
            for (int n = 0; n < (int)device_items.size(); n++)
            {
                bool is_selected = (selected_device_button == n);
                if (ImGui::Selectable(device_items[n], is_selected))
                    selected_device_button = n;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Select the input device to use for steering.");
        steer_button_device_index = selected_device_button - 1;

        if (steer_button_device_index != -1)
        {
            static bool want_left_button_input = false;
            static bool want_right_button_input = false;

            std::string steer_left_text = steer_left_button_name;
            std::string steer_right_text = steer_right_button_name;

            if (want_left_button_input)
            {
                want_right_button_input = false;
                steer_left_text = "Press a key...";
                std::string new_button = get_next_button_input(steer_button_device_index);
                if (!new_button.empty())
                {
                    steer_left_button_name = new_button;
                    if (steer_left_button_name == steer_right_button_name)
                        steer_right_button_name = "None";
                    want_left_button_input = false;
                }
            }

            if (want_right_button_input)
            {
                want_left_button_input = false;
                steer_right_text = "Press a key...";
                std::string new_button = get_next_button_input(steer_button_device_index);
                if (!new_button.empty())
                {
                    steer_right_button_name = new_button;
                    if (steer_right_button_name == steer_left_button_name)
                        steer_left_button_name = "None";
                    want_right_button_input = false;
                }
            }

            if (ImGui::Button(get_button_name(steer_button_device_index, steer_left_text).c_str()))
                want_left_button_input = !want_left_button_input;

            ImGui::SameLine(); ImGui::TextWrapped("Steer Left");

            if (ImGui::Button((get_button_name(steer_button_device_index, steer_right_text) + "##1").c_str()))
                want_right_button_input = !want_right_button_input;

            ImGui::SameLine(); ImGui::TextWrapped("Steer Right");
        }
    }

    ImGui::EndTabItem();

    // check if the settings changed
    if (   axis_device_index != last_axis_device_index
        || steering_axis_name != last_steering_axis_name
        || steer_button_device_index != last_steer_button_device_index
        || steer_left_button_name != last_steer_left_button_name
        || steer_right_button_name != last_steer_right_button_name
        || current_controlled_trailer != last_current_controlled_trailer
        || use_axis_steering != last_use_axis_steering
        || use_button_steering != last_use_button_steering
    )
    {
        json settings = {
            {"version", 1},
            {"axis_device_index", axis_device_index},
            {"steering_axis_name", steering_axis_name},
            {"steer_button_device_index", steer_button_device_index},
            {"steer_left_button_name", steer_left_button_name},
            {"steer_right_button_name", steer_right_button_name},
            {"current_controlled_trailer", current_controlled_trailer},
            {"use_axis_steering", use_axis_steering},
            {"use_button_steering", use_button_steering}
        };

        std::ofstream file("plugins/ETS2-TrailerUtil.json");
        file << settings.dump(4);
        file.close();

        last_axis_device_index = axis_device_index;
        last_steering_axis_name = steering_axis_name;
        last_steer_button_device_index = steer_button_device_index;
        last_steer_left_button_name = steer_left_button_name;
        last_steer_right_button_name = steer_right_button_name;
        last_current_controlled_trailer = current_controlled_trailer;
        last_use_axis_steering = use_axis_steering;
        last_use_button_steering = use_button_steering;

        scs_log(0, "Settings saved");
    }
}

void render_trailers()

{
    ImGui::TextWrapped("Vehicle Trailers:");
    ImGui::Separator();
    ImGui::Spacing();

    uint8_t* game_ctrl = *(uint8_t**)game_ctrl_ptr;

    if (game_ctrl)
    {
        uint8_t* game_actor = *(uint8_t**)(game_ctrl + game_actor_offset);
        if (game_actor)
        {
            prism::game_trailer_actor_u* trailer_actor = *(prism::game_trailer_actor_u**)(game_actor + 0xc8);

            if (trailer_actor)
            {
                ImGui::Spacing();

                int trailerIndex = 0;
                while (trailer_actor != nullptr)
                {
                    bool steerable = isTrailerSteerable(trailer_actor);

                    std::stringstream ss;
                    ss << "Trailer (" << trailerIndex << ")" << (!steerable ? " | Not Steerable" : "");


                    if (ImGui::CollapsingHeader(ss.str().c_str()))
                    {
                        ImGui::BeginDisabled(!steerable);

                        bool locked = isTrailerLocked(trailer_actor);
                        bool tempLocked = isTrailerTempLocked(trailer_actor);

                        ImGui::TextWrapped("Steering: ");

                        if (ImGui::Checkbox(("##Locked" + std::to_string(trailerIndex)).c_str(), &locked)) {
                            if (locked)
                            {
                                scs_log(0, "Trailer '%p' was added to the locked list", trailer_actor);
                                locked_trailers.push_back(trailer_actor);
                            }
                            else
                            {
                                scs_log(0, "Trailer '%p' was removed from the locked list", trailer_actor);
                                locked_trailers.erase(
                                    std::remove(locked_trailers.begin(), locked_trailers.end(), trailer_actor),
                                    locked_trailers.end()
                                );
                            }
                        }

                        if (ImGui::IsItemHovered())
                        {
                            if (locked)
                                ImGui::SetTooltip("Unlock the steering position");
                            else
                                ImGui::SetTooltip("Lock the steering position");
                        }

                        ImGui::SameLine();
                        ImGui::PushItemWidth(-1);

                        ImGui::SliderFloat(("##Steering" + std::to_string(trailerIndex)).c_str(), &trailer_actor->steering, -1.f, 1.f);

                        ImGui::PopItemWidth();
                        if (ImGui::IsItemActive())
                        {
                            if (!locked && !tempLocked)
                            {
                                scs_log(0, "Trailer '%p' was added to the temp locked list", trailer_actor);
                                temp_locked_trailers.push_back(trailer_actor);
                            }
                        }
                        else
                        {
                            if (tempLocked)
                            {
                                scs_log(0, "Trailer '%p' was removed from the temp locked list", trailer_actor);
                                temp_locked_trailers.erase(
                                    std::remove(temp_locked_trailers.begin(), temp_locked_trailers.end(), trailer_actor),
                                    temp_locked_trailers.end()
                                );
                            }
                        }

                        ImGui::EndDisabled();

                        ImGui::Spacing();

                        ImGui::Text("Suspention Height: ");
                        ImGui::PushItemWidth(-1);

                        ImGui::SliderFloat(("##Suspention" + std::to_string(trailerIndex)).c_str(), &trailer_actor->suspention_height, 0.25f, 5.f);
                        ImGui::PopItemWidth();


                        ImGui::Spacing();

                        if (trailer_actor->hook_joint)
                        {
                            std::string joint = "unknown";
                            std::string btn_text = "Unsupported";
                            switch (trailer_actor->hook_joint->physx_joint->GetMotion(prism::PxD6Axis::Enum::eTWIST))
                            {
                            case prism::PxD6Motion::Enum::eFREE:
                                joint = "free";
                                btn_text = "Lock";
                                break;
                            case prism::PxD6Motion::Enum::eLIMITED:
                                joint = "limited";
                                break;
                            case prism::PxD6Motion::Enum::eLOCKED:
                                joint = "locked";
                                btn_text = "Unlock";
                                break;
                            }

                            ImGui::Text("Joint Type: %s", joint.c_str());

                            if (ImGui::Button((btn_text + "##" + std::to_string(trailerIndex)).c_str()))
                            {
                                if (btn_text == "Lock")
                                {
                                    trailer_actor->hook_joint->physx_joint->SetMotion(prism::PxD6Axis::Enum::eTWIST, prism::PxD6Motion::Enum::eLOCKED);
                                }
                                else
                                {
                                    trailer_actor->hook_joint->physx_joint->SetMotion(prism::PxD6Axis::Enum::eTWIST, prism::PxD6Motion::Enum::eFREE);
                                }
                            }
                        }


                        ImGui::Separator();
                        ImGui::Spacing();
                        ImGui::Spacing();
                    }



                    trailer_actor = trailer_actor->slave;
                    trailerIndex++;
                }
            }
            else {
                ImGui::TextWrapped("You do not have a active trailer...");
                locked_trailers.clear();
            }
        }
        else return render_fatal("game_actor is missing");
    }
    else return render_fatal("game_ctrl is missing");



    ImGui::EndTabItem();
}

void render_about()
{
    ImGui::TextWrapped("About:");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextWrapped("Version: 1.0.4");
    ImGui::TextWrapped("Supported Game Version: 1.59");
    ImGui::TextWrapped("Author: Baldy09");

    ImGui::Spacing(); ImGui::Spacing();

    ImGui::TextWrapped("Description:");
    ImGui::TextWrapped("ETS2-TrailerUtil is a plugin designed to enhance trailer control "
        "and steering customization within Euro Truck Simulator 2. It "
        "provides both axis and button steering options, real-time trailer "
        "steering adjustments.");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::CollapsingHeader("Credits"))
    {
        ImGui::TextWrapped("Programming: Baldy09");
        ImGui::TextWrapped("Testing: FemmyRybo, VaselineeOnToast, OnyxSabertooth, superstar1607");
    }

    if (ImGui::CollapsingHeader("License"))
    {
        ImGui::TextWrapped("This plugin is distributed for personal and non-commercial use. "
            "No warranties are provided. Use at your own risk. "
            "Reuploads of the compiled DLL file are strictly prohibited. "
            "Any unauthorized distribution will result in the file being taken down.");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Support My Work"))
    {
        ShellExecuteA(NULL, "open", "https://www.paypal.com/paypalme/BaldyMods", NULL, NULL, SW_SHOWNORMAL);
    }
    ImGui::SameLine();
    if (ImGui::Button("Join My Discord"))
    {
        ShellExecuteA(NULL, "open", "https://discord.gg/NeggzRmHH7", NULL, NULL, SW_SHOWNORMAL);
    }
    ImGui::SameLine();
    if (ImGui::Button("Get Plugin Support"))
    {
        ShellExecuteA(NULL, "open", "https://discord.gg/insanux", NULL, NULL, SW_SHOWNORMAL);
    }

    ImGui::EndTabItem();
}



bool styled = false;
void dllmain::render_tick()
{
    if (!styled)
    {
        SetupImGuiStyle();
        styled = true;
    }

    ImGui::Begin("ETS2 Trailer Utils");

    if (running_truckersmp && !shown_truckersmp_warning)
    {
        ImGui::TextWrapped("TruckersMP Warning");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextWrapped("You are currently running TruckersMP. This plugin is not supported on multiplayer servers (however it does work) and may result in a ban.");
        ImGui::TextWrapped("\nIf you do get banned whilst using this plugin, please try contact the Game Moderation Management and discuss your ban there");


        ImGui::TextWrapped("\n\nBy clicking 'I Understand', you acknowledge that you are using this plugin at your own risk.");
        if (ImGui::Button("I Understand"))
        {
            shown_truckersmp_warning = true;
        }
    }


    if (!is_fatal_error && (!running_truckersmp || shown_truckersmp_warning)) {

        if (ImGui::BeginTabBar("MyTabBar"))
        {
            if (ImGui::BeginTabItem("Trailers"))
            {
                render_trailers();
            }

            if (ImGui::BeginTabItem("Keybinds"))
            {
                render_keybinds();
            }

            if (ImGui::BeginTabItem("About"))
            {
                render_about();
            }

            ImGui::EndTabBar();
        }
    }
    else
        render_error(error);

    ImGui::End();
}



void dllmain::main_tick()
{
    // Update devices
    for (input_device& device : input_devices)
    {
        DIJOYSTATE2 joystate = {};

        HRESULT hr = device.di8_device->Poll();
        if (FAILED(hr))
        {
            hr = device.di8_device->Acquire();
            while (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
            {
                hr = device.di8_device->Acquire();
            }

            if (FAILED(hr))
            {
                ImGui::TextWrapped("Failed to acquire device: 0x%08X", hr);
                continue;
            }
        }

        hr = device.di8_device->GetDeviceState(sizeof(DIJOYSTATE2), &joystate);
        if (FAILED(hr))
        {
            ImGui::TextWrapped("Failed to get device state: 0x%08X", hr);
            continue;
        }

        device.joystates = {
            { "lX",  ((float)joystate.lX - 32767.5f) / 32767.5f },
            { "lY",  ((float)joystate.lY - 32767.5f) / 32767.5f },
            { "lZ",  ((float)joystate.lZ - 32767.5f) / 32767.5f },
            { "lRx",  ((float)joystate.lRx - 32767.5f) / 32767.5f },
            { "lRy",  ((float)joystate.lRy - 32767.5f) / 32767.5f },
            { "lRz",  ((float)joystate.lRz - 32767.5f) / 32767.5f },
        };

        device.joystate = joystate;

        /*
            // Print joystick axes values every frame
            ImGui::TextWrapped("lX: %ld", joystate.lX);
            ImGui::TextWrapped("lY: %ld", joystate.lY);
            ImGui::TextWrapped("lZ: %ld", joystate.lZ);
            ImGui::TextWrapped("lRx: %ld", joystate.lRx);
            ImGui::TextWrapped("lRy: %ld", joystate.lRy);
            ImGui::TextWrapped("lRz: %ld", joystate.lRz);

            ImGui::TextWrapped("Slider 0: %ld", joystate.rglSlider[0]);
            ImGui::TextWrapped("Slider 1: %ld", joystate.rglSlider[1]);

            for (int i = 0; i < 4; ++i)
                ImGui::TextWrapped("POV %d: %lu", i, joystate.rgdwPOV[i]);

            for (int i = 0; i < 128; ++i)
                if (joystate.rgbButtons[i] & 0x80)
                    ImGui::TextWrapped("Button %d pressed", i);
        */
    }


    // Handle keybinds
    if (use_button_steering || use_axis_steering)
    {
        uint8_t* game_ctrl = *(uint8_t**)game_ctrl_ptr;

        if (game_ctrl)
        {
            uint8_t* game_actor = *(uint8_t**)(game_ctrl + game_actor_offset);

            if (game_actor)
            {
                prism::game_trailer_actor_u* trailer_actor = *(prism::game_trailer_actor_u**)(game_actor + 0xC0);
                if (trailer_actor)
                {
                    int trailerIndex = 0;
                    while (trailer_actor != nullptr)
                    {
                        if (trailerIndex == current_controlled_trailer)
                        {
                            if (use_button_steering && steer_button_device_index != -1)
                            {
                                std::string pressed_key = get_next_button_input(steer_button_device_index);
                                if (pressed_key != "")
                                {
                                    if (pressed_key == steer_left_button_name)
                                    {
                                        if (!isTrailerLocked(trailer_actor))
                                        {
                                            scs_log(0, "Trailer '%p' was added to the locked list, steer left was pressed", trailer_actor);
                                            locked_trailers.push_back(trailer_actor);
                                        }

                                        if (trailer_actor->steering - 0.01f < -1.f)
                                            trailer_actor->steering = -1.f;
                                        else
                                            trailer_actor->steering -= 0.01f;
                                    }

                                    if (pressed_key == steer_right_button_name)
                                    {
                                        if (!isTrailerLocked(trailer_actor))
                                        {
                                            scs_log(0, "Trailer '%p' was added to the locked list, steer right was pressed", trailer_actor);
                                            locked_trailers.push_back(trailer_actor);
                                        }

                                        if (trailer_actor->steering + 0.01f > 1.f)
                                            trailer_actor->steering = 1.f;
                                        else
                                            trailer_actor->steering += 0.01f;
                                    }
                                }
                            }

                            if (use_axis_steering && axis_device_index != -1 && steering_axis_name != "None")
                            {
                                if (!isTrailerLocked(trailer_actor))
                                {
                                    scs_log(0, "Trailer '%p' was added to the locked list, axis steering in use", trailer_actor);
                                    locked_trailers.push_back(trailer_actor);
                                }

                                input_device device = input_devices[axis_device_index];

                                float steering_position = device.joystates[steering_axis_name];

                                if (steering_position <= 1.f && steering_position >= -1.f)
                                {
                                    trailer_actor->steering = steering_position;
                                }
                                else
                                {
                                    //dxManager::ShowNotification("Axis steering input is invalid, it has been reset.");
                                    steering_axis_name = "None";
                                    use_axis_steering = false;
                                }
                            }

                            break;
                        }

                        trailer_actor = trailer_actor->slave;
                        trailerIndex++;
                    }
                }
            }
        }
    }
}




prism::g_advance_steering_t g_advance_steering_original;
void g_advance_steering_hook(prism::game_trailer_actor_u* vehicle_actor)
{
    if (isTrailerLocked(vehicle_actor) || isTrailerTempLocked(vehicle_actor))
    {
        prism::g_set_vehicle_steering(vehicle_actor->steering_data, vehicle_actor->steering);
        return;
    }

    g_advance_steering_original(vehicle_actor);
}



std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return s;
}

#pragma comment( linker, "/export:scs_telemetry_init=scs_telemetry_init" )
SCSAPI_RESULT scs_telemetry_init(const scs_u32_t version, const scs_telemetry_init_params_t* const params)
{
	scs_logging::init(params, "ETS2-TrailerUtil");

	scs_log(0, "ETS2 Trailer Utils | By: Baldy09 - Insanux");
	scs_log(0, "Plugin Loading...");

    HMODULE TruckersMP = GetModuleHandleA("core_ets2mp.dll");
	if (TruckersMP)
	{
		scs_log(1, "TruckersMP detected! This plugin is not supported on multiplayer servers and may result in a ban.");
		running_truckersmp = true;
	}



    //MH_Initialize();

    // Hook DX11
    scs_log(0, "Hooking rendering API...");

    kiero::RenderType::Enum render_type = kiero::RenderType::Auto;

    LaunchArgsManager* arguments = new LaunchArgsManager();

    if (arguments->has_arg("-rdevice")) {
        std::string device = to_lower(arguments->get_arg("-rdevice"));

        if (device == "dx11")
        {
            scs_log(0, "Render device override: D3D11");
            render_type = kiero::RenderType::D3D11;
        }
        else if (device == "dx12")
        {
            scs_log(0, "Render device override: D3D12");
            render_type = kiero::RenderType::D3D12;
        }
        else if (device == "gl")
        {
            scs_log(0, "Render device override: OpenGL");
            render_type = kiero::RenderType::OpenGL;
        }
        else if (device == "vk")
        {
            scs_log(0, "Render device override: Vulkan");
            render_type = kiero::RenderType::Vulkan;
        }
        else
        {
            scs_log(0, "Unknown render device override: '%s'", device.c_str());
            render_type = kiero::RenderType::Vulkan;
        }
    }

    renderer = new RenderManager(render_type);

    switch (renderer->GetRenderDevice())
    {
    case kiero::RenderType::D3D11:
        scs_log(0, "Rendering API: D3D11");
        break;
    case kiero::RenderType::D3D12:
        scs_log(0, "Rendering API: D3D12");
        break;
    case kiero::RenderType::OpenGL:
        scs_log(0, "Rendering API: OpenGL");
        break;
    case kiero::RenderType::Vulkan:
        scs_log(0, "Rendering API: Vulkan");
        break;
    }

    scs_log(0, "Hooked!");

    uintptr_t game_ctrl_ptr_instruction = bmem::patternScan("48 8B 0D ?? ?? ?? ?? 0F 57 C0 48 8B D0");
    if (!bmem::isAddressValid(game_ctrl_ptr_instruction))
    {
        scs_log(2, "Failed to find the game_ctrl pointer instruction!");

        is_fatal_error = true;
        error = "Failed to find the game_ctrl pointer instruction!";

        return SCS_RESULT_ok;
    }
    game_ctrl_ptr = bmem::relativeToAbsolute(game_ctrl_ptr_instruction, 3, 7);
    scs_log(0, "Found pointer to 'prism::game_ctrl' at: %p", game_ctrl_ptr);


    uintptr_t game_actor_offset_instruction = bmem::patternScan("48 8B 89 ?? ?? ?? ?? 48 8B DA 48 85 C9 74 ?? 48 8B 49");
    game_actor_offset = *reinterpret_cast<uint32_t*>(game_actor_offset_instruction + 3);
    scs_log(0, "Found 'game_ctrl->game_actor' offset ('%d')", game_actor_offset);
    scs_log(0, "Found pointer to 'prism::game_actor' at: %p", *(uint8_t**)game_ctrl_ptr + game_actor_offset);



    uintptr_t set_vehicle_steering_address = bmem::patternScan("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC ?? 8B 41 ?? 48 8B D9 0F 29 74");
    if (!bmem::isAddressValid(set_vehicle_steering_address))
    {
        scs_log(2, "Failed to find the 'prism::g_set_vehicle_steering' function!");

        is_fatal_error = true;
        error = "Failed to find the 'prism::g_set_vehicle_steering' function!";

        return SCS_RESULT_ok;
    }
    scs_log(0, "Found 'prism::g_set_vehicle_steering' at: %p", set_vehicle_steering_address);
    prism::g_set_vehicle_steering = (prism::g_set_vehicle_steering_t)set_vehicle_steering_address;


    uintptr_t advance_steering_address = bmem::patternScan("40 55 53 56 48 8D AC 24 ?? ?? ?? ?? 48 81 EC ?? ?? ?? ?? 48 8B 91");
    if (!bmem::isAddressValid(advance_steering_address))
    {
        scs_log(2, "Failed to find the 'prism::g_advance_steering' function!");

        is_fatal_error = true;
        error = "Failed to find the 'prism::g_advance_steering' function!";

        return SCS_RESULT_ok;
    }
    scs_log(0, "Found 'prism::g_advance_steering' at: %p", advance_steering_address);

    bmem::MH_Success(MH_CreateHook((LPVOID)advance_steering_address, g_advance_steering_hook, (LPVOID*)&g_advance_steering_original));
    bmem::MH_Success(MH_EnableHook((LPVOID)advance_steering_address));

    dinput_manager::InitDirectInput();
    for (auto& device : dinput_manager::GetInputDevices())
    {
        std::wstring wstr(device.info.tszInstanceName);
        std::string str(wstr.begin(), wstr.end());

        input_device inputdevice;
        inputdevice.name = str;
        inputdevice.di8_device = device.device;

        scs_log(0, "Added input device '%s'", str.c_str());

        input_devices.emplace_back(inputdevice);
    }

    scs_log(0, "Reading settings...");

    try {
        std::ifstream file("plugins/ETS2-TrailerUtil.json");

        std::ostringstream ss;
        ss << file.rdbuf();
        std::string file_content = ss.str();

        json settings = json::parse(file_content);

        axis_device_index               = settings["axis_device_index"];
        steering_axis_name              = settings["steering_axis_name"];
        steer_button_device_index       = settings["steer_button_device_index"];
        steer_left_button_name          = settings["steer_left_button_name"];
        steer_right_button_name         = settings["steer_right_button_name"];
        current_controlled_trailer      = settings["current_controlled_trailer"];
        use_axis_steering               = settings["use_axis_steering"];
        use_button_steering             = settings["use_button_steering"];

        last_axis_device_index          = axis_device_index;
        last_steering_axis_name         = steering_axis_name;
        last_steer_button_device_index  = steer_button_device_index;
        last_steer_left_button_name     = steer_left_button_name;
        last_steer_right_button_name    = steer_right_button_name;
        last_current_controlled_trailer = current_controlled_trailer;
        last_use_axis_steering          = use_axis_steering;
        last_use_button_steering        = use_button_steering;

        // Verify some values:
        if (axis_device_index < -1 || axis_device_index >= (int)input_devices.size()) {
			scs_log(1, "Warning: 'axis_device_index' is out of range, resetting to -1");
            axis_device_index = -1;
        }

		if (steer_button_device_index < -1 || steer_button_device_index >= (int)input_devices.size()) {
			scs_log(1, "Warning: 'steer_button_device_index' is out of range, resetting to -1");
			steer_button_device_index = -1;
		}

		file.close();
    }
    catch (...)
    {
        scs_log(2, "Failed to read settings!");
    }
    scs_log(0, "Done");

    scs_log(0, "Plugin Loaded");

    return SCS_RESULT_ok;
}

#pragma comment( linker, "/export:scs_telemetry_shutdown=scs_telemetry_shutdown" )
SCSAPI_VOID scs_telemetry_shutdown()
{
    dinput_manager::ShutdownDirectInput();

    delete renderer;

    scs_log(0, "Plugin Unloaded");
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    global::hModule = hinstDLL;

    return TRUE;
}