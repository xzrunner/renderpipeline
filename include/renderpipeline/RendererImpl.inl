#pragma once

#include "renderpipeline/RendererImpl.inl"
#include "renderpipeline/UniformNames.h"
#include "renderpipeline/Utility.h"

#include <unirender2/Device.h>
#include <unirender2/Context.h>
#include <unirender2/IndexBuffer.h>
#include <unirender2/VertexBuffer.h>
#include <unirender2/DrawState.h>
#include <unirender2/VertexArray.h>

namespace rp
{

template<typename VT, typename IT>
RendererImpl<typename VT, typename IT>::RendererImpl(const ur2::Device& dev)
{
    m_va = CreateVertexArray(dev);
}

template<typename VT, typename IT>
void RendererImpl<typename VT, typename IT>::
FlushBuffer(ur2::Context& ctx, ur2::PrimitiveType mode,
            const ur2::RenderState& rs, const std::shared_ptr<ur2::ShaderProgram>& shader)
{
	if (m_buf.indices.empty()) {
		return;
	}

    auto ibuf_sz = sizeof(IT) * m_buf.indices.size();
    auto ibuf = m_va->GetIndexBuffer();
    ibuf->Reserve(ibuf_sz);
    ibuf->ReadFromMemory(m_buf.indices.data(), ibuf_sz, 0);

    auto vbuf_sz = sizeof(VT) * m_buf.vertices.size();
    auto vbuf = m_va->GetVertexBuffer();
    vbuf->Reserve(vbuf_sz);
    vbuf->ReadFromMemory(m_buf.vertices.data(), vbuf_sz, 0);

    // todo
    //rc.BindTexture(m_tex_id, 0);
    //shader->SetMat4(MODEL_MAT_NAME, sm::mat4().x);

    ur2::DrawState draw;
    draw.render_state = rs;
    draw.program = shader;
    draw.vertex_array = m_va;
    ctx.Draw(mode, draw, nullptr);

	m_buf.Clear();
}

}