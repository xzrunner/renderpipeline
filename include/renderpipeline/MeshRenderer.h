#pragma once

#include "renderpipeline/IRenderer.h"
#include "renderpipeline/RendererImpl.h"

#include <SM_Vector.h>
#include <unirender2/ShaderProgram.h>
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

class MeshRenderer : public IRenderer, private RendererImpl<MeshVertex, unsigned short>, private boost::noncopyable
{
public:
    MeshRenderer(const ur2::Device& dev);

    virtual void Flush(ur2::Context& ctx) override {}

    void Draw(ur2::Context& ur_ctx, const model::MeshGeometry& mesh, const pt0::Material& material,
        const pt0::RenderContext& ctx, const std::shared_ptr<ur2::ShaderProgram>& shader = nullptr,
        bool face = true) const;

private:
    void InitShader(const ur2::Device& dev);

    enum class ShaderType
    {
        Base,
        Texture,
        Color
    };

    static std::shared_ptr<ur2::ShaderProgram>
        CreateFaceShader(const ur2::Device& dev, ShaderType type);
    static std::shared_ptr<ur2::ShaderProgram>
        CreateEdgeShader(const ur2::Device& dev);

}; // MeshRenderer

}