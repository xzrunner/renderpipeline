#pragma once

#include "renderpipeline/RendererImpl.inl"

#include <unirender/Blackboard.h>
#include <unirender/RenderContext.h>

namespace rp
{

template<typename VT, typename IT>
RendererImpl<typename VT, typename IT>::RendererImpl()
{
    auto& rc = ur::Blackboard::Instance()->GetRenderContext();
    m_vbo = rc.CreateBuffer(ur::VERTEXBUFFER, nullptr, 0);
    m_ebo = rc.CreateBuffer(ur::INDEXBUFFER, nullptr, 0);
}

template<typename VT, typename IT>
RendererImpl<typename VT, typename IT>::~RendererImpl()
{
    auto& rc = ur::Blackboard::Instance()->GetRenderContext();
    rc.ReleaseBuffer(ur::VERTEXBUFFER, m_vbo);
    rc.ReleaseBuffer(ur::INDEXBUFFER, m_ebo);
}

template<typename VT, typename IT>
void RendererImpl<typename VT, typename IT>::FlushBuffer(ur::DRAW_MODE mode, const std::shared_ptr<pt0::Shader>& shader)
{
	if (m_buf.indices.empty()) {
		return;
	}

	shader->Use();
	if (m_buf.indices.empty()) {
		return;
	}

	auto& rc = ur::Blackboard::Instance()->GetRenderContext();
	rc.BindTexture(m_tex_id, 0);
	if (m_buf.indices.empty()) {
		return;
	}

	shader->SetMat4(MODEL_MAT_NAME, sm::mat4().x);
//    shader->UpdateModelMat(sm::mat4().x);

	rc.BindBuffer(ur::VERTEXBUFFER, m_vbo);
	size_t vbuf_sz = sizeof(VT) * m_buf.vertices.size();
	rc.UpdateBuffer(m_vbo, m_buf.vertices.data(), vbuf_sz);

	rc.BindBuffer(ur::INDEXBUFFER, m_ebo);
	size_t ibuf_sz = sizeof(IT) * m_buf.indices.size();
	rc.UpdateBuffer(m_ebo, m_buf.indices.data(), ibuf_sz);

	rc.DrawElements(mode, 0, m_buf.indices.size());

	m_buf.Clear();
}

}