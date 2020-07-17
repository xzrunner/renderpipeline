#include "renderpipeline/VolumeRenderer.h"
#include "renderpipeline/UniformNames.h"

#include <unirender/ComponentDataType.h>
#include <unirender/ShaderProgram.h>
#include <unirender/VertexBufferAttribute.h>
#include <shadertrans/ShaderTrans.h>
#include <painting0/ModelMatUpdater.h>
#include <painting3/Shader.h>
#include <painting3/Blackboard.h>
#include <painting3/ViewMatUpdater.h>
#include <painting3/ProjectMatUpdater.h>
#include <shaderweaver/typedef.h>
#include <shaderweaver/Evaluator.h>
#include <shaderweaver/node/ShaderUniform.h>
#include <shaderweaver/node/ShaderInput.h>
#include <shaderweaver/node/ShaderOutput.h>
#include <shaderweaver/node/PositionTrans.h>
#include <shaderweaver/node/SampleTex3D.h>
#include <shaderweaver/node/Multiply.h>
#include <shaderweaver/node/VertexShader.h>
#include <shaderweaver/node/FragmentShader.h>

namespace rp
{

VolumeRenderer::VolumeRenderer(const ur::Device& dev)
    : RendererImpl(dev)
{
	InitShader(dev);
}

void VolumeRenderer::Flush(ur::Context& ctx)
{
    ur::RenderState rs_new(m_rs);
    PrepareRenderState(rs_new);

    FlushBuffer(ctx, ur::PrimitiveType::Triangles, rs_new, m_shaders[0]);
}

void VolumeRenderer::DrawCube(ur::Context& ctx, const ur::RenderState& rs, const float* positions,
                              const float* texcoords, int texid, uint32_t color)
{
    if (m_buf.vertices.empty())
    {
        m_rs = rs;
    }
    else
    {
        if (m_rs != rs) {
            Flush(ctx);
            m_rs = rs;
        }
    }

	if (m_tex_id != texid) {
		Flush(ctx);
		m_tex_id = texid;
	}

	m_buf.Reserve(6, 4);

	m_buf.index_ptr[0] = m_buf.curr_index;
	m_buf.index_ptr[1] = m_buf.curr_index + 1;
	m_buf.index_ptr[2] = m_buf.curr_index + 2;
	m_buf.index_ptr[3] = m_buf.curr_index;
	m_buf.index_ptr[4] = m_buf.curr_index + 2;
	m_buf.index_ptr[5] = m_buf.curr_index + 3;
	m_buf.index_ptr += 6;

	int ptr = 0;
	for (int i = 0; i < 4; ++i)
	{
		auto& v = m_buf.vert_ptr[i];
		v.pos.x = positions[ptr];
		v.pos.y = positions[ptr + 1];
		v.pos.z = positions[ptr + 2];
		v.uv.x = texcoords[ptr];
		v.uv.y = texcoords[ptr + 1];
		v.uv.z = texcoords[ptr + 2];
		v.col = color;
		ptr += 3;
	}
	m_buf.vert_ptr += 4;

	m_buf.curr_index += 4;
}

void VolumeRenderer::InitShader(const ur::Device& dev)
{
	// layout
    std::vector<std::shared_ptr<ur::VertexBufferAttribute>> vbuf_attrs(3);
    vbuf_attrs[0] = std::make_shared<ur::VertexBufferAttribute>(
        0, ur::ComponentDataType::Float, 3, 0, 28
    );
    vbuf_attrs[1] = std::make_shared<ur::VertexBufferAttribute>(
        1, ur::ComponentDataType::Float, 3, 12, 28
    );
    vbuf_attrs[2] = std::make_shared<ur::VertexBufferAttribute>(
        2, ur::ComponentDataType::UnsignedByte, 4, 24, 28
    );

	// vert
	std::vector<sw::NodePtr> vert_nodes;

	auto projection = std::make_shared<sw::node::ShaderUniform>(PROJ_MAT_NAME, sw::t_mat4);
	auto view       = std::make_shared<sw::node::ShaderUniform>(VIEW_MAT_NAME,       sw::t_mat4);
	auto model      = std::make_shared<sw::node::ShaderUniform>(MODEL_MAT_NAME,      sw::t_mat4);

	auto position   = std::make_shared<sw::node::ShaderInput>  (VERT_POSITION_NAME,     sw::t_pos3);

	auto pos_trans = std::make_shared<sw::node::PositionTrans>(4);
	sw::make_connecting({ projection, 0 }, { pos_trans, sw::node::PositionTrans::ID_PROJ });
	sw::make_connecting({ view,       0 }, { pos_trans, sw::node::PositionTrans::ID_VIEW });
	sw::make_connecting({ model,      0 }, { pos_trans, sw::node::PositionTrans::ID_MODEL });
	sw::make_connecting({ position,   0 }, { pos_trans, sw::node::PositionTrans::ID_POS });
    auto vert_end = std::make_shared<sw::node::VertexShader>();
    sw::make_connecting({ pos_trans, 0 }, { vert_end, 0 });
    vert_nodes.push_back(vert_end);

	// varying
	auto vert_in_uv  = std::make_shared<sw::node::ShaderInput>(VERT_TEXCOORD_NAME, sw::t_uvw);
	auto vert_out_uv = std::make_shared<sw::node::ShaderOutput>(FRAG_TEXCOORD_NAME, sw::t_uvw);
	sw::make_connecting({ vert_in_uv, 0 }, { vert_out_uv, 0 });
	vert_nodes.push_back(vert_out_uv);

	auto col_in_uv = std::make_shared<sw::node::ShaderInput>(VERT_COLOR_NAME, sw::t_flt4);
	auto col_out_uv = std::make_shared<sw::node::ShaderOutput>(FRAG_COLOR_NAME, sw::t_flt4);
	sw::make_connecting({ col_in_uv, 0 }, { col_out_uv, 0 });
	vert_nodes.push_back(col_out_uv);

	// frag
	auto tex_sample = std::make_shared<sw::node::SampleTex3D>();
	auto frag_in_tex = std::make_shared<sw::node::ShaderUniform>("u_texture0", sw::t_tex3d);
	auto frag_in_uv = std::make_shared<sw::node::ShaderInput>(FRAG_TEXCOORD_NAME, sw::t_uvw);
	sw::make_connecting({ frag_in_tex, 0 }, { tex_sample, sw::node::SampleTex3D::ID_TEX });
	sw::make_connecting({ frag_in_uv,  0 }, { tex_sample, sw::node::SampleTex3D::ID_UV });

	auto mul = std::make_shared<sw::node::Multiply>();
	auto frag_in_col = std::make_shared<sw::node::ShaderInput>(FRAG_COLOR_NAME, sw::t_flt4);
	sw::make_connecting({ tex_sample, 0 }, { mul, sw::node::Multiply::ID_A});
	sw::make_connecting({ frag_in_col, 0 }, { mul, sw::node::Multiply::ID_B });

    auto frag_end = std::make_shared<sw::node::FragmentShader>();
    sw::make_connecting({ mul, 0 }, { frag_end, 0 });

	// end
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

    m_shaders.resize(1);
    m_shaders[0] = shader;
}

void VolumeRenderer::PrepareRenderState(ur::RenderState& rs)
{
	// enable alpha blend
    rs.blending.enabled = true;
    rs.blending.src = ur::BlendingFactor::SrcAlpha;
    rs.blending.dst = ur::BlendingFactor::OneMinusSrcAlpha;
	// enable alpha test
    rs.alpha_test.enabled = true;
    rs.alpha_test.function = ur::AlphaTestFunc::Greater;
    rs.alpha_test.ref = 0.05f;
	// disable depth test
    rs.depth_test.enabled = false;
	// disable face cull
    rs.facet_culling.enabled = false;
}

}