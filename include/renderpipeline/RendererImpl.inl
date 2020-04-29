#pragma once

#include "renderpipeline/RendererImpl.inl"
#include "renderpipeline/UniformNames.h"
#include "renderpipeline/Utility.h"

#include <unirender/Device.h>
#include <unirender/Context.h>
#include <unirender/IndexBuffer.h>
#include <unirender/VertexBuffer.h>
#include <unirender/DrawState.h>
#include <unirender/VertexArray.h>

namespace rp
{

template<typename VT, typename IT>
RendererImpl<typename VT, typename IT>::RendererImpl(const ur::Device& dev)
{
    m_va = CreateVertexArray(dev);
}

template<typename VT, typename IT>
void RendererImpl<typename VT, typename IT>::
FlushBuffer(ur::Context& ctx, ur::PrimitiveType mode,
            const ur::RenderState& rs, const std::shared_ptr<ur::ShaderProgram>& shader)
{
	if (m_buf.indices.empty()) {
		return;
	}

    auto ibuf_sz = sizeof(IT) * m_buf.indices.size();
    auto ibuf = m_va->GetIndexBuffer();
    ibuf->Reset(ibuf_sz);
    ibuf->ReadFromMemory(m_buf.indices.data(), ibuf_sz, 0);

    auto vbuf_sz = sizeof(VT) * m_buf.vertices.size();
    auto vbuf = m_va->GetVertexBuffer();
    vbuf->Reset(vbuf_sz);
    vbuf->ReadFromMemory(m_buf.vertices.data(), vbuf_sz, 0);

    // todo
    //rc.BindTexture(m_tex_id, 0);
    //shader->SetMat4(MODEL_MAT_NAME, sm::mat4().x);

    ur::DrawState draw;
    draw.render_state = rs;
    draw.program = shader;
    draw.vertex_array = m_va;
    ctx.Draw(mode, draw, nullptr);

	m_buf.Clear();
}

}