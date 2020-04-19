#pragma once

#include "renderpipeline/IRenderer.h"
#include "renderpipeline/RendererImpl.h"

#include <SM_Vector.h>
#include <unirender2/PrimitiveType.h>

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
    Shape3Renderer(const ur2::Device& dev);

    virtual void Flush(ur2::Context& ctx) override;

    void DrawLines(ur2::Context& ctx, const ur2::RenderState& rs, size_t num,
        const float* positions, uint32_t color);

private:
    void InitShader(const ur2::Device& dev);

private:
    ur2::PrimitiveType m_draw_mode = ur2::PrimitiveType::Lines;

    ur2::RenderState m_rs;

}; // Shape3Renderer

}