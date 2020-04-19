#pragma once

#include <unirender2/typedef.h>

namespace pt3 { struct GlobalIllumination; }
namespace ur2 { class Device; class Context; class VertexArray; }

namespace rp
{

void InitGIWithSkybox(const ur2::Device& dev, ur2::Context& ctx,
    const ur2::TexturePtr skybox, pt3::GlobalIllumination& gi);

std::shared_ptr<ur2::VertexArray>
CreateVertexArray(const ur2::Device& dev);

}