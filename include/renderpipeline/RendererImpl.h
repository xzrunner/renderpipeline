#pragma once

#include "renderpipeline/RenderBuffer.h"

#include <unirender/PrimitiveType.h>

#include <memory>

namespace ur
{
    class Device;
    class Context;
    class ShaderProgram;
    class VertexArray;
    struct RenderState;
    class DescriptorSet;
    class PipelineLayout;
    class Pipeline;
}

namespace rp
{

template<typename VT, typename IT>
class RendererImpl
{
public:
    RendererImpl(const ur::Device& dev);

protected:
    void FlushBuffer(ur::Context& ctx,  ur::PrimitiveType mode, const ur::RenderState& rs, 
        const std::shared_ptr<ur::ShaderProgram>& shader, const std::shared_ptr<ur::DescriptorSet>& desc_set = nullptr,
        const std::shared_ptr<ur::PipelineLayout>& pipeline_layout = nullptr, const std::shared_ptr<ur::Pipeline>& pipeline = nullptr);

protected:
    RenderBuffer<VT, IT> m_buf;

    std::shared_ptr<ur::VertexArray> m_va = nullptr;

 //   uint32_t m_tex_id = 0;

}; // RendererImpl

}

#include "renderpipeline/RendererImpl.inl"
