#include "renderpipeline/GlobalIllumination.h"
#include "renderpipeline/Utility.h"

#include <unirender/Texture.h>
#include <painting3/GlobalIllumination.h>
#include <rendergraph/RenderContext.h>
#include <rendergraph/node/Texture.h>

namespace rp
{
namespace node
{

void GlobalIllumination::Execute(const std::shared_ptr<dag::Context>& ctx)
{
    if (m_gi.irradiance_map) {
        return;
    }

    if (m_imports[ID_SKYBOX].conns.empty()) {
        return;
    }

    auto node = m_imports[ID_SKYBOX].conns[0].node.lock();
    if (node->get_type() != rttr::type::get<rendergraph::node::Texture>()) {
        return;
    }

    auto tex_node = std::static_pointer_cast<rendergraph::node::Texture>(node);
    if (tex_node->GetType() != rendergraph::node::Texture::Type::TexCube) {
        return;
    }

    auto rc = std::static_pointer_cast<rendergraph::RenderContext>(ctx);
    InitGIWithSkybox(*rc->ur_dev, *rc->ur_ctx, tex_node->GetTexture(), m_gi);
}

void GlobalIllumination::Eval(const rendergraph::RenderContext& rc, size_t port_idx,
                              rendergraph::ShaderVariant& var, uint32_t& flags) const
{
    switch (port_idx)
    {
    case ID_IRRADIANCE_MAP:
        if (m_gi.irradiance_map) {
            var.type = rendergraph::VariableType::SamplerCube;
            var.p = &m_gi.irradiance_map;
        }
        break;
    case ID_PREFILTER_MAP:
        if (m_gi.prefilter_map) {
            var.type = rendergraph::VariableType::SamplerCube;
            var.p = &m_gi.prefilter_map;
        }
        break;
    case ID_BRDF_LUT:
        if (m_gi.brdf_lut) {
            var.type = rendergraph::VariableType::Sampler2D;
            var.p = &m_gi.brdf_lut;
        }
        break;
    }
}

}
}