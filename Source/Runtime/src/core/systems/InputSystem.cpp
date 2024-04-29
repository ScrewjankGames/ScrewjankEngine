// Library Headers
#include <GLFW/glfw3.h>

// Parent Include
#include <ScrewjankEngine/core/systems/InputSystem.hpp>

// STD Includes
#include <span>
#include <cmath>

// Engine Includes
#include <ScrewjankEngine/utils/Log.hpp>

// Library Includes
#include <imgui.h>

namespace sj
{
    void InputSystem::Process()
    {
        constexpr float kDeadZone = 0.0f;

        int count;

        GLFWgamepadstate state = {};
        auto connected = glfwGetGamepadState(GLFW_JOYSTICK_1, &state);

        if constexpr(g_IsDebugBuild)
        {
            if(ImGui::Begin("Input") && connected == GLFW_TRUE)
            {
                ImGui::TextUnformatted(glfwGetGamepadName(GLFW_JOYSTICK_1));
                ImGui::Text("LeftX: %f", state.axes[GLFW_GAMEPAD_AXIS_LEFT_X]);
                ImGui::Text("LeftY: %f", state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]);
                ImGui::Text("RightX: %f", state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X]);
                ImGui::Text("RightY: %f", state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]);
            }
            ImGui::End();
        }


    }
} // namespace sj