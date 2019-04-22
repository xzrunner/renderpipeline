#pragma once

#include <cstdint>

namespace ur { class RenderContext; }
namespace pt3 { struct GlobalIllumination; }

namespace rp
{

void InitGIWithSkybox(ur::RenderContext& rc,
    uint32_t skybox_id, pt3::GlobalIllumination& gi);

}