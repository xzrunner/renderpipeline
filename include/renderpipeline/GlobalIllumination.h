#pragma once

#include <painting3/GlobalIllumination.h>
#include <rendergraph/Node.h>

namespace ur { class RenderContext; }

namespace rp
{
namespace node
{

class GlobalIllumination : public rendergraph::Node
{
public:
    GlobalIllumination()
    {
        m_imports = {
            {{ rendergraph::VariableType::Port,    "prev"   }},
            {{ rendergraph::VariableType::Texture, "skybox" }}
        };
        m_exports = {
            {{ rendergraph::VariableType::Port,        "next"           }},
            {{ rendergraph::VariableType::SamplerCube, "irradiance_map" }},
            {{ rendergraph::VariableType::SamplerCube, "prefilter_map"  }},
            {{ rendergraph::VariableType::Sampler2D,   "brdf_lut"       }},
        };
    }

    virtual void Execute(const std::shared_ptr<dag::Context>& ctx = nullptr);
    virtual void Eval(const rendergraph::RenderContext& rc, size_t port_idx,
        rendergraph::ShaderVariant& var, uint32_t& flags) const override;

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

    RTTR_ENABLE(rendergraph::Node)

}; // GlobalIllumination

}
}