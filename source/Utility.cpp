#include "renderpipeline/Utility.h"
#include "renderpipeline/CreateIrradianceCubemap.h"
#include "renderpipeline/CreatePrefilterCubemap.h"
#include "renderpipeline/CreateBrdfLutTex.h"

#include <unirender/Texture.h>
#include <unirender/TextureCube.h>
#include <painting3/GlobalIllumination.h>

namespace rp
{

void InitGIWithSkybox(ur::RenderContext& rc, uint32_t skybox_id, pt3::GlobalIllumination& gi)
{
    if (!gi.irradiance_map) {
        gi.irradiance_map = std::make_shared<ur::TextureCube>(&rc);
    }
    auto tex_id = rp::CreateIrradianceCubemap(skybox_id);
    gi.irradiance_map->SetTexID(tex_id);

    if (!gi.prefilter_map) {
        gi.prefilter_map = std::make_shared<ur::TextureCube>(&rc);
    }
    tex_id = rp::CreatePrefilterCubemap(skybox_id);
    gi.prefilter_map->SetTexID(tex_id);

    tex_id = rp::CreateBrdfLutTex();
    gi.brdf_lut = std::make_shared<ur::Texture>(&rc, rp::BRDF_LUT_TEX_SIZE,
        rp::BRDF_LUT_TEX_SIZE, rp::BRDF_LUT_TEX_FMT, tex_id);
}

}