#include "renderpipeline/Shape3Renderer.h"
#include "renderpipeline/UniformNames.h"

#include <unirender/ShaderProgram.h>
#include <unirender/VertexArray.h>
#include <unirender/VertexBufferAttribute.h>
#include <shadertrans/ShaderTrans.h>
#include <painting0/ModelMatUpdater.h>
#include <painting3/Shader.h>
#include <painting3/ViewMatUpdater.h>
#include <painting3/ProjectMatUpdater.h>
#include <shaderweaver/typedef.h>
#include <shaderweaver/Evaluator.h>
#include <shaderweaver/node/ShaderUniform.h>
#include <shaderweaver/node/ShaderInput.h>
#include <shaderweaver/node/ShaderOutput.h>
#include <shaderweaver/node/PositionTrans.h>
#include <shaderweaver/node/Multiply.h>
#include <shaderweaver/node/VertexShader.h>
#include <shaderweaver/node/FragmentShader.h>

namespace rp
{

Shape3Renderer::Shape3Renderer(const ur::Device& dev)
    : RendererImpl(dev)
{
    InitShader(dev);
}

void Shape3Renderer::Flush(ur::Context& ctx)
{
    FlushBuffer(ctx, m_draw_mode, m_rs, m_shaders[0]);
}

void Shape3Renderer::DrawLines(ur::Context& ctx, const ur::RenderState& rs, size_t num,
                               const float* positions, uint32_t color)
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

    if (m_draw_mode != ur::PrimitiveType::Lines) {
        Flush(ctx);
        m_draw_mode = ur::PrimitiveType::Lines;
    }

    if (m_buf.vertices.size() + num >= RenderBuffer<Shape3Vertex, unsigned short>::MAX_VERTEX_NUM) {
        Flush(ctx);
    }

    m_buf.Reserve(num, num);

    int ptr = 0;
    for (size_t i = 0; i < num; ++i)
    {
        auto& v = m_buf.vert_ptr[i];
        v.pos.x = positions[ptr++];
        v.pos.y = positions[ptr++];
        v.pos.z = positions[ptr++];
        v.col = color;

        *m_buf.index_ptr++ = m_buf.curr_index + static_cast<unsigned short>(i);
    }
    m_buf.curr_index += static_cast<unsigned short>(num);
}

void Shape3Renderer::InitShader(const ur::Device& dev)
{
	// layout
    std::vector<std::shared_ptr<ur::VertexBufferAttribute>> vbuf_attrs(2);
    vbuf_attrs[0] = std::make_shared<ur::VertexBufferAttribute>(
        0, ur::ComponentDataType::Float, 3, 0, 16
    );
    vbuf_attrs[1] = std::make_shared<ur::VertexBufferAttribute>(
        1, ur::ComponentDataType::UnsignedByte, 4, 12, 16
    );
    m_va->SetVertexBufferAttrs(vbuf_attrs);

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
	auto col_in_uv = std::make_shared<sw::node::ShaderInput>(VERT_COLOR_NAME, sw::t_flt4);
	auto col_out_uv = std::make_shared<sw::node::ShaderOutput>(FRAG_COLOR_NAME, sw::t_flt4);
	sw::make_connecting({ col_in_uv, 0 }, { col_out_uv, 0 });
	vert_nodes.push_back(col_out_uv);

	// frag
	auto frag_in_col = std::make_shared<sw::node::ShaderInput>(FRAG_COLOR_NAME, sw::t_flt4);
    auto frag_end = std::make_shared<sw::node::FragmentShader>();
    sw::make_connecting({ frag_in_col, 0 }, { frag_end, 0 });

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

}