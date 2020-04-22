#pragma once

#include "renderpipeline/IRenderer.h"
#include "renderpipeline/RendererImpl.h"

#include <SM_Vector.h>
#include <SM_Matrix.h>
#include <unirender2/typedef.h>

#include <boost/noncopyable.hpp>

namespace tess { class Painter; class Palette; }

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
	SpriteRenderer(const ur2::Device& dev);

	virtual void Flush(ur2::Context& ctx) override;

	void DrawQuad(ur2::Context& ctx, const ur2::RenderState& rs, const float* positions,
        const float* texcoords, int tex_id, uint32_t color);
	void DrawPainter(ur2::Context& ctx, const ur2::RenderState& rs, const tess::Painter& pt,
        const sm::mat4& mat = sm::mat4());

	auto& GetPalette() const { return *m_palette; }

private:
	void InitShader(const ur2::Device& dev);

private:
	std::unique_ptr<tess::Palette> m_palette = nullptr;

//    ur2::TexturePtr m_tex = nullptr;
    int m_tex_id = 0;

    ur2::RenderState m_rs;
    std::shared_ptr<ur2::Framebuffer> m_fbo = nullptr;

}; // SpriteRenderer

}