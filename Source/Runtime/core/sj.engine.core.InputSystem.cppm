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
#include <ranges>

export module sj.engine.core.InputSystem;
import sj.engine.core.Program;
import sj.engine.core.Window;

import sj.engine.config.InputConfig;

import sj.std.string_hash;
import sj.std.containers.map;
import sj.std.containers.vector;

export namespace sj
{

class InputSystem
{
public:
    InputSystem() = default;

    void Initialize(const InputBindings& bindings)
    {
        mBindings = &bindings;

        auto allAxes =
            std::views::concat(bindings.keyboard_axes.keys(), bindings.gamepad_axes.keys());

        for(const hashed_string_sv& axis : allAxes)
            mInputAxes[axis.get_hash()] = 0;
    }

    // void RegisterAxisBinding(string_hash axisName, GamepadAxis input, float modifier)
    // {
    //     mBindings.gamepad_axes[axisName].emplace_back(input, modifier);
    //     auto outputIt = mInputAxes.find(axisName);
    //     if(outputIt == mInputAxes.end())
    //         mInputAxes[axisName] = 0.0f;
    // }

    // void RegisterAxisBinding(string_hash axisName, KeyboardButton input, float modifier)
    // {
    //     mBindings.keyboard_axes[axisName].emplace_back(input, modifier);
    //     auto outputIt = mInputAxes.find(axisName);
    //     if(outputIt == mInputAxes.end())
    //         mInputAxes[axisName] = 0.0f;
    // }

    [[nodiscard]] float GetAxisValue(string_hash name) const
    {
        auto it = mInputAxes.find(name);
        SJ_ASSERT(it != mInputAxes.end(), "Failed to find input binding");
        return it->second;
    }

    void Process(float _)
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

private:
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
        auto keyboardBindingsIt = mBindings->keyboard_axes.find(hashed_string_sv(axisName));
        if(keyboardBindingsIt != mBindings->keyboard_axes.end())
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

    const InputBindings* mBindings = nullptr;
    sj::dynamic_flat_map<string_hash, float> mInputAxes;
};
} // namespace sj
