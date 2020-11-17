#include "renderpipeline/HeightfieldGrayRenderer.h"

#include <heightfield/HeightField.h>
#include <unirender/Texture.h>
#include <unirender/ShaderProgram.h>
#include <unirender/ComponentDataType.h>
#include <unirender/VertexInputAttribute.h>
#include <shadertrans/ShaderTrans.h>
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

#version 330 core

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 texcoord;

layout(std140) uniform UBO_VS
{
    mat4 projection;
    mat4 view;
    mat4 model;

    vec2 inv_res;
} ubo_vs;

uniform sampler2D u_heightmap;

out VS_OUT {
#ifdef BUILD_NORMAL_MAP
    vec2  texcoord;
#endif // BUILD_NORMAL_MAP
    vec3  frag_pos;
    vec3  normal;
} vs_out;

vec3 ComputeNormalCentralDifference(vec2 position, float heightExaggeration)
{
    float leftHeight = texture(u_heightmap, position - vec2(1.0, 0.0) * ubo_vs.inv_res).r * heightExaggeration;
    float rightHeight = texture(u_heightmap, position + vec2(1.0, 0.0) * ubo_vs.inv_res).r * heightExaggeration;
    float bottomHeight = texture(u_heightmap, position - vec2(0.0, 1.0) * ubo_vs.inv_res).r * heightExaggeration;
    float topHeight = texture(u_heightmap, position + vec2(0.0, 1.0) * ubo_vs.inv_res).r * heightExaggeration;
    return normalize(vec3(leftHeight - rightHeight, 2.0, bottomHeight - topHeight));
}

void main()
{
    const float h_scale = 0.2;

	vec4 pos = position;
	pos.y = texture(u_heightmap, texcoord).r * h_scale;

#ifdef BUILD_NORMAL_MAP
    vs_out.texcoord = texcoord;
#endif // BUILD_NORMAL_MAP
    vs_out.frag_pos = vec3(ubo_vs.model * pos);
    vs_out.normal = ComputeNormalCentralDifference(texcoord, 500);

	gl_Position = ubo_vs.projection * ubo_vs.view * ubo_vs.model * pos;
}

)";

const char* fs = R"(

#version 330 core

#ifdef BUILD_NORMAL_MAP
uniform sampler2D u_normal_map;
#endif // BUILD_NORMAL_MAP

in VS_OUT {
#ifdef BUILD_NORMAL_MAP
    vec2  texcoord;
#endif // BUILD_NORMAL_MAP
    vec3  frag_pos;
    vec3  normal;
} fs_in;

void main()
{
//#ifdef BUILD_NORMAL_MAP
//    // fixme
//    //vec3 N = texture(u_normal_map, fs_in.texcoord).rgb;
//    vec3 N = normalize(texture(u_normal_map, fs_in.texcoord).rgb);
//#else
//    vec3 fdx = dFdx(fs_in.frag_pos);
//    vec3 fdy = dFdy(fs_in.frag_pos);
//    vec3 N = normalize(cross(fdx, fdy));
//#endif // BUILD_NORMAL_MAP
    vec3 N = fs_in.normal;

    vec3 light_dir = normalize(vec3(0, 1000, 1000) - fs_in.frag_pos);
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

//    // textures
//    m_height_map->Bind();
//#ifdef BUILD_NORMAL_MAP
//    m_normal_map->Bind();
//#endif // BUILD_NORMAL_MAP

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
    vals.AddVar("ubo_vs.inv_res", pt0::RenderVariant(inv_res));
    vals.Bind(*shader);
}

void HeightfieldGrayRenderer::Clear()
{
    HeightfieldRenderer::Clear();

    m_height_map.reset();
}

void HeightfieldGrayRenderer::BeforeDraw(ur::Context& ctx) const
{
    assert(m_shaders.size() == 1);
    auto shader = m_shaders.front();

    ctx.SetTexture(shader->QueryTexSlot("u_heightmap"), m_height_map);
#ifdef BUILD_NORMAL_MAP
    ctx.SetTexture(shader->QueryTexSlot("u_normal_map"), m_normal_map);
#endif // BUILD_NORMAL_MAP
}

void HeightfieldGrayRenderer::InitShader(const ur::Device& dev)
{
    std::vector<std::shared_ptr<ur::VertexInputAttribute>> vbuf_attrs(2);
    // rp::VERT_POSITION_NAME
    vbuf_attrs[0] = std::make_shared<ur::VertexInputAttribute>(
        0, ur::ComponentDataType::Float, 3, 0, 20
    );
    // rp::VERT_TEXCOORD_NAME
    vbuf_attrs[1] = std::make_shared<ur::VertexInputAttribute>(
        1, ur::ComponentDataType::Float, 2, 12, 20
    );
    m_va->SetVertexBufferAttrs(vbuf_attrs);

#ifdef BUILD_NORMAL_MAP
    auto shader = dev.CreateShaderProgram(
        "#define BUILD_NORMAL_MAP\n" + std::string(vs),
        "#define BUILD_NORMAL_MAP\n" + std::string(fs)
    );
#else
    std::vector<unsigned int> _vs, _fs;
    shadertrans::ShaderTrans::GLSL2SpirV(shadertrans::ShaderStage::VertexShader, vs, _vs);
    shadertrans::ShaderTrans::GLSL2SpirV(shadertrans::ShaderStage::PixelShader, fs, _fs);
    auto shader = dev.CreateShaderProgram(_vs, _fs);
#endif // BUILD_NORMAL_MAP


    shader->AddUniformUpdater(std::make_shared<pt0::ModelMatUpdater>(*shader, "ubo_vs.model"));
    shader->AddUniformUpdater(std::make_shared<pt3::ViewMatUpdater>(*shader, "ubo_vs.view"));
    shader->AddUniformUpdater(std::make_shared<pt3::ProjectMatUpdater>(*shader, "ubo_vs.projection"));

    m_shaders.resize(1);
    m_shaders[0] = shader;
}

}