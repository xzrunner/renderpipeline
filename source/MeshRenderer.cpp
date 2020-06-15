#include "renderpipeline/MeshRenderer.h"
#include "renderpipeline/UniformNames.h"

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
#include <shaderweaver/node/VertexShader.h>
#include <shaderweaver/node/FragmentShader.h>
#include <shaderweaver/node/SampleTex2D.h>
#include <shaderweaver/node/Multiply.h>
#include <unirender/DrawState.h>
#include <unirender/Context.h>
#include <unirender/VertexBufferAttribute.h>
#include <painting0/Material.h>
#include <painting0/ModelMatUpdater.h>
#include <painting0/CamPosUpdater.h>
#include <painting3/Shader.h>
#include <painting3/MaterialMgr.h>
#include <painting3/ViewMatUpdater.h>
#include <painting3/ProjectMatUpdater.h>
#include <model/MeshGeometry.h>
#include <model/typedef.h>

namespace
{

enum ShaderType
{
    SHADER_FACE_BASE = 0,
    SHADER_FACE_TEXTURE,
    SHADER_FACE_COLOR,
    SHADER_EDGE,

    SHADER_MAX_COUNT,
};

}

namespace rp
{

MeshRenderer::MeshRenderer(const ur::Device& dev)
    : RendererImpl(dev)
{
    InitShader(dev);
}

void MeshRenderer::Draw(ur::Context& ur_ctx,
                        const ur::DrawState& ds,
                        const model::MeshGeometry& mesh,
                        const pt0::Material& material,
                        const pt0::RenderContext& ctx,
                        bool face) const
{
    auto _ds = ds;
    if (!ds.program)
    {
        if (face) {
            if ((mesh.vertex_type & model::VERTEX_FLAG_TEXCOORDS) &&
                material.FetchVar(pt3::MaterialMgr::PhongUniforms::diffuse_tex.name) != nullptr) {
                _ds.program = m_shaders[SHADER_FACE_TEXTURE];
            } else if (mesh.vertex_type & model::VERTEX_FLAG_COLOR) {
                _ds.program = m_shaders[SHADER_FACE_COLOR];
            } else {
                _ds.program = m_shaders[SHADER_FACE_BASE];
            }
        } else {
            _ds.program = m_shaders[SHADER_EDGE];
        }
    }
    assert(_ds.program);

//    sd->Bind();
    material.Bind(*_ds.program);
    ctx.Bind(*_ds.program);

	auto& geo = mesh;
    if (!geo.vertex_array) {
        return;
    }

    auto mode = face ? ur::PrimitiveType::Triangles : ur::PrimitiveType::Lines;
    _ds.vertex_array = geo.vertex_array;
	for (auto& sub : geo.sub_geometries)
    {
        _ds.offset = sub.offset;
        _ds.count = sub.count;
        ur_ctx.Draw(mode, _ds, nullptr);
	}
}

void MeshRenderer::InitShader(const ur::Device& dev)
{
    m_shaders.resize(SHADER_MAX_COUNT);

    m_shaders[SHADER_FACE_BASE]    = CreateFaceShader(dev, ShaderType::Base);
    m_shaders[SHADER_FACE_TEXTURE] = CreateFaceShader(dev, ShaderType::Texture);
    m_shaders[SHADER_FACE_COLOR]   = CreateFaceShader(dev, ShaderType::Color);
    m_shaders[SHADER_EDGE]         = CreateEdgeShader(dev);
}

std::shared_ptr<ur::ShaderProgram>
MeshRenderer::CreateFaceShader(const ur::Device& dev, ShaderType type)
{
    //////////////////////////////////////////////////////////////////////////
    // layout
    //////////////////////////////////////////////////////////////////////////

    std::vector<std::shared_ptr<ur::VertexBufferAttribute>> vbuf_attrs;
    switch (type)
    {
    case ShaderType::Base:
        // VERT_POSITION_NAME
        vbuf_attrs.push_back(std::make_shared<ur::VertexBufferAttribute>(
            0, ur::ComponentDataType::Float, 3, 0, 24
        ));
        // VERT_NORMAL_NAME
        vbuf_attrs.push_back(std::make_shared<ur::VertexBufferAttribute>(
            1, ur::ComponentDataType::Float, 3, 12, 24
        ));
        break;
    case ShaderType::Texture:
        // VERT_POSITION_NAME
        vbuf_attrs.push_back(std::make_shared<ur::VertexBufferAttribute>(
            0, ur::ComponentDataType::Float, 3, 0, 32
        ));
        // VERT_NORMAL_NAME
        vbuf_attrs.push_back(std::make_shared<ur::VertexBufferAttribute>(
            1, ur::ComponentDataType::Float, 3, 12, 32
        ));
        // VERT_TEXCOORD_NAME
        vbuf_attrs.push_back(std::make_shared<ur::VertexBufferAttribute>(
            2, ur::ComponentDataType::Float, 2, 24, 32
        ));
        break;
    case ShaderType::Color:
        // VERT_POSITION_NAME
        vbuf_attrs.push_back(std::make_shared<ur::VertexBufferAttribute>(
            0, ur::ComponentDataType::Float, 3, 0, 32
        ));
        // VERT_NORMAL_NAME
        vbuf_attrs.push_back(std::make_shared<ur::VertexBufferAttribute>(
            1, ur::ComponentDataType::Float, 3, 12, 32
        ));
        // VERT_COLOR_NAME
        vbuf_attrs.push_back(std::make_shared<ur::VertexBufferAttribute>(
            2, ur::ComponentDataType::Float, 3, 24, 32
        ));
        break;
    default:
        assert(0);
    }



    //////////////////////////////////////////////////////////////////////////
    // vert
    //////////////////////////////////////////////////////////////////////////

    std::vector<sw::NodePtr> vert_nodes;

	auto projection = std::make_shared<sw::node::ShaderUniform>(PROJ_MAT_NAME,  sw::t_mat4);
	auto view       = std::make_shared<sw::node::ShaderUniform>(VIEW_MAT_NAME,  sw::t_mat4);
	auto model      = std::make_shared<sw::node::ShaderUniform>(MODEL_MAT_NAME, sw::t_mat4);

    auto position = std::make_shared<sw::node::ShaderInput>(VERT_POSITION_NAME, sw::t_flt3);
	auto normal   = std::make_shared<sw::node::ShaderInput>(VERT_NORMAL_NAME,   sw::t_nor3);
    auto texcoord = std::make_shared<sw::node::ShaderInput>(VERT_TEXCOORD_NAME, sw::t_uv);
    auto color    = std::make_shared<sw::node::ShaderInput>(VERT_COLOR_NAME,    sw::t_col3);

    // gl_Position =  u_projection * u_view * u_model * a_pos;
	auto pos_trans = std::make_shared<sw::node::PositionTrans>(4);
	sw::make_connecting({ projection, 0 }, { pos_trans, sw::node::PositionTrans::ID_PROJ });
	sw::make_connecting({ view, 0 },       { pos_trans, sw::node::PositionTrans::ID_VIEW });
	sw::make_connecting({ model, 0 },      { pos_trans, sw::node::PositionTrans::ID_MODEL });
	sw::make_connecting({ position, 0 },   { pos_trans, sw::node::PositionTrans::ID_POS });
    auto vert_end = std::make_shared<sw::node::VertexShader>();
    sw::make_connecting({ pos_trans, 0 }, { vert_end, 0 });
    vert_nodes.push_back(vert_end);

	auto frag_pos_trans = std::make_shared<sw::node::FragPosTrans>();
	sw::make_connecting({ model, 0 },    { frag_pos_trans, sw::node::FragPosTrans::ID_MODEL });
	sw::make_connecting({ position, 0 }, { frag_pos_trans, sw::node::FragPosTrans::ID_POS });

    auto normal_mat = std::make_shared<sw::node::ShaderUniform>(sw::node::NormalTrans::NormalMatName(), sw::t_mat3);
	auto norm_trans = std::make_shared<sw::node::NormalTrans>();
	sw::make_connecting({ normal_mat, 0 }, { norm_trans, sw::node::NormalTrans::ID_NORMAL_MAT });
	sw::make_connecting({ normal, 0 },     { norm_trans, sw::node::NormalTrans::ID_NORMAL });

    if (type == ShaderType::Texture)
    {
        // v_texcoord = a_texcoord;
        auto v_texcoord = std::make_shared<sw::node::ShaderOutput>(FRAG_TEXCOORD_NAME, sw::t_uv);
        sw::make_connecting({ texcoord, 0 }, { v_texcoord, 0 });
        vert_nodes.push_back(v_texcoord);
    }
    else if (type == ShaderType::Color)
    {
        // v_color = a_color;
        auto v_color = std::make_shared<sw::node::ShaderOutput>(FRAG_COLOR_NAME, sw::t_col3);
        sw::make_connecting({ color, 0 }, { v_color, 0 });
        vert_nodes.push_back(v_color);
    }

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

    auto frag_end = std::make_shared<sw::node::FragmentShader>();
    std::vector<sw::NodePtr> cache_nodes;
    switch (type)
    {
    case ShaderType::Base:
        // frag_color = phong;
        sw::make_connecting({ phong, 0 }, { frag_end, 0 });
        break;
    case ShaderType::Texture:
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
        break;
    case ShaderType::Color:
    {
        // frag_color = phong * v_color;

        auto v_color = std::make_shared<sw::node::ShaderInput>(FRAG_COLOR_NAME, sw::t_col3);
        cache_nodes.push_back(v_color);

        auto frag_color = std::make_shared<sw::node::Multiply>();
        sw::make_connecting({ v_color, 0 }, { frag_color, sw::node::Multiply::ID_A });
        sw::make_connecting({ phong, 0 },   { frag_color, sw::node::Multiply::ID_B });
        cache_nodes.push_back(frag_color);

        sw::make_connecting({ frag_color, 0 }, { frag_end, 0 });
    }
        break;
    default:
        assert(0);
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

    auto shader = dev.CreateShaderProgram(vert.GenShaderStr(), frag.GenShaderStr());
    shader->AddUniformUpdater(std::make_shared<pt0::ModelMatUpdater>(*shader, MODEL_MAT_NAME));
    shader->AddUniformUpdater(std::make_shared<pt3::ViewMatUpdater>(*shader, VIEW_MAT_NAME));
    shader->AddUniformUpdater(std::make_shared<pt3::ProjectMatUpdater>(*shader, PROJ_MAT_NAME));
    shader->AddUniformUpdater(std::make_shared<pt0::CamPosUpdater>(*shader, sw::node::CameraPos::CamPosName()));
    return shader;
}

std::shared_ptr<ur::ShaderProgram>
MeshRenderer::CreateEdgeShader(const ur::Device& dev)
{
    // layout
    std::vector<std::shared_ptr<ur::VertexBufferAttribute>> vbuf_attrs(3);
    // VERT_POSITION_NAME
    vbuf_attrs[0] = std::make_shared<ur::VertexBufferAttribute>(
        0, ur::ComponentDataType::Float, 3, 0, 32
    );
    // VERT_NORMAL_NAME
    vbuf_attrs[1] = std::make_shared<ur::VertexBufferAttribute>(
        1, ur::ComponentDataType::Float, 3, 12, 32
    );
    // VERT_TEXCOORD_NAME
    vbuf_attrs[2] = std::make_shared<ur::VertexBufferAttribute>(
        2, ur::ComponentDataType::Float, 2, 24, 32
    );

	// vert
	std::vector<sw::NodePtr> vert_nodes;

	auto projection = std::make_shared<sw::node::ShaderUniform>(PROJ_MAT_NAME,  sw::t_mat4);
	auto view       = std::make_shared<sw::node::ShaderUniform>(VIEW_MAT_NAME,  sw::t_mat4);
	auto model      = std::make_shared<sw::node::ShaderUniform>(MODEL_MAT_NAME, sw::t_mat4);

	auto position   = std::make_shared<sw::node::ShaderInput>  (VERT_POSITION_NAME, sw::t_pos3);

	auto pos_trans = std::make_shared<sw::node::PositionTrans>(4);
	sw::make_connecting({ projection, 0 }, { pos_trans, sw::node::PositionTrans::ID_PROJ });
	sw::make_connecting({ view,       0 }, { pos_trans, sw::node::PositionTrans::ID_VIEW });
	sw::make_connecting({ model,      0 }, { pos_trans, sw::node::PositionTrans::ID_MODEL });
	sw::make_connecting({ position,   0 }, { pos_trans, sw::node::PositionTrans::ID_POS });
    auto vert_end = std::make_shared<sw::node::VertexShader>();

    sw::make_connecting({ pos_trans, 0 }, { vert_end, 0 });
	vert_nodes.push_back(vert_end);

	//// varying
	//auto col_in_uv = std::make_shared<sw::node::ShaderInput>(VERT_COLOR_NAME, sw::t_flt4);
	//auto col_out_uv = std::make_shared<sw::node::ShaderOutput>(FRAG_COLOR_NAME, sw::t_flt4);
	//sw::make_connecting({ col_in_uv, 0 }, { col_out_uv, 0 });
	//vert_nodes.push_back(col_out_uv);

	// frag
    auto frag_col = std::make_shared<sw::node::Vector3>("", sm::vec3(0, 0, 0));
    auto frag_end = std::make_shared<sw::node::FragmentShader>();
    sw::make_connecting({ frag_col, 0 }, { frag_end, 0 });

	// end
	sw::Evaluator vert(vert_nodes);
	sw::Evaluator frag({ frag_end });

	//printf("//////////////////////////////////////////////////////////////////////////\n");
	//printf("%s\n", vert.GenShaderStr().c_str());
	//printf("//////////////////////////////////////////////////////////////////////////\n");
	//printf("%s\n", frag.GenShaderStr().c_str());
	//printf("//////////////////////////////////////////////////////////////////////////\n");

    auto shader = dev.CreateShaderProgram(vert.GenShaderStr(), frag.GenShaderStr());
    shader->AddUniformUpdater(std::make_shared<pt0::ModelMatUpdater>(*shader, MODEL_MAT_NAME));
    shader->AddUniformUpdater(std::make_shared<pt3::ViewMatUpdater>(*shader, VIEW_MAT_NAME));
    shader->AddUniformUpdater(std::make_shared<pt3::ProjectMatUpdater>(*shader, PROJ_MAT_NAME));
    shader->AddUniformUpdater(std::make_shared<pt0::CamPosUpdater>(*shader, sw::node::CameraPos::CamPosName()));
    return shader;
}

}