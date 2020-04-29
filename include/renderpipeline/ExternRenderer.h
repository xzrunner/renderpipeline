#pragma once

#include "renderpipeline/IRenderer.h"

#include <SM_Matrix.h>

#include <boost/noncopyable.hpp>

#include <memory>

namespace ur { class Device; class Context; class VertexArray; }

namespace rp
{

class ExternRenderer : public IRenderer, boost::noncopyable
{
public:
	ExternRenderer(const ur::Device& dev);

	virtual void Flush(ur::Context& ctx) override {}

	void DrawTexSpr(ur::Context& ctx,
        const std::shared_ptr<ur::ShaderProgram>& shader, const sm::mat4& mat) const;
	void DrawNoTexSpr(ur::Context& ctx,
        const std::shared_ptr<ur::ShaderProgram>& shader, const sm::mat4& mat) const;

private:
	void InitRenderData(const ur::Device& dev);

private:
    std::shared_ptr<ur::VertexArray> m_va_tex = nullptr;
    std::shared_ptr<ur::VertexArray> m_va_no_tex = nullptr;

}; // ExternRenderer

}