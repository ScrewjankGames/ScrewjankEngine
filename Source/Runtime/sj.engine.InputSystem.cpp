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

// STD Includes
#include <algorithm>
#include <ranges>
#include <cmath>
#include <span>

module sj.engine.InputSystem;
import sj.engine.config.InputConfig;

namespace sj
{
void InputSystem::Initialize(const InputBindings& bindings)
{
    mBindings = &bindings;

    // 1D axes
    {
        for(const hashed_string_sv& axis : bindings.keyboard_axes.keys())
            mInputAxes[axis.get_hash()] = 0;
    
        for(const hashed_string_sv& axis : bindings.gamepad_axes.keys())
            mInputAxes[axis.get_hash()] = 0;
    }

    // 2D Axes
    {
        for(const hashed_string_sv& axis : bindings.keyboard_axes_2D.keys())
            mInputAxes2D[axis.get_hash()] = vec2(0.0f, 0.0f);
    
        for(const hashed_string_sv& axis : bindings.gamepad_axes_2D.keys())
            mInputAxes2D[axis.get_hash()] = vec2(0.0f, 0.0f);
    }   
}

bool InputSystem::ProcessEvent(const SDL_Event& evt)
{
    switch(evt.type)
    {
        case SDL_EVENT_KEY_UP:
        case SDL_EVENT_KEY_DOWN:
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_MOTION:
            break;
        case SDL_EVENT_GAMEPAD_ADDED:
            OnGamepadConnected(evt.gdevice.which);
            break;
        case SDL_EVENT_GAMEPAD_REMOVED:
            OnGamepadDisconnected(evt.gdevice.which);
            break;
    }

    return false;
}

float InputSystem::GetAxisValue(string_hash name) const
{
    auto it = mInputAxes.find(name);
    SJ_ASSERT(it != mInputAxes.end(), "Failed to find input binding");
    return it->second;
}

vec2 InputSystem::GetAxisValue2D(string_hash name) const
{
    auto it = mInputAxes2D.find(name);
    SJ_ASSERT(it != mInputAxes2D.end(), "Failed to find input binding");
    return it->second;
}

void InputSystem::Process(float _)
{
    // Process keyboard input
    {
        std::span<const bool> keyboardState = GetKeyboardState();
        for(auto&& [axis_name, axis_value] : mInputAxes)
            axis_value = PollKeyboardAxis(axis_name).value_or(0.0f);

        for(auto&& [axis_name, axis_value] : mInputAxes2D)
            axis_value = PollKeyboardAxis2D(axis_name).value_or(vec2(0.0f, 0.0f));
    }

    if(mGamepad)
    {
        for(auto&& [axis_name, axis_value] : mInputAxes)
            axis_value = PollGamepadAxis(axis_name).value_or(0.0f);

        for(auto&& [axis_name, axis_value] : mInputAxes2D)
            axis_value = PollGamepadAxis2D(axis_name).value_or(vec2(0.0f, 0.0f));
    }
}

void InputSystem::OnGamepadConnected(SDL_JoystickID id)
{
    SJ_ENGINE_LOG_INFO("Gamepad connecting");
    if(mGamepad == nullptr)
    {
        mGamepad = SDL_OpenGamepad(id);
        SJ_ENGINE_LOG_INFO("Gamepad added: {}", SDL_GetGamepadName(mGamepad));
    }
}

void InputSystem::OnGamepadDisconnected(SDL_JoystickID id)
{
    SJ_ENGINE_LOG_INFO("Gamepad disconnecting");
    if(id == SDL_GetGamepadID(mGamepad))
    {
        SJ_ENGINE_LOG_INFO("Gamepad removed: {}", SDL_GetGamepadName(mGamepad));
        SDL_CloseGamepad(mGamepad);
        mGamepad = nullptr;
    }
}

std::span<const bool> InputSystem::GetKeyboardState() const
{
    int numKeys = -1;
    const bool* keys = SDL_GetKeyboardState(&numKeys);

    return std::span(keys, numKeys);
}

std::optional<float> InputSystem::PollKeyboardAxis(string_hash axisName)
{
    auto keyboardBindingsIt = mBindings->keyboard_axes.find(hashed_string_sv(axisName));
    if(keyboardBindingsIt == mBindings->keyboard_axes.end())
        return std::nullopt;

    std::optional<float> axisValue;
    std::span<const bool> keyboardState = GetKeyboardState();

    const AxisBindings<AxisBinding<KeyboardButton>>& bindings = keyboardBindingsIt->second;
    for(const AxisBinding<KeyboardButton>& binding : bindings)
    {
        const bool active = keyboardState[static_cast<size_t>(binding.input)];
        if(active)
            axisValue = axisValue.value_or(0.0f) + binding.modifier;
    }

    return axisValue;
}

std::optional<vec2> InputSystem::PollKeyboardAxis2D(string_hash axisName)
{
    auto keyboardBindingsIt = mBindings->keyboard_axes_2D.find(hashed_string_sv(axisName));
    if(keyboardBindingsIt == mBindings->keyboard_axes_2D.end())
        return std::nullopt;

    std::optional<vec2> axisValue;
    std::span<const bool> keyboardState = GetKeyboardState();

    const AxisBindings<KeyboardAxisBinding2D>& bindings = keyboardBindingsIt->second;
    for(const KeyboardAxisBinding2D& binding : bindings)
    {
        const bool xActiveL = keyboardState[static_cast<size_t>(binding.x_input_l)];
        const bool xActiveR = keyboardState[static_cast<size_t>(binding.x_input_r)];
        const float x = (-1.0f * xActiveL) + (1.0f * xActiveR);

        const bool yActiveL = keyboardState[static_cast<size_t>(binding.y_input_l)];
        const bool yActiveR = keyboardState[static_cast<size_t>(binding.y_input_r)];
        
        const float y = (1.0f * yActiveL) + (-1.0f * yActiveR);
        
        if(x == 0.0f && y == 0.0f)
            continue;

        axisValue = axisValue.value_or(vec2(0.0f, 0.0f)) + normalized( vec2(x, y) );
    }

    return axisValue;
}

std::optional<float> InputSystem::PollGamepadAxis(string_hash axisName)
{
    auto gamepadBindingsIt = mBindings->gamepad_axes.find(hashed_string_sv(axisName));
    if(gamepadBindingsIt == mBindings->gamepad_axes.end())
        return std::nullopt;

    std::optional<float> axisValue;
    const AxisBindings<AxisBinding<GamepadAxis>>& bindings = gamepadBindingsIt->second;
    for(const AxisBinding<GamepadAxis>& binding : bindings)
    {
        const i16 sample =
            SDL_GetGamepadAxis(mGamepad, static_cast<SDL_GamepadAxis>(binding.input));
        const float normalizedValue = float(sample) / std::numeric_limits<i16>::max();
        const float modifiedValue = normalizedValue * binding.modifier;
        axisValue = axisValue.value_or(0.0f) + modifiedValue;
    }

    return axisValue;
}

std::optional<vec2> InputSystem::PollGamepadAxis2D(string_hash axisName)
{
    auto gamepadBindingsIt = mBindings->gamepad_axes_2D.find(hashed_string_sv(axisName));
    if(gamepadBindingsIt == mBindings->gamepad_axes_2D.end())
        return std::nullopt;

    std::optional<vec2> axisValue;

    constexpr float kInvStickMax = 1.0f / std::numeric_limits<i16>::max();

    const AxisBindings<GamepadAxisBinding2D>& bindings = gamepadBindingsIt->second;
    for(const GamepadAxisBinding2D& binding : bindings)
    {
        const i16 xSample =
            SDL_GetGamepadAxis(mGamepad, static_cast<SDL_GamepadAxis>(binding.x_input));
        const i16 ySample =
            SDL_GetGamepadAxis(mGamepad, static_cast<SDL_GamepadAxis>(binding.y_input));
        const float rawX = float(xSample) * kInvStickMax * binding.x_modifier;
        const float rawY = float(ySample) * kInvStickMax * binding.y_modifier;
        const vec2 rawInput(rawX, rawY);

        const float rawMagnitude = magnitude(rawInput);
        const float outputMagnitude = [&] -> float {
            if(rawMagnitude <= binding.inner_radial_deadzone)
            {
                return 0.0f;
            }
            else if(rawMagnitude >= binding.outer_radial_deadzone)
            {
                return 0.0f;
            }
            else
            {
                float livezoneWidth = binding.outer_radial_deadzone - binding.inner_radial_deadzone;
                float t = (rawMagnitude - binding.inner_radial_deadzone) / livezoneWidth;
                return std::lerp(0.0f, 1.0f, t);
            }
        }();
        
        vec2 remappedInput = normalized(rawInput) * outputMagnitude;

        axisValue = axisValue.value_or(vec2(0.0f, 0.0f)) + vec2(rawX, rawY);
    }

    return axisValue;
}

} // namespace sj