// Parent Include
#include <ScrewjankEngine/core/systems/InputSystem.hpp>

// Library Headers
#include <GLFW/glfw3.h>

// STD Includes
#include <span>
#include <cmath>

// Engine Includes
#include <ScrewjankEngine/utils/Log.hpp>

namespace sj
{
    void InputSystem::Process()
    {
        constexpr float kDeadZone = 0.0f;

        int count;
        const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &count);

        for(int i = 0; i < count; i++)
        {
            float axis = axes[i];

            if(std::fabsf(axis) > kDeadZone)
            {
                //SJ_ENGINE_LOG_INFO("Axis {}: {}", i, axis);
            }
        }
    }
} // namespace sj