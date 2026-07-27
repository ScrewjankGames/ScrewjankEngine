module;
#include <ScrewjankStd/Assert.hpp>

export module sj.engine.CameraComponent;
import sj.std.math;
import sj.engine.TransformComponent;

export namespace sj
{
struct CameraComponent
{
    mat44 localToGoTransform;
    float fov = 0;
    float nearPlane = 0;
    float farPlane = 0;
};
} // namespace sj