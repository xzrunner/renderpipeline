#pragma once

#include "renderpipeline/IRenderer.h"
#include "renderpipeline/RendererImpl.h"

#include <SM_Vector.h>
#include <SM_Matrix.h>
#include <unirender/typedef.h>

#include <boost/noncopyable.hpp>

namespace tess { class Painter; class Palette; }
namespace ur { class DescriptorSet; }

namespace rp
{

struct SpriteVertex
{
	sm::vec2 pos;
	sm::vec2 uv;
	uint32_t col = 0;
};

class SpriteRenderer : public IRenderer, private RendererImpl<SpriteVertex, unsigned short>, private boost::noncopyable
{
public:
	SpriteRenderer(const ur::Device& dev, const ur::Context& ctx);

	virtual void Flush(ur::Context& ctx) override;

	void DrawQuad(ur::Context& ctx, const ur::RenderState& rs, const float* positions,
        const float* texcoords, const ur::TexturePtr& tex, uint32_t color);
	void DrawPainter(ur::Context& ctx, const ur::RenderState& rs, const tess::Painter& pt,
        const sm::mat4& mat = sm::mat4());

	auto& GetPalette() const { return *m_palette; }

private:
	void InitShader(const ur::Device& dev);

	void InitRenderer(const ur::Device& dev, const ur::Context& ctx);

private:
	std::unique_ptr<tess::Palette> m_palette = nullptr;

    ur::TexturePtr m_tex = nullptr;

    ur::RenderState m_rs;
    std::shared_ptr<ur::Framebuffer> m_fbo = nullptr;

	std::shared_ptr<ur::UniformBuffer> m_uniform_buf = nullptr;
	std::shared_ptr<ur::DescriptorSet> m_desc_set = nullptr;

	std::shared_ptr<ur::PipelineLayout> m_pipeline_layout = nullptr;
	std::shared_ptr<ur::Pipeline>       m_pipeline        = nullptr;

}; // SpriteRenderer

}