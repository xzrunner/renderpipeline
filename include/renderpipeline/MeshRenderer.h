#pragma once

#include "renderpipeline/IRenderer.h"
#include "renderpipeline/RendererImpl.h"

#include <SM_Vector.h>
#include <painting0/Material.h>
#include <painting0/RenderContext.h>

#include <boost/noncopyable.hpp>

namespace model { struct MeshGeometry; }
namespace pt3 { class Shader; }

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
    MeshRenderer();

    virtual void Flush() override;

    void Draw(const model::MeshGeometry& mesh, const pt0::Material& material,
        const pt0::RenderContext& ctx, const std::shared_ptr<ur::Shader>& shader = nullptr,
        bool face = true) const;

private:
    void InitShader();

    enum class ShaderType
    {
        Base,
        Texture,
        Color
    };

    static std::shared_ptr<pt3::Shader> CreateFaceShader(ShaderType type);
    static std::shared_ptr<pt3::Shader> CreateEdgeShader();

}; // MeshRenderer

}