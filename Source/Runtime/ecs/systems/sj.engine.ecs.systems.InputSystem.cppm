module;

// Engine Includes
#include <ScrewjankStd/Assert.hpp>
#include <ScrewjankStd/Log.hpp>
#include <ScrewjankStd/PlatformDetection.hpp>

// Library Includes
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>

#ifndef SJ_GOLD
    #include <imgui.h>
#endif // !SJ_GOLD

// STD Includes
#include <cmath>
#include <algorithm>
#include <variant>

export module sj.engine.ecs.systems.InputSystem;
import sj.std.math;
import sj.std.string_hash;
import sj.std.containers.map;
import sj.std.containers.vector;
import sj.engine.framework.Window;

export namespace sj
{
    enum class KeyboardButton
    {
        W = GLFW_KEY_W,
        A = GLFW_KEY_A,
        S = GLFW_KEY_S,
        D = GLFW_KEY_D,
    };

    enum class JoystickAxis
    {
        kLeftX = GLFW_GAMEPAD_AXIS_LEFT_X,
        kLeftY = GLFW_GAMEPAD_AXIS_LEFT_Y,
        kRightX = GLFW_GAMEPAD_AXIS_RIGHT_X,
        kRightY = GLFW_GAMEPAD_AXIS_RIGHT_Y,
        kLeftTrigger = GLFW_GAMEPAD_AXIS_LEFT_TRIGGER,
        kRightTrigger = GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER,
        kLastAxis = GLFW_GAMEPAD_AXIS_LAST
    };

    enum class ButtonEvent
    {
        kReleased = GLFW_RELEASE,
        kPressed = GLFW_PRESS,
        kRepeat = GLFW_REPEAT,
        kNumEvents = 3
    };

    class InputSystem
    {
    public:
        InputSystem(Window& display)
        {
            glfwSetKeyCallback(display.GetWindowHandle(), KeyCallback);
            glfwSetWindowUserPointer(display.GetWindowHandle(), this);
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

        void Process()
        {
            // Poll events, trigger callbacks
            glfwPollEvents();
            ImGui_ImplGlfw_NewFrame();

            constexpr float kDeadZone = 0.15f;
            constexpr float kDeadZoneSqr = kDeadZone * kDeadZone;

            GLFWgamepadstate state = {};
            auto connected = glfwGetGamepadState(GLFW_JOYSTICK_1, &state);
            if(connected == GLFW_TRUE)
            {
                for(int i = 0; i < static_cast<int>(JoystickAxis::kLastAxis); i++)
                {
                    const AxisBinding& binding = m_joystickAxisBindings[i];
                    if(binding.bindingName == string_hash {})
                        continue;

                    auto it = m_inputAxes.find(binding.bindingName);

                    if(it == m_inputAxes.end())
                        continue;

                    it->second.value = state.axes[i];
                }
            }
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

        static void KeyCallback([[maybe_unused]] GLFWwindow* window,
                                int glfw_key,
                                [[maybe_unused]] int scancode,
                                int action,
                                [[maybe_unused]] int mods)
        {
            InputSystem* instance =
                reinterpret_cast<InputSystem*>(glfwGetWindowUserPointer(window));

            SJ_ASSERT(instance != nullptr, "GLFW key callback can't find input system instance!");

            KeyboardButton sjKey = static_cast<KeyboardButton>(glfw_key);

            auto bindingIt = instance->m_keyAxisBindings.find(sjKey);
            if(bindingIt != instance->m_keyAxisBindings.end())
            {
                const AxisBinding& binding = bindingIt->second[action];
                if(binding.bindingName != string_hash {})
                {
                    instance->m_inputAxes[binding.bindingName].value += binding.effect;
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
