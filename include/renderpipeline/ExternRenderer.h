#pragma once

#include "renderpipeline/IRenderer.h"

#include <SM_Matrix.h>

#include <boost/noncopyable.hpp>

#include <memory>

namespace ur2 { class Device; class Context; class VertexArray; }

namespace rp
{

class ExternRenderer : public IRenderer, boost::noncopyable
{
public:
	ExternRenderer(const ur2::Device& dev);

	virtual void Flush(ur2::Context& ctx) override {}

	void DrawTexSpr(ur2::Context& ctx,
        const std::shared_ptr<ur2::ShaderProgram>& shader, const sm::mat4& mat) const;
	void DrawNoTexSpr(ur2::Context& ctx,
        const std::shared_ptr<ur2::ShaderProgram>& shader, const sm::mat4& mat) const;

private:
	void InitRenderData(const ur2::Device& dev);

private:
    std::shared_ptr<ur2::VertexArray> m_va_tex = nullptr;
    std::shared_ptr<ur2::VertexArray> m_va_no_tex = nullptr;

}; // ExternRenderer

}