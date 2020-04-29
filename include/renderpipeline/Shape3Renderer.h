#pragma once

#include "renderpipeline/IRenderer.h"
#include "renderpipeline/RendererImpl.h"

#include <SM_Vector.h>
#include <unirender/PrimitiveType.h>

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
    Shape3Renderer(const ur::Device& dev);

    virtual void Flush(ur::Context& ctx) override;

    void DrawLines(ur::Context& ctx, const ur::RenderState& rs, size_t num,
        const float* positions, uint32_t color);

private:
    void InitShader(const ur::Device& dev);

private:
    ur::PrimitiveType m_draw_mode = ur::PrimitiveType::Lines;

    ur::RenderState m_rs;

}; // Shape3Renderer

}