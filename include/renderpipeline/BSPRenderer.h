#pragma once

#include "renderpipeline/IRenderer.h"
#include "renderpipeline/RendererImpl.h"

#include <SM_Vector.h>

#include <boost/noncopyable.hpp>

namespace ur2 { class Device; }

namespace rp
{

struct BSPVertex
{
    sm::vec3 position;
    sm::vec2 texcoord;
    sm::vec2 texcoord_light;
};

class BSPRenderer : public IRenderer, private RendererImpl<BSPVertex, unsigned short>, private boost::noncopyable
{
public:
    BSPRenderer(const ur2::Device& dev);

    virtual void Flush(ur2::Context& ctx) override {}

    void Draw() const;

private:
    void InitShader(const ur2::Device& dev);

}; // BSPRenderer

}