module;
#include <SDL3/SDL.h>

export module sj.engine.config.InputConfig;
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

template <class InputSource>
struct AxisBinding
{
    InputSource input = {};
    float modifier = 0.0f;
};

template <class InputSource>
using AxisBindings = sj::dynamic_vector<AxisBinding<InputSource>>;

struct InputBindings
{
    sj::dynamic_flat_map<hashed_string_sv, AxisBindings<KeyboardButton>> keyboard_axes;
    sj::dynamic_flat_map<hashed_string_sv, AxisBindings<GamepadAxis>> gamepad_axes;
};

} // namespace sj