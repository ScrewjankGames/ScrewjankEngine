module;

// Library Includes
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_joystick.h>
#include <SDL3/SDL_scancode.h>
#include <imgui_impl_sdl3.h>

#include <span>
#include <optional>
export module sj.engine.InputSystem;
import sj.engine.Program;
import sj.engine.Window;

import sj.engine.config.InputConfig;

import sj.std.primitives;
import sj.std.math;
import sj.std.string_hash;
import sj.std.containers.map;
import sj.std.containers.vector;

export namespace sj
{

class InputSystem
{
public:
    InputSystem() = default;

    void Initialize(const InputBindings& bindings);

    bool ProcessEvent(const SDL_Event& evt);

    [[nodiscard]] float GetAxisValue(string_hash name) const;

    [[nodiscard]] vec2 GetAxisValue2D(string_hash name) const;

    // TODO: see
    // https://blog.hypersect.com/interpreting-analog-sticks/?_sp=e3a301c9-0451-4b5d-9651-0f353db0d26f.1790467267574
    void Process(float _);

private:
    void OnGamepadConnected(SDL_JoystickID id);
    void OnGamepadDisconnected(SDL_JoystickID id);

    std::span<const bool> GetKeyboardState() const;

    std::optional<float> PollKeyboardAxis(string_hash axisName);
    std::optional<vec2> PollKeyboardAxis2D(string_hash axisName);

    std::optional<float> PollGamepadAxis(string_hash axisName);
    std::optional<vec2> PollGamepadAxis2D(string_hash axisName);

    const InputBindings* mBindings = nullptr;
    sj::dynamic_flat_map<string_hash, float> mInputAxes;
    sj::dynamic_flat_map<string_hash, vec2> mInputAxes2D;

    SDL_Gamepad* mGamepad = nullptr;
};
} // namespace sj
