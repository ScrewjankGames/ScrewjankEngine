
module;

// Engine Includes
#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankStd/Log.hpp>
#include <ScrewjankStd/PlatformDetection.hpp>

// Library Includes
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_joystick.h>
#include <SDL3/SDL_keycode.h>
#include <imgui_impl_sdl3.h>

// STD Includes
#include <cstddef>
#include <cmath>
#include <algorithm>
#include <variant>

export module sj.engine.core.InputSystem;
import sj.engine.core.Program;
import sj.engine.core.Window;

import sj.std.math;
import sj.std.string_hash;
import sj.std.containers.map;
import sj.std.containers.vector;

export namespace sj
{
    enum class KeyboardButton
    {
        W = SDLK_W,
        A = SDLK_A,
        S = SDLK_S,
        D = SDLK_D,
    };

    enum class JoystickAxis
    {
        kLeftX = SDL_GAMEPAD_AXIS_LEFTX,
        kLeftY = SDL_GAMEPAD_AXIS_LEFTY,
        kRightX = SDL_GAMEPAD_AXIS_RIGHTX,
        kRightY = SDL_GAMEPAD_AXIS_RIGHTY,
        kLeftTrigger = SDL_GAMEPAD_AXIS_LEFT_TRIGGER,
        kRightTrigger = SDL_GAMEPAD_AXIS_RIGHT_TRIGGER,
        kLastAxis = SDL_GAMEPAD_AXIS_COUNT
    };

    enum class ButtonEvent
    {
        kReleased = 0,
        kPressed = 1,
        kNumEvents = 2
    };

    class InputSystem : public IModule
    {
    public:
        InputSystem(Program& _)
        {
        }

        void RegisterAxis(string_hash axis, float defaultValue)
        {
            m_inputAxes[axis] = Axis {.value = defaultValue, .defaultValue = defaultValue};
        }

        void RegisterAxisBinding(string_hash axisName, KeyboardButton button, float effect)
        {
            m_keyAxisBindings[button][static_cast<int>(ButtonEvent::kPressed)] =
                AxisBinding {.bindingName = axisName, .effect = effect};

            m_keyAxisBindings[button][static_cast<int>(ButtonEvent::kReleased)] =
                AxisBinding {.bindingName = axisName, .effect = -effect};
        }

        void RegisterAxisBinding(string_hash axisName, JoystickAxis stick, float multiplier = 1.0f)
        {
            m_joystickAxisBindings[static_cast<int>(stick)] =
                AxisBinding {.bindingName = axisName, .effect = multiplier};
        }

        [[nodiscard]] float GetAxisValue(string_hash name) const
        {
            auto it = m_inputAxes.find(name);
            SJ_ASSERT(it != m_inputAxes.end(), "Failed to find input binding");
            return it->second.value;
        }

        [[nodiscard]] float GetAxisDefaultValue(string_hash name) const
        {
            auto it = m_inputAxes.find(name);
            SJ_ASSERT(it != m_inputAxes.end(), "Failed to find input binding");
            return it->second.value;
        }

        void Process(float _) override
        {
            // constexpr float kDeadZone = 0.15f;
            // constexpr float kDeadZoneSqr = kDeadZone * kDeadZone;

            // std::span<SDL_JoystickID> gamepads = [](){
            //     int count = 0;
            //     SDL_JoystickID* ids = SDL_GetGamepads(&count);
            //     return std::span{ids, static_cast<size_t>(count)};
            // }();

            // if(gamepads.size() == 0)
            //     return;

            // GLFWgamepadstate state = {};
            // auto connected = glfwGetGamepadState(GLFW_JOYSTICK_1, &state);
            // if(connected == GLFW_TRUE)
            // {
            //     for(int i = 0; i < static_cast<int>(JoystickAxis::kLastAxis); i++)
            //     {
            //         const AxisBinding& binding = m_joystickAxisBindings[i];
            //         if(binding.bindingName == string_hash {})
            //             continue;

            //         auto it = m_inputAxes.find(binding.bindingName);

            //         if(it == m_inputAxes.end())
            //             continue;

            //         it->second.value = state.axes[i];
            //     }
            // }
        }

    private:
        using AnyBinding = std::variant<KeyboardButton, JoystickAxis, std::monostate>;

        struct AxisBinding
        {
            string_hash bindingName = {};
            float effect = 0.0f;
        };

        struct Axis
        {
            float value = 0.0f;
            float defaultValue = 0.0f;
        };

        void HandleKeyEvent(SDL_KeyboardEvent& event)
        {
            KeyboardButton sjKey = static_cast<KeyboardButton>(event.key);
            ButtonEvent action = event.type == SDL_EVENT_KEY_DOWN ? ButtonEvent::kPressed : ButtonEvent::kReleased;
                
            auto bindingIt = m_keyAxisBindings.find(sjKey);
            if(bindingIt != m_keyAxisBindings.end())
            {
                const AxisBinding& binding = bindingIt->second[static_cast<int>(action)];
                if(binding.bindingName != string_hash {})
                {
                    m_inputAxes[binding.bindingName].value += binding.effect;
                }
            }
        }

        static constexpr size_t kNumEvents = static_cast<size_t>(ButtonEvent::kNumEvents);

        sj::dynamic_flat_map<KeyboardButton, std::array<AxisBinding, kNumEvents>> m_keyAxisBindings;
        std::array<AxisBinding, static_cast<size_t>(JoystickAxis::kLastAxis)>
            m_joystickAxisBindings = {};

        sj::dynamic_flat_map<string_hash, Axis> m_inputAxes;
    };
} // namespace sj
