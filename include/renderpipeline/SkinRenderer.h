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
    SkinRenderer(const ur::Device& dev);

    virtual void Flush(ur::Context& ctx) override {}

    void Draw(ur::Context& ur_ctx, const model::Model& model, const model::Model::Mesh& mesh,
        const pt0::Material& material, const pt0::RenderContext& ctx) const;

private:
    void InitShader(const ur::Device& dev);

    static std::shared_ptr<ur::ShaderProgram>
        BuildShader(const ur::Device& dev, bool tex_map);

}; // SkinRenderer

}