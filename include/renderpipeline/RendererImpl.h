#pragma once

#include "renderpipeline/RenderBuffer.h"

#include <unirender/typedef.h>

#include <memory>

namespace pt0 { class Shader; }

namespace rp
{

template<typename T>
class RendererImpl
{
public:
    RendererImpl();
    ~RendererImpl();

protected:
    void FlushBuffer(ur::DRAW_MODE mode,
        const std::shared_ptr<pt0::Shader>& shader);

protected:
    RenderBuffer<T> m_buf;

    uint32_t m_vbo = 0, m_ebo = 0;

    uint32_t m_tex_id = 0;

}; // RendererImpl

}

#include "renderpipeline/RendererImpl.inl"
