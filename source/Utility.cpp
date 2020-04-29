#include "renderpipeline/Utility.h"
#include "renderpipeline/CreateIrradianceCubemap.h"
#include "renderpipeline/CreatePrefilterCubemap.h"
#include "renderpipeline/CreateBrdfLutTex.h"

#include <unirender/Device.h>
#include <unirender/VertexArray.h>
#include <painting3/GlobalIllumination.h>

namespace rp
{

void InitGIWithSkybox(const ur::Device& dev, ur::Context& ctx,
                      const ur::TexturePtr skybox, pt3::GlobalIllumination& gi)
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

std::shared_ptr<ur::VertexArray>
CreateVertexArray(const ur::Device& dev)
{
    auto va = dev.CreateVertexArray();

    auto usage = ur::BufferUsageHint::StaticDraw;

    auto ibuf = dev.CreateIndexBuffer(usage, 0);
    va->SetIndexBuffer(ibuf);

    auto vbuf = dev.CreateVertexBuffer(ur::BufferUsageHint::StaticDraw, 0);
    va->SetVertexBuffer(vbuf);

    return va;
}

}