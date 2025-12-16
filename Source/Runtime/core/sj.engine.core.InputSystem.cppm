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
#include <SDL3/SDL_scancode.h>
#include <imgui_impl_sdl3.h>

// STD Includes
#include <cstddef>
#include <cmath>
#include <algorithm>
#include <type_traits>
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
    W = SDL_SCANCODE_W,
    A = SDL_SCANCODE_A,
    S = SDL_SCANCODE_S,
    D = SDL_SCANCODE_D,
};
inline constexpr size_t kNumKeyboardButtons = SDL_Scancode::SDL_SCANCODE_COUNT;

enum class GamepadAxis
{
    kLeftX = SDL_GAMEPAD_AXIS_LEFTX,
    kLeftY = SDL_GAMEPAD_AXIS_LEFTY,
    kRightX = SDL_GAMEPAD_AXIS_RIGHTX,
    kRightY = SDL_GAMEPAD_AXIS_RIGHTY,
    kLeftTrigger = SDL_GAMEPAD_AXIS_LEFT_TRIGGER,
    kRightTrigger = SDL_GAMEPAD_AXIS_RIGHT_TRIGGER,
    kLastAxis = SDL_GAMEPAD_AXIS_COUNT
};
inline constexpr size_t kNumJoystickAxes = static_cast<size_t>(GamepadAxis::kLastAxis);

enum class ButtonEvent
{
    kReleased = 0,
    kPressed = 1,
    kNumEvents = 2
};

template <class T>
concept AxisInput = requires {
    std::is_same_v<std::remove_cvref_t<T>, KeyboardButton> ||
        std::is_same_v<std::remove_cvref_t<T>, GamepadAxis>;
};

class InputSystem : public IModule
{
public:
    InputSystem(Program& program)
    {

    }

    void RegisterAxisBinding(string_hash axisName, AxisInput auto input, float modifier)
    {
        mInputAxes[axisName].bindings.emplace_back(input, modifier);
    }

    [[nodiscard]] float GetAxisValue(string_hash name) const
    {
        auto it = mInputAxes.find(name);
        SJ_ASSERT(it != mInputAxes.end(), "Failed to find input binding");
        return it->second.value;
    }

    [[nodiscard]] float GetAxisDefaultValue(string_hash name) const
    {
        auto it = mInputAxes.find(name);
        SJ_ASSERT(it != mInputAxes.end(), "Failed to find input binding");
        return it->second.value;
    }

    void Process(float _) override
    {
        std::span<const bool> keyboardState = GetKeyboardState();

        for(Axis& axis : mInputAxes.values())
        {
            axis.value = 0.0f;
            for(const Axis::Binding& binding : axis.bindings)
            {
                if(std::holds_alternative<KeyboardButton>(binding.input))
                {
                    const KeyboardButton button = std::get<KeyboardButton>(binding.input);
                    const bool pressed = keyboardState.at(static_cast<size_t>(button));
                    if(pressed)
                        axis.value += binding.modifier;
                }
                // else if(std::holds_alternative<GamepadAxis>(binding.input))
                // {
                //     axis.value = GetAxisValue(0, std::get<GamepadAxis>( binding.input ));
                // }
            }
        }
    }

private:
    struct Axis
    {
        float value = 0.0f;

        struct Binding
        {
            std::variant<KeyboardButton, GamepadAxis> input;
            float modifier = 0.0f;
        };
        sj::dynamic_vector<Binding> bindings;
    };

    std::span<const bool> GetKeyboardState() const
    {
        int numKeys = -1;
        const bool* keys = SDL_GetKeyboardState(&numKeys);

        return std::span(keys, numKeys);
    }

    float GetAxisValue(int gamepadIndex, GamepadAxis axis)
    {
        return 0.0f;
    }

    sj::dynamic_flat_map<string_hash, Axis> mInputAxes;
};
} // namespace sj
