#pragma once

#include "renderpipeline/IRenderer.h"
#include "renderpipeline/RendererImpl.h"

#include <SM_Vector.h>

#include <boost/noncopyable.hpp>

namespace pt3 { class WindowContext; }

namespace rp
{

struct Shape3Vertex
{
    sm::vec3 pos;
    uint32_t col = 0;
};

class Shape3Renderer : public IRenderer, private RendererImpl<Shape3Vertex, unsigned short>, private boost::noncopyable
{
public:
    Shape3Renderer();

    virtual void Flush() override;

    void DrawLines(size_t num, const float* positions, uint32_t color);

private:
    void InitShader();

private:
    ur::DRAW_MODE m_draw_mode = ur::DRAW_LINES;

}; // Shape3Renderer

}