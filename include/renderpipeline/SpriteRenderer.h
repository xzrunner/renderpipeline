#pragma once

#include "renderpipeline/IRenderer.h"
#include "renderpipeline/RendererImpl.h"

#include <SM_Vector.h>
#include <SM_Matrix.h>

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

class SpriteRenderer : public IRenderer, private RendererImpl<SpriteVertex>, private boost::noncopyable
{
public:
	SpriteRenderer();

	virtual void Flush() override;

	void DrawQuad(const float* positions, const float* texcoords, int texid, uint32_t color);
	void DrawPainter(const tess::Painter& pt, const sm::mat4& mat = sm::mat4());

	auto& GetPalette() const { return *m_palette; }

private:
	void InitShader();

private:
	std::unique_ptr<tess::Palette> m_palette = nullptr;

}; // SpriteRenderer

}