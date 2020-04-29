#include "renderpipeline/HeightfieldGrayRenderer.h"

#include <heightfield/HeightField.h>
#include <unirender/Texture.h>
#include <unirender/ShaderProgram.h>
#include <unirender/ComponentDataType.h>
#include <unirender/VertexBufferAttribute.h>
#include <renderpipeline/UniformNames.h>
#include <painting0/ShaderUniforms.h>
#include <painting0/ModelMatUpdater.h>
#include <painting3/ViewMatUpdater.h>
#include <painting3/ProjectMatUpdater.h>
#include <painting3/Shader.h>
#include <model/TextureLoader.h>

namespace
{

const char* vs = R"(

attribute vec4 position;
attribute vec2 texcoord;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

uniform vec2 u_inv_res;

uniform sampler2D u_heightmap;

#ifdef BUILD_NORMAL_MAP
varying vec2 v_texcoord;
#endif // BUILD_NORMAL_MAP
varying vec3 v_fragpos;
varying vec3 v_normal;

vec3 ComputeNormalCentralDifference(vec2 position, float heightExaggeration)
{
    float leftHeight = texture2D(u_heightmap, position - vec2(1.0, 0.0) * u_inv_res).r * heightExaggeration;
    float rightHeight = texture2D(u_heightmap, position + vec2(1.0, 0.0) * u_inv_res).r * heightExaggeration;
    float bottomHeight = texture2D(u_heightmap, position - vec2(0.0, 1.0) * u_inv_res).r * heightExaggeration;
    float topHeight = texture2D(u_heightmap, position + vec2(0.0, 1.0) * u_inv_res).r * heightExaggeration;
    return normalize(vec3(leftHeight - rightHeight, 2.0, bottomHeight - topHeight));
}

void main()
{
    const float h_scale = 0.2;

	vec4 pos = position;
	pos.y = texture2D(u_heightmap, texcoord).r * h_scale;

#ifdef BUILD_NORMAL_MAP
    v_texcoord = texcoord;
#endif // BUILD_NORMAL_MAP
    v_fragpos = vec3(u_model * pos);
    v_normal = ComputeNormalCentralDifference(texcoord, 500);

	gl_Position = u_projection * u_view * u_model * pos;
}

)";

const char* fs = R"(

#ifdef BUILD_NORMAL_MAP
uniform sampler2D u_normal_map;
varying vec2 v_texcoord;
#endif // BUILD_NORMAL_MAP
varying vec3 v_fragpos;
varying vec3 v_normal;

void main()
{
//#ifdef BUILD_NORMAL_MAP
//    // fixme
//    //vec3 N = texture2D(u_normal_map, v_texcoord).rgb;
//    vec3 N = normalize(texture2D(u_normal_map, v_texcoord).rgb);
//#else
//    vec3 fdx = dFdx(v_fragpos);
//    vec3 fdy = dFdy(v_fragpos);
//    vec3 N = normalize(cross(fdx, fdy));
//#endif // BUILD_NORMAL_MAP
    vec3 N = v_normal;

    vec3 light_dir = normalize(vec3(0, 1000, 1000) - v_fragpos);
    float diff = max(dot(N, light_dir), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);
	gl_FragColor = vec4(diffuse, 1.0);
}

)";

}

namespace rp
{

HeightfieldGrayRenderer::HeightfieldGrayRenderer(const ur::Device& dev)
    : HeightfieldRenderer(dev)
{
    InitShader(dev);
}

void HeightfieldGrayRenderer::Setup(const ur::Device& dev, ur::Context& ctx,
                                    const std::shared_ptr<hf::HeightField>& hf)
{
    m_hf = hf;
    if (!m_hf) {
        return;
    }

    assert(hf);
    auto old = m_height_map;
    m_height_map = hf->GetHeightmap(dev);

#ifdef BUILD_NORMAL_MAP
    m_normal_map = terraingraph::TextureBaker::GenNormalMap(*hf, rc);
#endif // BUILD_NORMAL_MAP

    // textures
    m_height_map->Bind();
#ifdef BUILD_NORMAL_MAP
    m_normal_map->Bind();
#endif // BUILD_NORMAL_MAP

    // vertex buffer
    if (!old ||
        old->GetWidth() != m_height_map->GetWidth() ||
        old->GetHeight() != m_height_map->GetHeight()) {
        BuildVertBuf(ctx);
    }

    // bind shader
    auto shader = m_shaders.front();
//    shader->Bind();

    // update uniforms
    pt0::ShaderUniforms vals;
    sm::vec2 inv_res(1.0f / m_height_map->GetWidth(), 1.0f / m_height_map->GetHeight());
    vals.AddVar("u_inv_res", pt0::RenderVariant(inv_res));
    vals.Bind(*shader);
}

void HeightfieldGrayRenderer::Clear()
{
    HeightfieldRenderer::Clear();

    m_height_map.reset();
}

void HeightfieldGrayRenderer::InitShader(const ur::Device& dev)
{
    std::vector<std::shared_ptr<ur::VertexBufferAttribute>> vbuf_attrs(2);
    // rp::VERT_POSITION_NAME
    vbuf_attrs[0] = std::make_shared<ur::VertexBufferAttribute>(
        ur::ComponentDataType::Float, 3, 0, 20
    );
    // rp::VERT_TEXCOORD_NAME
    vbuf_attrs[1] = std::make_shared<ur::VertexBufferAttribute>(
        ur::ComponentDataType::Float, 2, 12, 20
    );
    m_va->SetVertexBufferAttrs(vbuf_attrs);

#ifdef BUILD_NORMAL_MAP
    auto shader = dev.CreateShaderProgram(
        "#define BUILD_NORMAL_MAP\n" + std::string(vs),
        "#define BUILD_NORMAL_MAP\n" + std::string(fs)
    );
#else
    auto shader = dev.CreateShaderProgram(vs, fs);
#endif // BUILD_NORMAL_MAP


    shader->AddUniformUpdater(std::make_shared<pt0::ModelMatUpdater>(*shader, MODEL_MAT_NAME));
    shader->AddUniformUpdater(std::make_shared<pt3::ViewMatUpdater>(*shader, VIEW_MAT_NAME));
    shader->AddUniformUpdater(std::make_shared<pt3::ProjectMatUpdater>(*shader, PROJ_MAT_NAME));

    m_shaders.resize(1);
    m_shaders[0] = shader;
}

}