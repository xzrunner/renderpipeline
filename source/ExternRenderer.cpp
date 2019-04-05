#include "renderpipeline/ExternRenderer.h"

#include <unirender/RenderContext.h>
#include <unirender/Blackboard.h>
#include <painting2/Shader.h>

namespace rp
{

ExternRenderer::ExternRenderer()
{
	InitRenderData();
}

ExternRenderer::~ExternRenderer()
{
	ur::Blackboard::Instance()->GetRenderContext().ReleaseVAO(
		m_vb_no_tex.vao, m_vb_no_tex.vbo, m_vb_no_tex.ebo);
	ur::Blackboard::Instance()->GetRenderContext().ReleaseVAO(
		m_vb_tex.vao, m_vb_tex.vbo, m_vb_tex.ebo);
}

void ExternRenderer::Flush()
{
}

void ExternRenderer::DrawTexSpr(const std::shared_ptr<pt2::Shader>& shader, const sm::mat4& mat) const
{
    shader->UpdateModelMat(mat);

	auto& rc = ur::Blackboard::Instance()->GetRenderContext();
	rc.DrawElementsVAO(ur::DRAW_TRIANGLES, 0, 6, m_vb_tex.vao);
}

void ExternRenderer::DrawNoTexSpr(const std::shared_ptr<pt2::Shader>& shader, const sm::mat4& mat) const
{
    shader->UpdateModelMat(mat);

	auto& rc = ur::Blackboard::Instance()->GetRenderContext();
	rc.DrawElementsVAO(ur::DRAW_TRIANGLES, 0, 6, m_vb_no_tex.vao);
}

void ExternRenderer::InitRenderData()
{
	// tex
	{
		ur::RenderContext::VertexInfo vi;

		float vertices[] = {
			// pos          // tex
			-0.5f, -0.5f,   0.0f, 0.0f,
			 0.5f, -0.5f,   1.0f, 0.0f,
			 0.5f,  0.5f,   1.0f, 1.0f,
			-0.5f,  0.5f,   0.0f, 1.0f,
		};
		vi.vn = 4;
		vi.vertices = vertices;
		vi.stride = 4 * sizeof(float);

		unsigned short indices[] = { 0, 1, 2, 0, 2, 3 };
		vi.in = 6;
		vi.indices = indices;

		vi.va_list.push_back(ur::VertexAttrib("pos", 2, 4, 16, 0));
		vi.va_list.push_back(ur::VertexAttrib("tex", 2, 4, 16, 8));

		ur::Blackboard::Instance()->GetRenderContext().CreateVAO(
			vi, m_vb_tex.vao, m_vb_tex.vbo, m_vb_tex.ebo);
	}
	// no tex
	{
		ur::RenderContext::VertexInfo vi;

		float vertices[] = {
			-0.5f, -0.5f,
			 0.5f, -0.5f,
			 0.5f,  0.5f,
			-0.5f,  0.5f,
		};
		vi.vn = 4;
		vi.vertices = vertices;
		vi.stride = 4 * sizeof(float);

		unsigned short indices[] = { 0, 1, 2, 0, 2, 3 };
		vi.in = 6;
		vi.indices = indices;

		vi.va_list.push_back(ur::VertexAttrib("pos", 2, 4, 8, 0));

		ur::Blackboard::Instance()->GetRenderContext().CreateVAO(
			vi, m_vb_no_tex.vao, m_vb_no_tex.vbo, m_vb_no_tex.ebo);
	}
}

}