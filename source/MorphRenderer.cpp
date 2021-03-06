#include "renderpipeline/MorphRenderer.h"
#include "renderpipeline/UniformNames.h"

#include <unirender/ShaderProgram.h>
#include <unirender/VertexInputAttribute.h>
#include <shadertrans/ShaderTrans.h>
#include <painting0/ModelMatUpdater.h>
#include <painting0/CamPosUpdater.h>
#include <painting3/Shader.h>
#include <painting3/MaterialMgr.h>
#include <painting3/ViewMatUpdater.h>
#include <painting3/ProjectMatUpdater.h>
#include <shaderweaver/typedef.h>
#include <shaderweaver/Evaluator.h>
#include <shaderweaver/node/ShaderUniform.h>
#include <shaderweaver/node/ShaderInput.h>
#include <shaderweaver/node/ShaderOutput.h>
#include <shaderweaver/node/PositionTrans.h>
#include <shaderweaver/node/VertexShader.h>
#include <shaderweaver/node/FragmentShader.h>
#include <shaderweaver/node/SampleTex2D.h>
#include <shaderweaver/node/Multiply.h>
#include <shaderweaver/node/CameraPos.h>
#include <shaderweaver/node/Subtract.h>
#include <shaderweaver/node/Add.h>
#include <shaderweaver/node/Vector1.h>

namespace
{

const char* VERT_POSE1_VERTEX_NAME = "pose1_vert";
const char* VERT_POSE1_NORMAL_NAME = "pose1_normal";
const char* VERT_POSE2_VERTEX_NAME = "pose2_vert";
const char* VERT_POSE2_NORMAL_NAME = "pose2_normal";

}

namespace rp
{

MorphRenderer::MorphRenderer(const ur::Device& dev)
    : RendererImpl(dev)
{
    InitShader(dev);
}

void MorphRenderer::Draw() const
{
//    m_shaders[0]->Bind();
}

void MorphRenderer::InitShader(const ur::Device& dev)
{
    //////////////////////////////////////////////////////////////////////////
    // layout
    //////////////////////////////////////////////////////////////////////////

    std::vector<std::shared_ptr<ur::VertexInputAttribute>> vbuf_attrs(3);
    // VERT_POSE1_VERTEX_NAME
    vbuf_attrs[0] = std::make_shared<ur::VertexInputAttribute>(
        0, ur::ComponentDataType::Float, 3, 0, 24
    );
    // VERT_POSE1_NORMAL_NAME
    vbuf_attrs[1] = std::make_shared<ur::VertexInputAttribute>(
        1, ur::ComponentDataType::Float, 3, 12, 24
    );
    // VERT_TEXCOORD_NAME
    vbuf_attrs[2] = std::make_shared<ur::VertexInputAttribute>(
        2, ur::ComponentDataType::Float, 2, 0, 0
    );

 //   std::vector<ur::VertexAttrib> layout;
	//layout.emplace_back(VERT_POSE1_VERTEX_NAME, 3, 4, 24,  0);
	//layout.emplace_back(VERT_POSE1_NORMAL_NAME, 3, 4, 24, 12);
	//layout.emplace_back(VERT_POSE2_VERTEX_NAME, 3, 4, 24,  0);
	//layout.emplace_back(VERT_POSE2_NORMAL_NAME, 3, 4, 24, 12);
	//layout.emplace_back(VERT_TEXCOORD_NAME,     2, 4,  0,  0);
 //   rc.CreateVertexLayout(layout);

    //////////////////////////////////////////////////////////////////////////
    // vert
    //////////////////////////////////////////////////////////////////////////

    std::vector<sw::NodePtr> vert_nodes;

	auto projection = std::make_shared<sw::node::ShaderUniform>(PROJ_MAT_NAME,  sw::t_mat4);
	auto view       = std::make_shared<sw::node::ShaderUniform>(VIEW_MAT_NAME,  sw::t_mat4);
	auto model      = std::make_shared<sw::node::ShaderUniform>(MODEL_MAT_NAME, sw::t_mat4);

    auto vertex1  = std::make_shared<sw::node::ShaderInput>(VERT_POSE1_VERTEX_NAME, sw::t_flt3);
    auto normall  = std::make_shared<sw::node::ShaderInput>(VERT_POSE1_NORMAL_NAME, sw::t_flt3);
    auto vertex2  = std::make_shared<sw::node::ShaderInput>(VERT_POSE2_VERTEX_NAME, sw::t_flt3);
    auto normal2  = std::make_shared<sw::node::ShaderInput>(VERT_POSE2_NORMAL_NAME, sw::t_flt3);
    auto texcoord = std::make_shared<sw::node::ShaderInput>(VERT_TEXCOORD_NAME, sw::t_uv);

    // 	vec4 pos = vertex1 + (vertex2 - vertex1) * u_blend;
    auto pos_tot = std::make_shared<sw::node::Subtract>();
    sw::make_connecting({ vertex2, 0 }, { pos_tot, sw::node::Subtract::ID_A});
    sw::make_connecting({ vertex1, 0 }, { pos_tot, sw::node::Subtract::ID_B});
    auto pos_off = std::make_shared<sw::node::Multiply>();
    auto blend = std::make_shared<sw::node::ShaderUniform>(
        pt3::MaterialMgr::AnimUniforms::blend.name, sw::t_flt1
    );
    sw::make_connecting({ pos_tot, 0 }, { pos_off, sw::node::Multiply::ID_A });
    sw::make_connecting({ blend,   0 }, { pos_off, sw::node::Multiply::ID_B });
    auto pos = std::make_shared<sw::node::Add>();
    sw::make_connecting({ vertex1, 0 }, { pos, sw::node::Add::ID_A });
    sw::make_connecting({ pos_off, 0 }, { pos, sw::node::Add::ID_B });

    // gl_Position =  u_projection * u_view * u_model * pos;
	auto pos_trans = std::make_shared<sw::node::PositionTrans>(3);
	sw::make_connecting({ projection, 0 }, { pos_trans, sw::node::PositionTrans::ID_PROJ });
	sw::make_connecting({ view, 0 },       { pos_trans, sw::node::PositionTrans::ID_VIEW });
	sw::make_connecting({ model, 0 },      { pos_trans, sw::node::PositionTrans::ID_MODEL });
	sw::make_connecting({ pos, 0 },        { pos_trans, sw::node::PositionTrans::ID_POS });
    auto vert_end = std::make_shared<sw::node::VertexShader>();
    sw::make_connecting({ pos_trans, 0 }, { vert_end, 0 });

    // v_texcoord = a_texcoord;
    auto v_texcoord = std::make_shared<sw::node::ShaderOutput>(FRAG_TEXCOORD_NAME, sw::t_uv);
    sw::make_connecting({ texcoord, 0 }, { v_texcoord, 0 });
    vert_nodes.push_back(v_texcoord);

    // v_color = vec4(pose1_normal * 0.001, 0);
    auto color = std::make_shared<sw::node::Multiply>();
    auto times = std::make_shared<sw::node::Vector1>("", 0.001f);
    sw::make_connecting({ normall, 0 }, { color, sw::node::Multiply::ID_A });
    sw::make_connecting({ times,   0 }, { color, sw::node::Multiply::ID_B });
    auto v_color = std::make_shared<sw::node::ShaderOutput>(FRAG_TEXCOORD_NAME, sw::t_uv);
    sw::make_connecting({ color, 0 }, { v_color, 0 });
    vert_nodes.push_back(v_color);

    //////////////////////////////////////////////////////////////////////////
    // frag
    //////////////////////////////////////////////////////////////////////////

    // vec4 base = texture2D(u_texture0, v_texcoord);
    auto tex_sample = std::make_shared<sw::node::SampleTex2D>();
    auto frag_in_tex = std::make_shared<sw::node::ShaderUniform>("u_texture0", sw::t_tex2d);
    auto frag_in_uv = std::make_shared<sw::node::ShaderInput>(FRAG_TEXCOORD_NAME, sw::t_uv);
    sw::make_connecting({ frag_in_tex, 0 }, { tex_sample, sw::node::SampleTex2D::ID_TEX });
    sw::make_connecting({ frag_in_uv,  0 }, { tex_sample, sw::node::SampleTex2D::ID_UV });

    // vec4 f_color = v_color * 0.0001
    auto f_color = std::make_shared<sw::node::Multiply>();
    auto f_times = std::make_shared<sw::node::Vector1>("", 0.0001f);
    sw::make_connecting({ v_color, 0 }, { f_color, sw::node::Multiply::ID_A });
    sw::make_connecting({ f_times, 0 }, { f_color, sw::node::Multiply::ID_B });

    // gl_FragColor = base * f_color;
    auto mul = std::make_shared<sw::node::Multiply>();
    sw::make_connecting({ tex_sample, 0 }, { mul, sw::node::Multiply::ID_A });
    sw::make_connecting({ f_color,    0 }, { mul, sw::node::Multiply::ID_B });

    auto frag_end = std::make_shared<sw::node::FragmentShader>();
    sw::make_connecting({ mul, 0 }, { frag_end, 0 });

    //////////////////////////////////////////////////////////////////////////
    // end
    //////////////////////////////////////////////////////////////////////////

	sw::Evaluator vert(vert_nodes);
	sw::Evaluator frag({ frag_end });

	//printf("//////////////////////////////////////////////////////////////////////////\n");
	//printf("%s\n", vert.GenShaderStr().c_str());
	//printf("//////////////////////////////////////////////////////////////////////////\n");
	//printf("%s\n", frag.GenShaderStr().c_str());
	//printf("//////////////////////////////////////////////////////////////////////////\n");

    std::vector<unsigned int> vs, fs;
    shadertrans::ShaderTrans::GLSL2SpirV(shadertrans::ShaderStage::VertexShader, vert.GenShaderStr(), vs);
    shadertrans::ShaderTrans::GLSL2SpirV(shadertrans::ShaderStage::PixelShader, frag.GenShaderStr(), fs);
    auto shader = dev.CreateShaderProgram(vs, fs);

    shader->AddUniformUpdater(std::make_shared<pt0::ModelMatUpdater>(*shader, MODEL_MAT_NAME));
    shader->AddUniformUpdater(std::make_shared<pt3::ViewMatUpdater>(*shader, VIEW_MAT_NAME));
    shader->AddUniformUpdater(std::make_shared<pt3::ProjectMatUpdater>(*shader, PROJ_MAT_NAME));
    shader->AddUniformUpdater(std::make_shared<pt0::CamPosUpdater>(*shader, sw::node::CameraPos::CamPosName()));

    m_shaders.resize(1);
    m_shaders[0] = shader;
}

}