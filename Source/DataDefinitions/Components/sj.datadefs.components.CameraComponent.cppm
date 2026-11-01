module;

export module sj.datadefs.components.CameraComponent;
import sj.std.math;

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