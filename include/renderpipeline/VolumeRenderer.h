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

class VolumeRenderer : public IRenderer, private RendererImpl<VolumeVertex>, private boost::noncopyable
{
public:
	VolumeRenderer();

	virtual void Flush() override;

	void DrawCube(const float* positions, const float* texcoords, int texid, uint32_t color);

private:
	void InitShader();

	void PrepareRenderState();
	void RestoreRenderState();

}; // VolumeRenderer

}