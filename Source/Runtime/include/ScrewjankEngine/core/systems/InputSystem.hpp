#pragma once

// Shared Includes
#include <ScrewjankShared/Math/Vec2.hpp>

namespace sj
{
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