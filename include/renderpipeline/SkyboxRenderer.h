#pragma once

#include "renderpipeline/IRenderer.h"

namespace ur { class TextureCube; }

namespace rp
{

class SkyboxRenderer : public IRenderer
{
public:
    SkyboxRenderer();

    virtual void Flush() override;

    void Draw(const ur::TextureCube& tcube) const;

private:
    void InitShader();

}; // SkyboxRenderer

}