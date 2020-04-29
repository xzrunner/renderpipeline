#pragma once

#include "renderpipeline/IRenderer.h"
#include "renderpipeline/RendererImpl.h"

#include <SM_Vector.h>
#include <unirender/ShaderProgram.h>
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
    MeshRenderer(const ur::Device& dev);

    virtual void Flush(ur::Context& ctx) override {}

    void Draw(ur::Context& ur_ctx, const model::MeshGeometry& mesh, const pt0::Material& material,
        const pt0::RenderContext& ctx, const std::shared_ptr<ur::ShaderProgram>& shader = nullptr,
        bool face = true) const;

private:
    void InitShader(const ur::Device& dev);

    enum class ShaderType
    {
        Base,
        Texture,
        Color
    };

    static std::shared_ptr<ur::ShaderProgram>
        CreateFaceShader(const ur::Device& dev, ShaderType type);
    static std::shared_ptr<ur::ShaderProgram>
        CreateEdgeShader(const ur::Device& dev);

}; // MeshRenderer

}