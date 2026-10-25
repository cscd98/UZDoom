#pragma once
#include "libretro.h"
#include "matrix.h"

namespace Libretro
{
inline void EyeProjection(VSMatrix &matrix, const retro_vr_eye_state &eye, float nearPlane, float farPlane)
{
    // frustum() multiplies in this engine, unlike perspective()/ortho().
    matrix.loadIdentity();
    matrix.frustum(-nearPlane*eye.fov_tan[0], nearPlane*eye.fov_tan[1],
        -nearPlane*eye.fov_tan[3], nearPlane*eye.fov_tan[2], nearPlane, farPlane);
}

inline void HeadPlaneProjection(VSMatrix &matrix, const retro_vr_eye_state &eye,
    const float *headPosition, const float *headOrientation, float width, float height,
    float distance, float verticalOffset = 0)
{
    EyeProjection(matrix, eye, 0.01f, 100.0f);

    // VSMatrix::translate is affine-only; compose the model transform before
    // multiplying by the perspective projection so homogeneous W is preserved.
    VSMatrix plane(1);
    plane.multQuaternion(FVector4(-eye.orientation[0], -eye.orientation[1],
        -eye.orientation[2], eye.orientation[3]));
    plane.translate(headPosition[0]-eye.position[0], headPosition[1]-eye.position[1],
        headPosition[2]-eye.position[2]);
    plane.multQuaternion(FVector4(headOrientation[0], headOrientation[1],
        headOrientation[2], headOrientation[3]));

    // The same physical plane in both eyes, rather than identical NDC positions
    // which diverge when the headset's left/right FOVs are asymmetric.
    const float planeHeight = distance * 1.0f;
    const float planeWidth = planeHeight * width/height;
    plane.translate(-planeWidth/2, planeHeight/2 - verticalOffset*planeHeight, -distance);
    plane.scale(planeWidth/width, -planeHeight/height, 0);

    matrix.multMatrix(plane);
}
}
