#pragma once

#include <painting3/GlobalIllumination.h>
#include <rendergraph/Node.h>

namespace ur { class RenderContext; }

namespace rp
{
namespace node
{

class GlobalIllumination : public rg::Node
{
public:
    GlobalIllumination()
    {
        m_imports = {
            {{ rg::VariableType::Port,    "prev"   }},
            {{ rg::VariableType::Texture, "skybox" }}
        };
        m_exports = {
            {{ rg::VariableType::Port,        "next"           }},
            {{ rg::VariableType::SamplerCube, "irradiance_map" }},
            {{ rg::VariableType::SamplerCube, "prefilter_map"  }},
            {{ rg::VariableType::Sampler2D,   "brdf_lut"       }},
        };
    }

    virtual void Execute(const rg::RenderContext& rc) override;
    virtual void Eval(const rg::RenderContext& rc, size_t port_idx,
        rg::ShaderVariant& var, uint32_t& flags) const override;

private:
    enum InputID
    {
        ID_SKYBOX = 1,
    };

    enum OutputID
    {
        ID_IRRADIANCE_MAP = 1,
        ID_PREFILTER_MAP,
        ID_BRDF_LUT
    };

private:
    pt3::GlobalIllumination m_gi;

    RTTR_ENABLE(rg::Node)

}; // GlobalIllumination

}
}