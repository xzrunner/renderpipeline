#pragma once

#include "renderpipeline/IRenderer.h"

#include <SM_Matrix.h>

#include <boost/noncopyable.hpp>

#include <memory>

namespace pt2 { class Shader; }

namespace rp
{

class ExternRenderer : public IRenderer, boost::noncopyable
{
public:
	ExternRenderer();
	~ExternRenderer();

	virtual void Flush() override;

	void DrawTexSpr(const std::shared_ptr<pt2::Shader>& shader, const sm::mat4& mat) const;
	void DrawNoTexSpr(const std::shared_ptr<pt2::Shader>& shader, const sm::mat4& mat) const;

private:
	void InitRenderData();

private:
	struct VertexBuf
	{
		uint32_t vao = 0;
		uint32_t vbo = 0;
		uint32_t ebo = 0;
	};

private:
	VertexBuf m_vb_tex, m_vb_no_tex;

}; // ExternRenderer

}