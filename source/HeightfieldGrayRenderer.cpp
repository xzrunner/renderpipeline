#include "renderpipeline/HeightfieldGrayRenderer.h"

#include <heightfield/HeightField.h>
#include <unirender/Blackboard.h>
#include <unirender/VertexAttrib.h>
#include <unirender/RenderContext.h>
#include <renderpipeline/UniformNames.h>
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

uniform sampler2D u_heightmap;

#ifdef BUILD_NORMAL_MAP
varying vec2 v_texcoord;
#endif // BUILD_NORMAL_MAP
varying vec3 v_fragpos;

void main()
{
    const float h_scale = 0.2;

	vec4 pos = position;
	pos.y = texture2D(u_heightmap, texcoord).r * h_scale;

#ifdef BUILD_NORMAL_MAP
    v_texcoord = texcoord;
#endif // BUILD_NORMAL_MAP
    v_fragpos = vec3(u_model * pos);
	gl_Position = u_projection * u_view * u_model * pos;
}

)";

const char* fs = R"(

#ifdef BUILD_NORMAL_MAP
uniform sampler2D u_normal_map;
varying vec2 v_texcoord;
#endif // BUILD_NORMAL_MAP
varying vec3 v_fragpos;

void main()
{
#ifdef BUILD_NORMAL_MAP
    // fixme
    //vec3 N = texture2D(u_normal_map, v_texcoord).rgb;
    vec3 N = normalize(texture2D(u_normal_map, v_texcoord).rgb);
#else
    vec3 fdx = dFdx(v_fragpos);
    vec3 fdy = dFdy(v_fragpos);
    vec3 N = normalize(cross(fdx, fdy));
#endif // BUILD_NORMAL_MAP

    vec3 light_dir = normalize(vec3(0, -10, 10) - v_fragpos);
    float diff = max(dot(N, light_dir), 0.0);
    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);
	gl_FragColor = vec4(diffuse, 1.0);
}

)";

}

namespace rp
{

HeightfieldGrayRenderer::HeightfieldGrayRenderer()
{
    InitShader();
}

void HeightfieldGrayRenderer::Setup(const std::shared_ptr<hf::HeightField>& hf)
{
    m_hf = hf;
    if (!m_hf) {
        return;
    }

    assert(hf);
    auto old = m_height_map;
    m_height_map = hf->GetHeightmap();

#ifdef BUILD_NORMAL_MAP
    auto& rc = ur::Blackboard::Instance()->GetRenderContext();
    m_normal_map = terraingraph::TextureBaker::GenNormalMap(*hf, rc);
#endif // BUILD_NORMAL_MAP

    // textures
    std::vector<uint32_t> texture_ids;
    texture_ids.push_back(m_height_map->TexID());
#ifdef BUILD_NORMAL_MAP
    texture_ids.push_back(m_normal_map->TexID());
#endif // BUILD_NORMAL_MAP

    assert(m_shaders.size() == 1);
    m_shaders.front()->SetUsedTextures(texture_ids);

    // vertex buffer
    if (!old ||
        old->Width() != m_height_map->Width() ||
        old->Height() != m_height_map->Height()) {
        BuildVertBuf();
    }
}

void HeightfieldGrayRenderer::Clear()
{
    HeightfieldRenderer::Clear();

    m_height_map.reset();
}

void HeightfieldGrayRenderer::InitShader()
{
	auto& rc = ur::Blackboard::Instance()->GetRenderContext();

    std::vector<ur::VertexAttrib> layout;
    layout.push_back(ur::VertexAttrib(rp::VERT_POSITION_NAME, 3, 4, 20, 0));
    layout.push_back(ur::VertexAttrib(rp::VERT_TEXCOORD_NAME, 2, 4, 20, 12));
    rc.CreateVertexLayout(layout);

    std::vector<std::string> texture_names;
    texture_names.push_back("u_heightmap");
#ifdef BUILD_NORMAL_MAP
    texture_names.push_back("u_normal_map");
#endif // BUILD_NORMAL_MAP

    pt3::Shader::Params sp(texture_names, layout);
#ifdef BUILD_NORMAL_MAP
    std::string _vs = "#define BUILD_NORMAL_MAP\n" + std::string(vs);
    std::string _fs = "#define BUILD_NORMAL_MAP\n" + std::string(fs);
    sp.vs = _vs.c_str();
    sp.fs = _fs.c_str();
#else
    sp.vs = vs;
    sp.fs = fs;
#endif // BUILD_NORMAL_MAP

    sp.uniform_names.Add(pt0::UniformTypes::ModelMat, rp::MODEL_MAT_NAME);
    sp.uniform_names.Add(pt0::UniformTypes::ViewMat,  rp::VIEW_MAT_NAME);
    sp.uniform_names.Add(pt0::UniformTypes::ProjMat,  rp::PROJ_MAT_NAME);
    //sp.uniform_names.Add(pt0::UniformTypes::CamPos, sw::node::CameraPos::CamPosName());

    auto shader = std::make_shared<pt3::Shader>(&rc, sp);
    m_shaders.push_back(shader);
}

}