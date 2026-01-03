#pragma once

#include <gfx/seadDrawLockContext.h>
#include <heap/seadDisposer.h>

namespace sead
{
// no content yet, just for the enum
class Graphics : public IDisposer
{
    using UnknownCallback = void (*)(int);
    static Graphics* sInstance;

public:
    enum class AlphaFunc{
    };

    enum class BlendEquation{
    };

    enum class BlendFactor{
    };

    enum class CullingMode{
        Front,
        Back,
    };

    enum class DepthFunc{
        Invalid = -1,
        Never = 0x0200,
        Less = 0x0201,
        Equal = 0x0202,
        LessEqual = 0x0203,
        Greater = 0x0204,
        NotEqual = 0x0205,
        GreaterEqual = 0x0206,
        Always = 0x0207,
    };

    enum DevicePosture
    {
        cDevicePosture_Same = 0,
        cDevicePosture_RotateRight = 1,
        cDevicePosture_RotateLeft = 2,
        cDevicePosture_RotateHalfAround = 3,
        cDevicePosture_FlipX = 4,
        cDevicePosture_FlipY = 5,
        cDevicePosture_FlipXY = 3,
        cDevicePosture_Invalid = 4,
    };

    enum class PolygonMode{
    };

    enum class StencilFunc{
    };

    enum class StencilOp{
    };

    void lockDrawContext();
    void unlockDrawContext();
    void initHostIO();
    void initializeDrawLockContext(Heap*);

    static Graphics* instance() { return sInstance; }

private:
    UnknownCallback _20;
    DrawLockContext* mDrawLockContext;
};

}  // namespace sead
