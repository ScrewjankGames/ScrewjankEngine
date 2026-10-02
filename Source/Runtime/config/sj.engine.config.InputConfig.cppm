module;
#include <SDL3/SDL.h>
#include <SDL3/SDL_scancode.h>

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
    E = SDL_SCANCODE_E,
    SPACE = SDL_SCANCODE_SPACE,
    LCTRL = SDL_SCANCODE_LCTRL
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

struct KeyboardAxisBinding2D
{
    KeyboardButton x_input_l = {};
    KeyboardButton x_input_r = {};
    
    KeyboardButton y_input_l = {};
    KeyboardButton y_input_r = {};
};

struct GamepadAxisBinding2D
{
    GamepadAxis x_input {};
    float x_modifier = 1.0f;
    
    GamepadAxis y_input {};
    float y_modifier = 1.0f;

    float inner_radial_deadzone = 0.075f;
    float outer_radial_deadzone = 0.925f;
};

template<class Binding>
using AxisBindings = sj::dynamic_vector<Binding>;

struct InputBindings
{
    sj::dynamic_flat_map<hashed_string_sv, AxisBindings<AxisBinding<KeyboardButton>>> keyboard_axes;
    sj::dynamic_flat_map<hashed_string_sv, AxisBindings<KeyboardAxisBinding2D>> keyboard_axes_2D;

    sj::dynamic_flat_map<hashed_string_sv, AxisBindings<AxisBinding<GamepadAxis>>> gamepad_axes;
    sj::dynamic_flat_map<hashed_string_sv, AxisBindings<GamepadAxisBinding2D>> gamepad_axes_2D;

};

} // namespace sj