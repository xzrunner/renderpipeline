#pragma once

#include "renderpipeline/IRenderer.h"
#include "renderpipeline/RendererImpl.h"

#include <SM_Vector.h>
#include <painting0/Material.h>
#include <painting0/RenderContext.h>
#include <model/Model.h>

#include <boost/noncopyable.hpp>

namespace rp
{

struct SkinVertex
{
    sm::vec3 position;
    sm::vec3 normal;
    sm::vec2 texcoord;
    uint32_t blend_indices;
    uint32_t blend_weights;
};

class SkinRenderer : public IRenderer, private RendererImpl<SkinVertex, unsigned short>, private boost::noncopyable
{
public:
    SkinRenderer();

    virtual void Flush() override;

    void Draw(const model::Model& model, const model::Model::Mesh& mesh,
        const pt0::Material& material, const pt0::RenderContext& ctx) const;

private:
    void InitShader();

    static std::shared_ptr<pt0::Shader> BuildShader(bool tex_map);

}; // SkinRenderer

}