#pragma once

// Shared Includes
#include <ScrewjankShared/Math/Vec2.hpp>

// Library Includes

namespace sj
{
    //enum PadButtons
    //{
    //    kCross = GLFW_GAMEPAD_BUTTON_CROSS,
    //    kCircle = GLFW_GAMEPAD_BUTTON_CIRCLE,
    //    kSquare = GLFW_GAMEPAD_BUTTON_SQUARE,
    //    kTriangle = GLFW_GAMEPAD_BUTTON_SQUARE
    //};

    class InputSystem
    {
    public:

        InputSystem() = default;

        void Process();

        static const Vec2& GetLeftStick();
        static const Vec2& GetRightStick();

    private:
        static Vec2 s_publishedLeftStick;
        static Vec2 s_publishedRightStick;
    };
}