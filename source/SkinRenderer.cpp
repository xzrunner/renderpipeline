#include "renderpipeline/SkinRenderer.h"

#include "renderpipeline/UniformNames.h"

#include <unirender/Blackboard.h>
#include <unirender/RenderContext.h>
#include <unirender/VertexAttrib.h>
#include <shaderweaver/typedef.h>
#include <shaderweaver/Evaluator.h>
#include <shaderweaver/node/ShaderUniform.h>
#include <shaderweaver/node/ShaderInput.h>
#include <shaderweaver/node/ShaderOutput.h>
#include <shaderweaver/node/PositionTrans.h>
#include <shaderweaver/node/FragPosTrans.h>
#include <shaderweaver/node/NormalTrans.h>
#include <shaderweaver/node/Phong.h>
#include <shaderweaver/node/CameraPos.h>
#include <shaderweaver/node/Vector1.h>
#include <shaderweaver/node/Vector3.h>
#include <shaderweaver/node/Skin.h>
#include <shaderweaver/node/SampleTex2D.h>
#include <shaderweaver/node/Multiply.h>
#include <shaderweaver/node/VertexShader.h>
#include <shaderweaver/node/FragmentShader.h>
#include <painting3/Shader.h>
#include <painting3/MaterialMgr.h>
#include <model/MeshGeometry.h>
#include <model/typedef.h>

namespace
{

const char* BLEND_INDICES_NAME = "blend_indices";
const char* BLEND_WEIGHTS_NAME = "blend_weights";

enum ShaderType
{
    SHADER_TEX_MAP = 0,
    SHADER_NO_TEX_MAP,
};

}

namespace rp
{

SkinRenderer::SkinRenderer()
{
    InitShader();
}

void SkinRenderer::Flush()
{
}

void SkinRenderer::Draw(const model::Model& model,
                        const model::Model::Mesh& mesh,
                        const pt0::Material& material,
                        const pt0::RenderContext& ctx) const
{
    auto& rc = ur::Blackboard::Instance()->GetRenderContext();

    auto& geo = mesh.geometry;

    bool do_tex_map = false;
    if ((geo.vertex_type & model::VERTEX_FLAG_TEXCOORDS))
    {
        auto& model_mat = model.materials[mesh.material];
        if (model_mat->diffuse_tex != -1) {
            do_tex_map = true;
        }
    }

    auto shader = do_tex_map ? m_shaders[SHADER_TEX_MAP] : m_shaders[SHADER_NO_TEX_MAP];
    shader->Use();

	auto mode = shader->GetDrawMode();

    material.Bind(*shader);
    ctx.Bind(*shader);

	for (auto& sub : geo.sub_geometries)
	{
		if (sub.index) {
			rc.DrawElementsVAO(mode, sub.offset, sub.count, geo.vao);
		} else {
			rc.DrawArraysVAO(mode, sub.offset, sub.count, geo.vao);
		}
	}
}

void SkinRenderer::InitShader()
{
    m_shaders.resize(2);
    m_shaders[SHADER_TEX_MAP]    = BuildShader(true);
    m_shaders[SHADER_NO_TEX_MAP] = BuildShader(false);
}

std::shared_ptr<pt0::Shader> SkinRenderer::BuildShader(bool tex_map)
{
    auto& rc = ur::Blackboard::Instance()->GetRenderContext();

    //////////////////////////////////////////////////////////////////////////
    // layout
    //////////////////////////////////////////////////////////////////////////

    std::vector<ur::VertexAttrib> layout;
    layout.emplace_back(VERT_POSITION_NAME, 3, 4, 40, 0);
    layout.emplace_back(VERT_NORMAL_NAME,   3, 4, 40, 12);
    layout.emplace_back(VERT_TEXCOORD_NAME, 2, 4, 40, 24);
    layout.emplace_back(BLEND_INDICES_NAME, 4, 1, 40, 32);
    layout.emplace_back(BLEND_WEIGHTS_NAME, 4, 1, 40, 36);
    rc.CreateVertexLayout(layout);

    //////////////////////////////////////////////////////////////////////////
    // vert
    //////////////////////////////////////////////////////////////////////////

    std::vector<sw::NodePtr> vert_nodes;

    // input attribute
    auto position = std::make_shared<sw::node::ShaderInput>(VERT_POSITION_NAME, sw::t_flt3);
	auto normal   = std::make_shared<sw::node::ShaderInput>(VERT_NORMAL_NAME,   sw::t_nor3);
    auto texcoord = std::make_shared<sw::node::ShaderInput>(VERT_TEXCOORD_NAME, sw::t_uv);

    // calc bone
    // vec4 obj_pos = u_bone_matrix[round2int(blend_indices.x)] * position * blend_weights.x;
    // obj_pos += u_bone_matrix[round2int(blend_indices.y)] * position * blend_weights.y;
    // obj_pos += u_bone_matrix[round2int(blend_indices.z)] * position * blend_weights.z;
    // obj_pos += u_bone_matrix[round2int(blend_indices.w)] * position * blend_weights.w;
    auto blend_indices = std::make_shared<sw::node::ShaderInput>(BLEND_INDICES_NAME, sw::t_flt4);
    auto blend_weights = std::make_shared<sw::node::ShaderInput>(BLEND_WEIGHTS_NAME, sw::t_flt4);
    auto skinned_pos = std::make_shared<sw::node::Skin>();
    sw::make_connecting({ position, 0 },      { skinned_pos, sw::node::Skin::ID_POSITION });
    sw::make_connecting({ blend_indices, 0 }, { skinned_pos, sw::node::Skin::ID_BLEND_INDICES });
    sw::make_connecting({ blend_weights, 0 }, { skinned_pos, sw::node::Skin::ID_BLEND_WEIGHTS });

    // prepare mvp mat
	auto projection = std::make_shared<sw::node::ShaderUniform>(PROJ_MAT_NAME,  sw::t_mat4);
	auto view       = std::make_shared<sw::node::ShaderUniform>(VIEW_MAT_NAME,  sw::t_mat4);
	auto model      = std::make_shared<sw::node::ShaderUniform>(MODEL_MAT_NAME, sw::t_mat4);

    // gl_Position =  u_projection * u_view * u_model * a_pos;
	auto pos_trans = std::make_shared<sw::node::PositionTrans>(4);
	sw::make_connecting({ projection, 0 },  { pos_trans, sw::node::PositionTrans::ID_PROJ });
	sw::make_connecting({ view, 0 },        { pos_trans, sw::node::PositionTrans::ID_VIEW });
	sw::make_connecting({ model, 0 },       { pos_trans, sw::node::PositionTrans::ID_MODEL });
	sw::make_connecting({ skinned_pos, 0 }, { pos_trans, sw::node::PositionTrans::ID_POS });
    auto vert_end = std::make_shared<sw::node::VertexShader>();
    sw::make_connecting({ pos_trans, 0 }, { vert_end, 0 });
    vert_nodes.push_back(vert_end);

	auto frag_pos_trans = std::make_shared<sw::node::FragPosTrans>();
	sw::make_connecting({ model, 0 },       { frag_pos_trans, sw::node::FragPosTrans::ID_MODEL });
	sw::make_connecting({ skinned_pos, 0 }, { frag_pos_trans, sw::node::FragPosTrans::ID_POS });

    auto normal_mat = std::make_shared<sw::node::ShaderUniform>(sw::node::NormalTrans::NormalMatName(), sw::t_mat3);
	auto norm_trans = std::make_shared<sw::node::NormalTrans>();
	sw::make_connecting({ normal_mat, 0 }, { norm_trans, sw::node::NormalTrans::ID_NORMAL_MAT });
	sw::make_connecting({ normal, 0 },     { norm_trans, sw::node::NormalTrans::ID_NORMAL });

    // v_texcoord = a_texcoord;
    auto v_texcoord = std::make_shared<sw::node::ShaderOutput>(FRAG_TEXCOORD_NAME, sw::t_uv);
    sw::make_connecting({ texcoord, 0 }, { v_texcoord, 0 });
    vert_nodes.push_back(v_texcoord);

    // v_world_pos = vec3(u_model * a_pos);
    auto v_world_pos = std::make_shared<sw::node::ShaderOutput>(FRAG_POSITION_NAME, sw::t_flt3);
    sw::make_connecting({ frag_pos_trans, 0 }, { v_world_pos, 0 });
    vert_nodes.push_back(v_world_pos);

    // v_normal = mat3(transpose(inverse(u_model))) * a_normal;
    auto v_normal = std::make_shared<sw::node::ShaderOutput>(FRAG_NORMAL_NAME, sw::t_nor3);
    sw::make_connecting({ norm_trans, 0 }, { v_normal, 0 });
    vert_nodes.push_back(v_normal);

    //////////////////////////////////////////////////////////////////////////
    // frag
    //////////////////////////////////////////////////////////////////////////

    // phong = ambient + diffuse + specular + emission;
    auto phong = std::make_shared<sw::node::Phong>();

    auto cam_pos = std::make_shared<sw::node::CameraPos>();
	auto frag_in_pos = std::make_shared<sw::node::ShaderInput>(FRAG_POSITION_NAME, sw::t_flt3);
	auto frag_in_nor = std::make_shared<sw::node::ShaderInput>(FRAG_NORMAL_NAME, sw::t_nor3);
    sw::make_connecting({ cam_pos, 0 },     { phong, sw::node::Phong::ID_VIEW_POS });
	sw::make_connecting({ frag_in_pos, 0 }, { phong, sw::node::Phong::ID_FRAG_POS });
	sw::make_connecting({ frag_in_nor, 0 }, { phong, sw::node::Phong::ID_NORMAL });

    auto& lit_pos_name = pt3::MaterialMgr::PositionUniforms::light_pos.name;
    auto lit_pos      = std::make_shared<sw::node::ShaderUniform>(lit_pos_name, sw::t_flt3);
    auto lit_ambient  = std::make_shared<sw::node::Vector3>("", sm::vec3(0.5f, 0.5f, 0.5f));
    auto lit_diffuse  = std::make_shared<sw::node::Vector3>("", sm::vec3(1.0f, 1.0f, 1.0f));
    auto lit_specular = std::make_shared<sw::node::Vector3>("", sm::vec3(1.0f, 1.0f, 1.0f));
    sw::make_connecting({ lit_pos, 0 },      { phong, sw::node::Phong::ID_LIT_POSITION });
    sw::make_connecting({ lit_ambient, 0 },  { phong, sw::node::Phong::ID_LIT_AMBIENT });
    sw::make_connecting({ lit_diffuse, 0 },  { phong, sw::node::Phong::ID_LIT_DIFFUSE });
    sw::make_connecting({ lit_specular, 0 }, { phong, sw::node::Phong::ID_LIT_SPECULAR });

    auto mat_diffuse   = std::make_shared<sw::node::ShaderUniform>(
        pt3::MaterialMgr::PhongUniforms::diffuse.name,   sw::t_flt3);
    auto mat_specular  = std::make_shared<sw::node::ShaderUniform>(
        pt3::MaterialMgr::PhongUniforms::specular.name,  sw::t_flt3);
    auto mat_shininess = std::make_shared<sw::node::ShaderUniform>(
        pt3::MaterialMgr::PhongUniforms::shininess.name, sw::t_flt1);
    //auto mat_emission  = std::make_shared<sw::node::ShaderUniform>("mat_emission",  sw::t_flt3);
    //auto mat_diffuse   = std::make_shared<sw::node::Vector3>("", sm::vec3(1, 1, 1));
    //auto mat_specular  = std::make_shared<sw::node::Vector3>("", sm::vec3(1, 1, 1));
    //auto mat_shininess = std::make_shared<sw::node::Vector1>("", 50.0f);
    auto mat_emission  = std::make_shared<sw::node::Vector3>("", sm::vec3(0, 0, 0));
    sw::make_connecting({ mat_diffuse,   0 }, { phong, sw::node::Phong::ID_MAT_DIFFUSE });
    sw::make_connecting({ mat_specular,  0 }, { phong, sw::node::Phong::ID_MAT_SPECULAR });
    sw::make_connecting({ mat_shininess, 0 }, { phong, sw::node::Phong::ID_MAT_SHININESS });
    sw::make_connecting({ mat_emission,  0 }, { phong, sw::node::Phong::ID_MAT_EMISSION });

    sw::NodePtr frag_end = std::make_shared<sw::node::FragmentShader>();
    std::vector<sw::NodePtr> cache_nodes;
    if (tex_map)
    {
        // frag_color = phong * texture2D(u_texture0, v_texcoord);
        auto tex_sample  = std::make_shared<sw::node::SampleTex2D>();
	    auto frag_in_tex = std::make_shared<sw::node::ShaderUniform>("u_texture0", sw::t_tex2d);
	    auto frag_in_uv  = std::make_shared<sw::node::ShaderInput>(FRAG_TEXCOORD_NAME, sw::t_uv);
	    sw::make_connecting({ frag_in_tex, 0 }, { tex_sample, sw::node::SampleTex2D::ID_TEX });
	    sw::make_connecting({ frag_in_uv,  0 }, { tex_sample, sw::node::SampleTex2D::ID_UV });
        cache_nodes.push_back(tex_sample);
        cache_nodes.push_back(frag_in_tex);
        cache_nodes.push_back(frag_in_uv);

        auto frag_color = std::make_shared<sw::node::Multiply>();
        sw::make_connecting({ tex_sample, 0 }, { frag_color, sw::node::Multiply::ID_A });
        sw::make_connecting({ phong, 0 },      { frag_color, sw::node::Multiply::ID_B });
        cache_nodes.push_back(frag_color);

        sw::make_connecting({ frag_color, 0 }, { frag_end, 0 });
    }
    else
    {
        // frag_color = phong;
        sw::make_connecting({ phong, 0 }, { frag_end, 0 });
    }

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

	std::vector<std::string> texture_names;
	pt3::Shader::Params sp(texture_names, layout);
	sp.vs = vert.GenShaderStr().c_str();
	sp.fs = frag.GenShaderStr().c_str();

	sp.uniform_names.Add(pt0::UniformTypes::ModelMat, MODEL_MAT_NAME);
	sp.uniform_names.Add(pt0::UniformTypes::ViewMat,  VIEW_MAT_NAME);
	sp.uniform_names.Add(pt0::UniformTypes::ProjMat,  PROJ_MAT_NAME);
    sp.uniform_names.Add(pt0::UniformTypes::CamPos,   sw::node::CameraPos::CamPosName());

	return std::make_shared<pt3::Shader>(&rc, sp);
}

}