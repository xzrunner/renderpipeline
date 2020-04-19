#include "renderpipeline/Utility.h"
#include "renderpipeline/CreateIrradianceCubemap.h"
#include "renderpipeline/CreatePrefilterCubemap.h"
#include "renderpipeline/CreateBrdfLutTex.h"

#include <unirender2/Device.h>
#include <unirender2/VertexArray.h>
#include <painting3/GlobalIllumination.h>

namespace rp
{

void InitGIWithSkybox(const ur2::Device& dev, ur2::Context& ctx,
                      const ur2::TexturePtr skybox, pt3::GlobalIllumination& gi)
{
    if (!gi.irradiance_map) {
        gi.irradiance_map = CreateIrradianceCubemap(dev, ctx, skybox);
    }
    if (!gi.prefilter_map) {
        gi.prefilter_map = CreatePrefilterCubemap(dev, ctx, skybox);
    }
    if (!gi.brdf_lut) {
        gi.brdf_lut = CreateBrdfLutTex(dev, ctx);
    }
}

std::shared_ptr<ur2::VertexArray>
CreateVertexArray(const ur2::Device& dev)
{
    auto va = dev.CreateVertexArray();

    auto usage = ur2::BufferUsageHint::StaticDraw;

    auto ibuf = dev.CreateIndexBuffer(usage, 0);
    va->SetIndexBuffer(ibuf);

    auto vbuf = dev.CreateVertexBuffer(ur2::BufferUsageHint::StaticDraw, 0);
    va->SetVertexBuffer(vbuf);

    return va;
}

}