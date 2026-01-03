#pragma once

#include <gfx/seadColor.h>
#include <gfx/seadGraphics.h>

namespace sead
{
class DrawContext;

class GraphicsContext
{
public:
    GraphicsContext();
    virtual ~GraphicsContext();

    void apply(DrawContext* drawContext) const;
    void applyAlphaTest(DrawContext* drawContext) const;
    void applyDepthAndStencilTest(DrawContext* drawContext) const;
    void applyColorMask(DrawContext* drawContext) const;
    void applyBlendAndFastZ(DrawContext* drawContext) const;
    void applyBlendConstantColor(DrawContext* drawContext) const;

    void setDepthTestEnabled(bool isEnabled) { mIsDepthTestEnabled = isEnabled; }
    void setDepthWriteEnabled(bool isEnabled) { mIsDepthWriteEnabled = isEnabled; }
    void setCullFace(Graphics::CullingMode face) { mCullFace = face; }

    void setPoop()
    {
        mIsDepthTestEnabled = false;
        mIsDepthWriteEnabled = false;
        mCullFace = sead::Graphics::CullingMode::Front;
    }

private:
    bool mIsDepthTestEnabled;
    bool mIsDepthWriteEnabled;
    Graphics::DepthFunc mDepthFunc;
    Graphics::CullingMode mCullFace;
    bool mIsBlendEnabled;
    Graphics::BlendFactor mBlendFactorSrcRGB;
    Graphics::BlendFactor mBlendFactorSrcA;
    Graphics::BlendFactor mBlendFactorDstRGB;
    Graphics::BlendFactor mBlendFactorDstA;
    Graphics::BlendEquation mBlendEquationRGB;
    Graphics::BlendEquation mBlendEquationA;
    Color4f mBlendConstantColor;
    bool mAlphaTestEnable;
    Graphics::AlphaFunc mAlphaTestFunc;
    f32 mAlphaTestRef;
    bool mColorMaskR;
    bool mColorMaskG;
    bool mColorMaskB;
    bool mColorMaskA;
    bool mStencilTestEnable;
    Graphics::StencilFunc mStencilTestFunc;
    s32 mStencilTestRef;
    u32 mStencilTestMask;
    Graphics::StencilOp mStencilOpFail;
    Graphics::StencilOp mStencilOpZFail;
    Graphics::StencilOp mStencilOpZPass;
    Graphics::PolygonMode mPolygonModeFront;
    Graphics::PolygonMode mPolygonModeBack;
    bool mPolygonOffsetFrontEnable;
    bool mPolygonOffsetBackEnable;
    bool mPolygonOffsetPointLineEnable;
};

}  // namespace sead
