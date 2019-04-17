#include "renderpipeline/VolumeRenderer.h"
#include "renderpipeline/UniformNames.h"

#include <unirender/Blackboard.h>
#include <unirender/RenderContext.h>
#include <painting3/Shader.h>
#include <painting3/Blackboard.h>
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

VolumeRenderer::VolumeRenderer()
{
	InitShader();
}

void VolumeRenderer::Flush()
{
    PrepareRenderState();
    FlushBuffer(ur::DRAW_TRIANGLES, m_shaders[0]);
	RestoreRenderState();
}

void VolumeRenderer::DrawCube(const float* positions, const float* texcoords, int texid, uint32_t color)
{
	if (m_tex_id != texid) {
		Flush();
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

void VolumeRenderer::InitShader()
{
	auto& rc = ur::Blackboard::Instance()->GetRenderContext();

	// layout
	std::vector<ur::VertexAttrib> layout;
	layout.push_back(ur::VertexAttrib(VERT_POSITION_NAME, 3, sizeof(float),    28, 0));
	layout.push_back(ur::VertexAttrib(VERT_TEXCOORD_NAME, 3, sizeof(float),    28, 12));
	layout.push_back(ur::VertexAttrib(VERT_COLOR_NAME,    4, sizeof(uint8_t),  28, 24));
	rc.CreateVertexLayout(layout);

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

	std::vector<std::string> texture_names;
	pt3::Shader::Params sp(texture_names, layout);
	sp.vs = vert.GenShaderStr().c_str();
	sp.fs = frag.GenShaderStr().c_str();

	sp.uniform_names.Add(pt0::UniformTypes::ModelMat, MODEL_MAT_NAME);
	sp.uniform_names.Add(pt0::UniformTypes::ViewMat,  VIEW_MAT_NAME);
	sp.uniform_names.Add(pt0::UniformTypes::ProjMat,  PROJ_MAT_NAME);
	auto& wc = pt3::Blackboard::Instance()->GetWindowContext();
	auto shader = std::make_shared<pt3::Shader>(&rc, sp);
    shader->AddNotify(std::const_pointer_cast<pt3::WindowContext>(wc));
    m_shaders.push_back(shader);
}

void VolumeRenderer::PrepareRenderState()
{
	auto& rc = ur::Blackboard::Instance()->GetRenderContext();

	// enable alpha blend
	rc.EnableBlend(true);
	rc.SetBlend(ur::BLEND_SRC_ALPHA, ur::BLEND_ONE_MINUS_SRC_ALPHA);
	// enable alpha test
	rc.SetAlphaTest(ur::ALPHA_GREATER, 0.05f);
	// disable depth test
	rc.SetZTest(ur::DEPTH_DISABLE);
	// disable face cull
	rc.SetCullMode(ur::CULL_DISABLE);
}

void VolumeRenderer::RestoreRenderState()
{
	auto& rc = ur::Blackboard::Instance()->GetRenderContext();

	// disable alpha blend
	rc.EnableBlend(false);
	// disable alpha test
	rc.SetAlphaTest(ur::ALPHA_DISABLE);
	// enable depth test
	rc.SetZTest(ur::DEPTH_LESS_EQUAL);
	rc.SetZWrite(true);
	// enable face cull
	rc.SetFrontFace(true);
	rc.SetCullMode(ur::CULL_BACK);
}

}