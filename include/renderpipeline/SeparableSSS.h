#pragma once

#include <rendergraph/Node.h>

#include <SM_Vector.h>

#include <vector>

namespace rp
{
namespace node
{

class SeparableSSS : public rendergraph::Node
{
public:
    SeparableSSS() {
        m_exports = {
            {{ rendergraph::VariableType::Vec4Array, "kernel" }}
        };
    }

    virtual void Eval(const rendergraph::RenderContext& rc, size_t port_idx,
        rendergraph::ShaderVariant& var, uint32_t& flags) const override;

    void SetProps(int nsamples, const sm::vec3& strength, const sm::vec3& falloff) {
        m_samples_num = nsamples;
        m_strength = strength;
        m_falloff = falloff;
    }

private:
    static void CalculateKernel(std::vector<sm::vec4>& kernel, int samples_num,
        const sm::vec3& strength, const sm::vec3& falloff);

    static sm::vec3 Profile(float r, const sm::vec3& falloff);
    static sm::vec3 Gaussian(float variance, float r, const sm::vec3& falloff);

private:
    int      m_samples_num = 7;
    sm::vec3 m_strength    = sm::vec3(0.48f, 0.41f, 0.28f);
    sm::vec3 m_falloff     = sm::vec3(1.0f, 0.37f, 0.3f);

    mutable std::vector<sm::vec4> m_kernel;

    RTTR_ENABLE(rendergraph::Node)

}; // SeparableSSS

}
}