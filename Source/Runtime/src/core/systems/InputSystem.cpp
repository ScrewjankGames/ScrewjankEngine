// Parent Include
#include <ScrewjankEngine/core/systems/InputSystem.hpp>

// Engine Includes
#include <ScrewjankEngine/utils/Log.hpp>
#include <ScrewjankEngine/platform/PlatformDetection.hpp>

// Library Includes
#include <imgui.h>
#include <GLFW/glfw3.h>

// STD Includes
#include <span>
#include <cmath>


namespace sj
{
    Vec2 InputSystem::s_publishedLeftStick = {};
    Vec2 InputSystem::s_publishedRightStick = {};

    void InputSystem::Process()
    {
        constexpr float kDeadZone = 0.15f;
        constexpr float kDeadZoneSqr = kDeadZone * kDeadZone;

        GLFWgamepadstate state = {};
        auto connected = glfwGetGamepadState(GLFW_JOYSTICK_1, &state);

        const Vec2 rawLeftStick = {
            state.axes[GLFW_GAMEPAD_AXIS_LEFT_X],
            state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]
        };

        const Vec2 rawRightStick = {
            state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X],
            state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]
        };

        auto applyDeadZoneFn = [](const Vec2& stick) -> Vec2 {
            float stickMagnitudeSqr = MagnitudeSqr(stick);
            if(stickMagnitudeSqr < kDeadZoneSqr)
            {
                return Vec2::Zero;
            }
            else
            {
                const float magnitude = std::sqrtf(stickMagnitudeSqr);
                Vec2 res = (stick / magnitude) * (magnitude - kDeadZone) / (1.0f - kDeadZone);

                res[0] = std::clamp<float>(res[0], -1, 1);
                res[1] = std::clamp<float>(res[1], -1, 1);
                
                return res;
            }
        };

        s_publishedLeftStick = applyDeadZoneFn(rawLeftStick);
        s_publishedRightStick = applyDeadZoneFn(rawRightStick);

        if constexpr(g_IsDebugBuild)
        {
            if(ImGui::Begin("Input") && connected == GLFW_TRUE)
            {
                ImGui::Text("Raw %s:", glfwGetGamepadName(GLFW_JOYSTICK_1));
                ImGui::Text("Left:  [%f, %f]", rawLeftStick[0], rawLeftStick[1]);
                ImGui::Text("Right: [%f, %f]", rawRightStick[0], rawRightStick[1]);

                ImGui::Text("Published %s: ", glfwGetGamepadName(GLFW_JOYSTICK_1));
                ImGui::Text("Left:  [%f, %f]", s_publishedLeftStick[0], s_publishedLeftStick[1]);
                ImGui::Text("Right: [%f, %f]", s_publishedRightStick[0], s_publishedRightStick[1]);
            }
            ImGui::End();
        }
    }
    
    const Vec2& InputSystem::GetLeftStick()
    {
        return s_publishedLeftStick;
    }

    const Vec2& InputSystem::GetRightStick()
    {
        return s_publishedRightStick;
    }

} // namespace sj