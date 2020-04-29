#pragma once

#include "renderpipeline/IRenderer.h"
#include "renderpipeline/RendererImpl.h"

#include <SM_Vector.h>

#include <boost/noncopyable.hpp>

namespace rp
{

struct VolumeVertex
{
    sm::vec3 pos;
    sm::vec3 uv;
    uint32_t col = 0;
};

class VolumeRenderer : public IRenderer, private RendererImpl<VolumeVertex, unsigned short>, private boost::noncopyable
{
public:
	VolumeRenderer(const ur::Device& dev);

	virtual void Flush(ur::Context& ctx) override;

	void DrawCube(ur::Context& ctx, const ur::RenderState& rs,
        const float* positions, const float* texcoords, int texid, uint32_t color);

private:
	void InitShader(const ur::Device& dev);

	static void PrepareRenderState(ur::RenderState& rs);

private:
    int m_tex_id = 0;

    ur::RenderState m_rs;

}; // VolumeRenderer

}