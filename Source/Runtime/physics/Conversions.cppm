module;

#include <box3d/math_functions.h>

export module sj.engine.physics:Conversions;
import sj.std;

export namespace sj
{

vec4 b3Vec3ToVec4(const b3Vec3& v, float w = 0.0f)
{
    return vec4(v.x, v.y, v.z, w);
}

quat b3QuatToQuat(const b3Quat& q)
{
    return quat(q.v.x, q.v.y, q.v.z, q.s);
}

b3Vec3 vec4ToB3Vec3(const vec4& v)
{
    return b3Vec3 {
        .x = v.get_x(),
        .y = v.get_y(),
        .z = v.get_z(),
    };
}

mat44 b3TransformToMat44(const b3Transform& trans, const vec4& scale)
{
    vec4 t = b3Vec3ToVec4(trans.p);
    quat q = b3QuatToQuat(trans.q);

    mat44 out = mat44::from_sqt(sqt {.s=scale, .q=q, .t=t});
    out.set<3,3>(1.0f);
    return out;
}

} // namespace sj