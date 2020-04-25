#include "renderpipeline/SkyboxRenderer.h"
#include "renderpipeline/UniformNames.h"

#include <unirender2/ShaderProgram.h>
#include <unirender2/Device.h>
#include <unirender2/Context.h>
#include <unirender2/Texture.h>
#include <unirender2/DrawState.h>
#include <painting3/Shader.h>
#include <painting3/ViewMatUpdater.h>
#include <painting3/ProjectMatUpdater.h>
#include <shaderweaver/typedef.h>
#include <shaderweaver/Evaluator.h>
#include <shaderweaver/node/ShaderUniform.h>
#include <shaderweaver/node/ShaderInput.h>
#include <shaderweaver/node/ShaderOutput.h>
#include <shaderweaver/node/MatrixOnlyRot.h>
#include <shaderweaver/node/Multiply.h>
#include <shaderweaver/node/VertexShader.h>
#include <shaderweaver/node/Swizzle.h>
#include <shaderweaver/node/SampleCube.h>
#include <shaderweaver/node/Tonemap.h>
#include <shaderweaver/node/GammaCorrect.h>
#include <shaderweaver/node/CombineTwo.h>
#include <shaderweaver/node/Vector1.h>
#include <shaderweaver/node/FragmentShader.h>

namespace rp
{

SkyboxRenderer::SkyboxRenderer(const ur2::Device& dev)
{
    InitShader(dev);
}

void SkyboxRenderer::Draw(ur2::Context& ctx, const ur2::Texture& cube_tex) const
{
    cube_tex.Bind();

    ur2::DrawState draw;
    draw.program = m_shaders[0];
    ctx.DrawCube(ur2::Context::VertexLayout::Pos, draw);
}

void SkyboxRenderer::InitShader(const ur2::Device& dev)
{
    //////////////////////////////////////////////////////////////////////////
    // layout
    //////////////////////////////////////////////////////////////////////////


    //////////////////////////////////////////////////////////////////////////
    // vert
    //////////////////////////////////////////////////////////////////////////

    std::vector<sw::NodePtr> vert_nodes;

    auto projection = std::make_shared<sw::node::ShaderUniform>(PROJ_MAT_NAME, sw::t_mat4);
	auto view       = std::make_shared<sw::node::ShaderUniform>(VIEW_MAT_NAME, sw::t_mat4);

    // mat4 view_rot = mat4(mat3(view));
    auto view_rot = std::make_shared<sw::node::MatrixOnlyRot>();
    sw::make_connecting({ view, 0 }, { view_rot, 0 });

    auto position = std::make_shared<sw::node::ShaderInput>(VERT_POSITION_NAME, sw::t_pos3);

    // vec4 clip_pos = projection * view_rot * vec4(position, 1.0);
    auto clip_pos = std::make_shared<sw::node::Multiply>();
    clip_pos->SetInputPortCount(3);
    sw::make_connecting({ projection, 0 }, { clip_pos, 0 });
    sw::make_connecting({ view_rot, 0 },   { clip_pos, 1 });
    sw::make_connecting({ position, 0 },   { clip_pos, 2 });

    // vec4 clip_pos_z1 = clip_pos.xyww;
    auto clip_pos_z1 = std::make_shared<sw::node::Swizzle>();
    clip_pos_z1->SetChannels({
        sw::node::Swizzle::CHANNEL_R,
        sw::node::Swizzle::CHANNEL_G,
        sw::node::Swizzle::CHANNEL_A,
        sw::node::Swizzle::CHANNEL_A,
    });
    sw::make_connecting({ clip_pos, 0 }, { clip_pos_z1, 0 });

    // end
    auto vert_end = std::make_shared<sw::node::VertexShader>();
    sw::make_connecting({ clip_pos_z1, 0 }, { vert_end, 0 });
    vert_nodes.push_back(vert_end);

    // v_world_pos = position;
    auto v_world_pos = std::make_shared<sw::node::ShaderOutput>(FRAG_POSITION_NAME, sw::t_flt3);
    sw::make_connecting({ position, 0 }, { v_world_pos, 0 });
    vert_nodes.push_back(v_world_pos);

    //////////////////////////////////////////////////////////////////////////
    // frag
    //////////////////////////////////////////////////////////////////////////

    // vec4 frag_color = textureCube(u_texture0, v_world_pos);
    auto tex_sample = std::make_shared<sw::node::SampleCube>();
    auto frag_in_tex = std::make_shared<sw::node::ShaderUniform>("u_texture0", sw::t_tex_cube);
    auto frag_in_dir = std::make_shared<sw::node::ShaderInput>(FRAG_POSITION_NAME, sw::t_flt3);
    sw::make_connecting({ frag_in_tex, 0 }, { tex_sample, sw::node::SampleCube::ID_TEX });
    sw::make_connecting({ frag_in_dir, 0 }, { tex_sample, sw::node::SampleCube::ID_DIR });

    // HDR tonemap
    auto tonemap = std::make_shared<sw::node::Tonemap>();
    sw::make_connecting({ tex_sample, sw::node::SampleCube::ID_RGBA }, { tonemap, 0 });

    // gamma correct
    auto gamma = std::make_shared<sw::node::GammaCorrect>();
    sw::make_connecting({ tonemap, 0 }, { gamma, 0 });

    // frag_color
    auto frag_color = std::make_shared<sw::node::CombineTwo>();
    sw::make_connecting({ gamma, 0 }, { frag_color, sw::node::CombineTwo::ID_A });
    auto vec_1 = std::make_shared<sw::node::Vector1>("", 1.0f);
    sw::make_connecting({ vec_1, 0 }, { frag_color, sw::node::CombineTwo::ID_B });

    // end
    auto frag_end = std::make_shared<sw::node::FragmentShader>();
    sw::make_connecting({ frag_color, 0 }, { frag_end, 0 });

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

    auto shader = dev.CreateShaderProgram(vert.GenShaderStr(), frag.GenShaderStr());
    shader->AddUniformUpdater(std::make_shared<pt3::ViewMatUpdater>(*shader, VIEW_MAT_NAME));
    shader->AddUniformUpdater(std::make_shared<pt3::ProjectMatUpdater>(*shader, PROJ_MAT_NAME));

    m_shaders.resize(1);
    m_shaders[0] = shader;
}

}