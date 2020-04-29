#pragma once

#include "renderpipeline/IRenderer.h"

namespace ur { class Device; class Context; class Texture; }

namespace rp
{

class SkyboxRenderer : public IRenderer
{
public:
    SkyboxRenderer(const ur::Device& dev);

    virtual void Flush(ur::Context& ctx) override {}

    void Draw(const ur::Device& dev, ur::Context& ctx,
        const ur::Texture& cube_tex) const;

private:
    void InitShader(const ur::Device& dev);

}; // SkyboxRenderer

}