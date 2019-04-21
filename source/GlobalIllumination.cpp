#include "renderpipeline/GlobalIllumination.h"
#include "renderpipeline/Utility.h"

#include "renderpipeline/HDREquirectangularToCubemap.h"

#include <unirender/Texture.h>
#include <unirender/TextureCube.h>
#include <painting3/GlobalIllumination.h>
#include <rendergraph/RenderContext.h>
#include <rendergraph/node/Texture.h>

namespace rp
{
namespace node
{

void GlobalIllumination::Execute(const rg::RenderContext& rc)
{
    if (m_gi.irradiance_map) {
        return;
    }

    if (m_imports[ID_SKYBOX].conns.empty()) {
        return;
    }

    auto node = m_imports[ID_SKYBOX].conns[0].node.lock();
    if (node->get_type() != rttr::type::get<rg::node::Texture>()) {
        return;
    }

    auto tex_node = std::static_pointer_cast<rg::node::Texture>(node);
    if (tex_node->GetType() != rg::node::Texture::Type::TexCube) {
        return;
    }

    auto img_id = tex_node->GetTexID();
    auto cube_id = rp::HDREquirectangularToCubemap(img_id);
//    cube_id = img_id;
    InitGIWithSkybox(rc.rc, cube_id, m_gi);
}

void GlobalIllumination::Eval(const rg::RenderContext& rc, size_t port_idx,
                              rg::ShaderVariant& var, uint32_t& flags) const
{
    switch (port_idx)
    {
    case ID_IRRADIANCE_MAP:
        if (m_gi.irradiance_map) {
            var.type = rg::VariableType::SamplerCube;
            var.id = m_gi.irradiance_map->GetTexID();
        }
        break;
    case ID_PREFILTER_MAP:
        if (m_gi.prefilter_map) {
            var.type = rg::VariableType::SamplerCube;
            var.id = m_gi.prefilter_map->GetTexID();
        }
        break;
    case ID_BRDF_LUT:
        if (m_gi.brdf_lut) {
            var.type = rg::VariableType::Sampler2D;
            var.id = m_gi.brdf_lut->TexID();
        }
        break;
    }
}

}
}