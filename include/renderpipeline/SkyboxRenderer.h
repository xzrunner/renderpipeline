#pragma once

#include "renderpipeline/IRenderer.h"

namespace ur2 { class Device; class Context; class Texture; }

namespace rp
{

class SkyboxRenderer : public IRenderer
{
public:
    SkyboxRenderer(const ur2::Device& dev);

    virtual void Flush(ur2::Context& ctx) override {}

    void Draw(ur2::Context& ctx, const ur2::Texture& cube_tex) const;

private:
    void InitShader(const ur2::Device& dev);

}; // SkyboxRenderer

}