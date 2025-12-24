module;
#include <glaze/core/opts.hpp>
#include <glaze/json/write.hpp>

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

import sj.datadefs.Serialization;

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

class InputSystem : public IModule
{
public:
    InputSystem(Program& program)
    {
    }

    void RegisterAxisBinding(string_hash axisName, GamepadAxis input, float modifier)
    {
        mBindings.gamepad_axes[axisName].emplace_back(input, modifier);
        auto outputIt = mInputAxes.find(axisName);
        if(outputIt == mInputAxes.end())
            mInputAxes[axisName] = 0.0f;
    }

    void RegisterAxisBinding(string_hash axisName, KeyboardButton input, float modifier)
    {
        mBindings.keyboard_axes[axisName].emplace_back(input, modifier);
        auto outputIt = mInputAxes.find(axisName);
        if(outputIt == mInputAxes.end())
            mInputAxes[axisName] = 0.0f;
    }

    [[nodiscard]] float GetAxisValue(string_hash name) const
    {
        auto it = mInputAxes.find(name);
        SJ_ASSERT(it != mInputAxes.end(), "Failed to find input binding");
        return it->second;
    }

    void Process(float _) override
    {
        std::span<const bool> keyboardState = GetKeyboardState();

        for(auto&& [axis_name, axis_value] : mInputAxes)
        {
            axis_value = PollKeyboardAxis(axis_name)
                             .or_else([this, axis_name]() {
                                 return PollGamepadAxis(axis_name);
                             })
                             .value_or(0.0f);
        }
    }

    void DebugLogBindings()
    {
        std::string str;
        glz::error_ctx ctx = glz::write_json(mBindings, str);
        SJ_ENGINE_LOG_INFO("{}", str);
    }

private:
    template <class InputSource>
    struct AxisBinding
    {
        InputSource input = {};
        float modifier = 0.0f;
    };

    template <class InputSource>
    using AxisBindings = sj::dynamic_vector<AxisBinding<InputSource>>;

    std::span<const bool> GetKeyboardState() const
    {
        int numKeys = -1;
        const bool* keys = SDL_GetKeyboardState(&numKeys);

        return std::span(keys, numKeys);
    }

    std::optional<float> PollKeyboardAxis(string_hash axisName)
    {
        std::span<const bool> keyboardState = GetKeyboardState();
        std::optional<float> axisValue;

        // Poll Keyboard
        auto keyboardBindingsIt = mBindings.keyboard_axes.find(axisName);
        if(keyboardBindingsIt != mBindings.keyboard_axes.end())
        {
            const AxisBindings<KeyboardButton>& bindings = keyboardBindingsIt->second;
            for(const AxisBinding<KeyboardButton>& binding : bindings)
            {
                const bool active = keyboardState.at(static_cast<size_t>(binding.input));
                if(active)
                    axisValue = axisValue.value_or(0.0f) + binding.modifier;
            }
        }

        return axisValue;
    }

    std::optional<float> PollGamepadAxis(string_hash axisName)
    {
        return {};
    }

    struct Bindings
    {
        sj::dynamic_flat_map<string_hash, AxisBindings<KeyboardButton>> keyboard_axes;
        sj::dynamic_flat_map<string_hash, AxisBindings<GamepadAxis>> gamepad_axes;
    } mBindings;

    sj::dynamic_flat_map<string_hash, float> mInputAxes;
};
} // namespace sj
