#pragma once
#include "libretro.h"
#include <cmath>
#include <initializer_list>

namespace Libretro
{
// Aim (-Z) is supplied by the frontend; grip has a different orientation.
inline bool ControllerAim(const retro_vr_controller_poses &controllers, retro_vr_pose &aim)
{
    aim = {};
    const auto &state = controllers.hands[RETRO_VR_HAND_RIGHT];
    if (!(state.flags & RETRO_VR_HAND_ACTIVE) ||
        !(state.aim.flags & RETRO_VR_POSE_ORIENTATION_VALID))
    {
        return false;
    }

    double norm = 0;
    for (float v : state.aim.orientation)
    {
        norm += double(v)*v;
    }

    if (!std::isfinite(norm) || norm < 0.5 || norm > 1.5)
    {
        return false;
    }

    aim = state.aim;
    for (float &v : aim.orientation)
    {
        v /= std::sqrt(norm);
    }

    for (float v : aim.position)
    {
        if (!std::isfinite(v))
        {
            aim.flags &= ~RETRO_VR_POSE_POSITION_VALID;
        }
    }

    return true;
}

inline void MultiplyOrientation(const float *a, const float *b, float *out)
{
    const float result[4]{
        a[3]*b[0] + a[0]*b[3] + a[1]*b[2] - a[2]*b[1],
        a[3]*b[1] - a[0]*b[2] + a[1]*b[3] + a[2]*b[0],
        a[3]*b[2] + a[0]*b[1] - a[1]*b[0] + a[2]*b[3],
        a[3]*b[3] - a[0]*b[0] - a[1]*b[1] - a[2]*b[2]
    };
    for (unsigned i = 0; i < 4; ++i)
    {
        out[i] = result[i];
    }
}

inline void AimCalibration(const float *controller, const float *head, float *correction)
{
    const float inverse[4]{-controller[0], -controller[1], -controller[2], controller[3]};

    MultiplyOrientation(inverse, head, correction);
}

// Convert a tracking-space vector to engine world axes. Doom stretches the
// vertical axis when rendering, so undo that stretch for world-space traces.
inline void TrackingVector(const float *v, double bodyYaw, double pixelStretch, double *world)
{
    world[0] = -v[2]*std::cos(bodyYaw) + v[0]*std::sin(bodyYaw);
    world[1] = -v[2]*std::sin(bodyYaw) - v[0]*std::cos(bodyYaw);
    world[2] = v[1]/pixelStretch;
}

inline void AimForward(const float *q, float *forward)
{
    forward[0] = -2*(q[0]*q[2] + q[3]*q[1]);
    forward[1] = 2*(q[3]*q[0] - q[1]*q[2]);
    forward[2] = 2*(q[0]*q[0] + q[1]*q[1]) - 1;
}

inline void AimOrientation(float *q, double yaw, double pitch)
{
    q[0] = std::cos(yaw/2)*std::sin(pitch/2);
    q[1] = std::sin(yaw/2)*std::cos(pitch/2);
    q[2] = -std::sin(yaw/2)*std::sin(pitch/2);
    q[3] = std::cos(yaw/2)*std::cos(pitch/2);
}
}
