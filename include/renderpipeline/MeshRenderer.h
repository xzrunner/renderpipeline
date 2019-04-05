#pragma once

#include "renderpipeline/IRenderer.h"
#include "renderpipeline/RendererImpl.h"

#include <SM_Vector.h>
#include <painting0/Material.h>
#include <painting0/RenderContext.h>

#include <boost/noncopyable.hpp>

namespace model { struct MeshGeometry; }

namespace rp
{

struct MeshVertex
{
    sm::vec3 position;
    sm::vec3 normal;
    sm::vec2 texcoord;
};

class MeshRenderer : public IRenderer, private RendererImpl<MeshVertex>, private boost::noncopyable
{
public:
    MeshRenderer();

    virtual void Flush() override;

    void Draw(const model::MeshGeometry& mesh, const pt0::Material& material,
        const pt0::RenderContext& ctx, const std::shared_ptr<pt0::Shader>& shader = nullptr) const;

private:
    void InitShader();

}; // MeshRenderer

}