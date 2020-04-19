#pragma once

#include "renderpipeline/RenderBuffer.h"

#include <unirender2/PrimitiveType.h>

#include <memory>

namespace ur2
{
    class Device;
    class Context;
    class ShaderProgram;
    class VertexArray;
    struct RenderState;
}

namespace rp
{

template<typename VT, typename IT>
class RendererImpl
{
public:
    RendererImpl(const ur2::Device& dev);

protected:
    void FlushBuffer(ur2::Context& ctx,  ur2::PrimitiveType mode,
        const ur2::RenderState& rs, const std::shared_ptr<ur2::ShaderProgram>& shader);

protected:
    RenderBuffer<VT, IT> m_buf;

    std::shared_ptr<ur2::VertexArray> m_va = nullptr;

 //   uint32_t m_tex_id = 0;

}; // RendererImpl

}

#include "renderpipeline/RendererImpl.inl"
