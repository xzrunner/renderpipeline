#pragma once

#include <unirender/typedef.h>

namespace pt3 { struct GlobalIllumination; }
namespace ur { class Device; class Context; class VertexArray; }

namespace rp
{

void InitGIWithSkybox(const ur::Device& dev, ur::Context& ctx,
    const ur::TexturePtr skybox, pt3::GlobalIllumination& gi);

std::shared_ptr<ur::VertexArray>
CreateVertexArray(const ur::Device& dev);

}