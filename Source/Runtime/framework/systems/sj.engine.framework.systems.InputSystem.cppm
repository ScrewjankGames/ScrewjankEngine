module;

// Engine Includes
#include <ScrewjankStd/Log.hpp>
#include <ScrewjankStd/PlatformDetection.hpp>

// Library Includes
#include <GLFW/glfw3.h>
#ifndef SJ_GOLD
    #include <imgui.h>
#endif // !SJ_GOLD

// STD Includes
#include <cmath>
#include <algorithm>

export module sj.engine.framework.systems.InputSystem;
import sj.std.math;

export namespace sj
{
    // enum PadButtons
    //{
    //     kCross = GLFW_GAMEPAD_BUTTON_CROSS,
    //     kCircle = GLFW_GAMEPAD_BUTTON_CIRCLE,
    //     kSquare = GLFW_GAMEPAD_BUTTON_SQUARE,
    //     kTriangle = GLFW_GAMEPAD_BUTTON_SQUARE
    // };

    class InputSystem
    {
    public:
        InputSystem() = default;

        void Process()
        {
            constexpr float kDeadZone = 0.15f;
            constexpr float kDeadZoneSqr = kDeadZone * kDeadZone;

            GLFWgamepadstate state = {};
            auto connected = glfwGetGamepadState(GLFW_JOYSTICK_1, &state);

            const Vec2 rawLeftStick = {state.axes[GLFW_GAMEPAD_AXIS_LEFT_X],
                                       state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]};

            const Vec2 rawRightStick = {state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X],
                                        state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]};

            auto applyDeadZoneFn = [](const Vec2& stick) -> Vec2 {
                float stickMagnitudeSqr = MagnitudeSqr(stick);
                if(stickMagnitudeSqr < kDeadZoneSqr)
                {
                    return Vec2_Zero;
                }
                else
                {
                    const float magnitude = std::sqrtf(stickMagnitudeSqr);
                    Vec2 res = (stick / magnitude) * (magnitude - kDeadZone) / (1.0f - kDeadZone);

                    res.SetX(std::clamp<float>(res.GetX(), -1, 1));
                    res.SetY(std::clamp<float>(res.GetY(), -1, 1));

                    return res;
                }
            };

            s_publishedLeftStick = applyDeadZoneFn(rawLeftStick);
            s_publishedRightStick = applyDeadZoneFn(rawRightStick);

#ifndef SJ_GOLD
            if(ImGui::Begin("Input") && connected == GLFW_TRUE)
            {
                ImGui::Text("Raw %s:", glfwGetGamepadName(GLFW_JOYSTICK_1));
                ImGui::Text("Left:  [%f, %f]", rawLeftStick.GetX(), rawLeftStick.GetY());
                ImGui::Text("Right: [%f, %f]", rawRightStick.GetX(), rawRightStick.GetY());

                ImGui::Text("Published %s: ", glfwGetGamepadName(GLFW_JOYSTICK_1));
                ImGui::Text("Left:  [%f, %f]",
                            s_publishedLeftStick.GetX(),
                            s_publishedLeftStick.GetY());
                ImGui::Text("Right: [%f, %f]",
                            s_publishedRightStick.GetX(),
                            s_publishedRightStick.GetY());
            }
            ImGui::End();
#endif // !SJ_GOLD
        }

        static const Vec2& GetLeftStick()
        {
            return s_publishedLeftStick;
        }

        static const Vec2& GetRightStick()
        {
            return s_publishedRightStick;
        }

    private:
        static Vec2 s_publishedLeftStick;
        static Vec2 s_publishedRightStick;
    };
} // namespace sj

namespace sj {
    Vec2 InputSystem::s_publishedLeftStick = {};
    Vec2 InputSystem::s_publishedRightStick = {};
}
