#include "renderpipeline/SpriteRenderer.h"
#include "renderpipeline/Callback.h"
#include "renderpipeline/RenderBuffer.h"
#include "renderpipeline/UniformNames.h"

#include <SM_Rect.h>
#include <sm_const.h>
#include <SM_Matrix.h>
#include <tessellation/Painter.h>
#include <tessellation/Palette.h>
#include <painting0/ModelMatUpdater.h>
#include <painting2/Shader.h>
#include <painting2/ViewMatUpdater.h>
#include <painting2/ProjectMatUpdater.h>
#include <unirender/ShaderProgram.h>
#include <unirender/VertexInputAttribute.h>
#include <unirender/Texture.h>
#include <unirender/TextureTarget.h>
#include <unirender/Factory.h>
#include <unirender/Descriptor.h>
#include <unirender/Adaptor.h>
#include <unirender/UniformBuffer.h>
#include <shadertrans/ShaderTrans.h>
#include <shaderweaver/typedef.h>
#include <shaderweaver/Evaluator.h>
#include <shaderweaver/node/ShaderUniform.h>
#include <shaderweaver/node/ShaderInput.h>
#include <shaderweaver/node/ShaderOutput.h>
#include <shaderweaver/node/PositionTrans.h>
#include <shaderweaver/node/SampleTex2D.h>
#include <shaderweaver/node/Multiply.h>
#include <shaderweaver/node/VertexShader.h>
#include <shaderweaver/node/FragmentShader.h>

#include <iostream>

namespace
{

void copy_vertex_buffer(const sm::mat4& mat, rp::RenderBuffer<rp::SpriteVertex, unsigned short>& dst,
	                    const tess::Painter::Buffer& src)
{
	dst.Reserve(src.indices.size(), src.vertices.size());
	for (auto& i : src.indices) {
		*dst.index_ptr++ = dst.curr_index + i;
	}
	for (auto& v : src.vertices)
	{
		dst.vert_ptr->pos = mat * v.pos;
		dst.vert_ptr->uv  = v.uv;
		dst.vert_ptr->col = v.col;
		++dst.vert_ptr;
	}
	dst.curr_index += static_cast<unsigned short>(src.vertices.size());
}

const char* vs = R"(

struct VS_INPUT
{
	float2 position : POSITION;
	float2 texcoord : TEXCOORD;
	float4 color    : COLOR;
};

cbuffer u_mvp
{
	float4x4 model;
	float4x4 view;
	float4x4 projection;
}

struct VS_OUTPUT
{
	float4 position : SV_Position;
	float4 color    : COLOR;
	float2 texcoord : TEXCOORD;
};

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	OUT.color = IN.color;
	OUT.texcoord = IN.texcoord;

    // for vulkan
	//OUT.position = mul(mul(mul(float4(IN.position.x, -IN.position.y, 0.0, 1.0), model), view), projection);
	OUT.position = mul(mul(mul(float4(IN.position.x, IN.position.y, 0.0, 1.0), model), view), projection);

	return OUT;
}

)";

const char* fs = R"(

Texture2D ColorTexture : register(t1);
SamplerState ColorSampler : register(s1);

struct VS_OUTPUT
{
	float4 position : SV_Position;
	float4 color    : COLOR;
	float2 texcoord : TEXCOORD;
};

float4 main(VS_OUTPUT IN) : SV_TARGET
{
	float4 color = ColorTexture.Sample(ColorSampler, IN.texcoord);
	return color * IN.color;
}

)";

struct {
	sm::mat4 model;
	sm::mat4 view;
	sm::mat4 projection;
} ubo_vs;

}

namespace rp
{

SpriteRenderer::SpriteRenderer(const ur::Device& dev, const ur::Context& ctx)
    : RendererImpl(dev)
{
	m_palette = std::make_unique<tess::Palette>(dev);

	InitShader(dev);
	InitRenderer(dev, ctx);

    m_rs = ur::DefaultRenderState2D();
}

void SpriteRenderer::Flush(ur::Context& ctx)
{
	// ubo
	if (m_uniform_buf) {
		m_uniform_buf->Update(&ubo_vs, sizeof(ubo_vs));
	}

    ctx.SetTexture(0, m_tex);

    auto fbo = ctx.GetFramebuffer();
    ctx.SetFramebuffer(m_fbo);
    FlushBuffer(ctx, ur::PrimitiveType::Triangles, m_rs, m_shaders[0], 
		m_desc_set, m_pipeline_layout, m_pipeline);
    ctx.SetFramebuffer(fbo);
}

void SpriteRenderer::DrawQuad(ur::Context& ctx, const ur::RenderState& rs,
                              const float* positions, const float* texcoords,
                              const ur::TexturePtr& tex, uint32_t color)
{
    if (m_buf.vertices.empty())
    {
        m_tex = tex;
        m_rs  = rs;
        m_fbo = ctx.GetFramebuffer();
    }
    else
    {
        if (m_tex != tex || m_rs != rs || m_fbo != ctx.GetFramebuffer())
        {
            Flush(ctx);

            m_tex = tex;
            m_rs  = rs;
            m_fbo = ctx.GetFramebuffer();
        }

        if (m_buf.vertices.size() + 4 >= RenderBuffer<SpriteVertex, unsigned short>::MAX_VERTEX_NUM) {
            Flush(ctx);
        }
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
		v.uv.x = texcoords[ptr];
		v.uv.y = texcoords[ptr + 1];
		v.col = color;
		ptr += 2;
	}
	m_buf.vert_ptr += 4;

	m_buf.curr_index += 4;
}

void SpriteRenderer::DrawPainter(ur::Context& ctx, const ur::RenderState& rs,
                                 const tess::Painter& pt, const sm::mat4& mat)
{
	if (pt.IsEmpty()) {
		return;
	}

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

	//if (m_palette->GetTexture() == m_tex)
	//{
	//	copy_vertex_buffer(mat, m_buf, pt.GetBuffer());
	//}
	//else
	{
        auto p_tex = m_palette->GetTexture();
		int tex_id = p_tex->GetTexID();
		int tex_w = p_tex->GetWidth();
		int tex_h = p_tex->GetHeight();
		sm::irect qr(0, 0, tex_w, tex_h);
        ur::TexturePtr cached_tex = nullptr;
		auto cached_texcoords = Callback::QueryCachedTexQuad(tex_id, qr, cached_tex);
		if (cached_texcoords)
		{
            if (cached_tex != m_tex)
            {
	            Flush(ctx);
	            m_tex = cached_tex;
            }

            if (m_buf.vertices.size() + pt.GetBuffer().vertices.size() >= RenderBuffer<SpriteVertex, unsigned short>::MAX_VERTEX_NUM) {
                Flush(ctx);
            }

            copy_vertex_buffer(mat, m_buf, pt.GetBuffer());

            float x = cached_texcoords[0];
            float y = cached_texcoords[1];
            float w = cached_texcoords[2] - cached_texcoords[0];
            float h = cached_texcoords[5] - cached_texcoords[1];
            size_t v_sz = pt.GetBuffer().vertices.size();

            assert(m_buf.curr_index - v_sz < m_buf.vertices.size());
            auto v_ptr = &m_buf.vertices[m_buf.curr_index - v_sz];
            for (size_t i = 0; i < v_sz; ++i)
            {
	            auto& v = *v_ptr++;
	            v.uv.x = x + w * v.uv.x;
	            v.uv.y = y + h * v.uv.y;
            }
		}
		else
		{
			Callback::AddCacheSymbol(p_tex, qr);

			Flush(ctx);
            m_tex = m_palette->GetTexture();
			copy_vertex_buffer(mat, m_buf, pt.GetBuffer());
		}
	}
}

void SpriteRenderer::InitShader(const ur::Device& dev)
{
	// layout
    std::vector<std::shared_ptr<ur::VertexInputAttribute>> vbuf_attrs(3);
    // vec2 position
    vbuf_attrs[0] = std::make_shared<ur::VertexInputAttribute>(
        0, ur::ComponentDataType::Float, 2, 0, 20
    );
    // vec2 texcoord
    vbuf_attrs[1] = std::make_shared<ur::VertexInputAttribute>(
        1, ur::ComponentDataType::Float, 2, 8, 20
    );
    // vec4 color
    vbuf_attrs[2] = std::make_shared<ur::VertexInputAttribute>(
        2, ur::ComponentDataType::UnsignedByte, 4, 16, 20
    );
    m_va->SetVertexBufferAttrs(vbuf_attrs);

	// vert
	std::vector<sw::NodePtr> vert_nodes;

	auto projection = std::make_shared<sw::node::ShaderUniform>(PROJ_MAT_NAME,  sw::t_mat4);
	auto view       = std::make_shared<sw::node::ShaderUniform>(VIEW_MAT_NAME,  sw::t_mat4);
	auto model      = std::make_shared<sw::node::ShaderUniform>(MODEL_MAT_NAME, sw::t_mat4);

	auto position   = std::make_shared<sw::node::ShaderInput>  (VERT_POSITION_NAME, sw::t_pos2);

	auto pos_trans = std::make_shared<sw::node::PositionTrans>(2);
	sw::make_connecting({ projection, 0 }, { pos_trans, sw::node::PositionTrans::ID_PROJ });
	sw::make_connecting({ view,       0 }, { pos_trans, sw::node::PositionTrans::ID_VIEW });
	sw::make_connecting({ model,      0 }, { pos_trans, sw::node::PositionTrans::ID_MODEL });
	sw::make_connecting({ position,   0 }, { pos_trans, sw::node::PositionTrans::ID_POS });
    auto vert_end = std::make_shared<sw::node::VertexShader>();
    sw::make_connecting({ pos_trans, 0 }, { vert_end, 0 });
    vert_nodes.push_back(vert_end);

	// varying
	auto vert_in_uv  = std::make_shared<sw::node::ShaderInput>(VERT_TEXCOORD_NAME, sw::t_uv);
	auto vert_out_uv = std::make_shared<sw::node::ShaderOutput>(FRAG_TEXCOORD_NAME, sw::t_uv);
	sw::make_connecting({ vert_in_uv, 0 }, { vert_out_uv, 0 });
	vert_nodes.push_back(vert_out_uv);

	auto col_in_uv = std::make_shared<sw::node::ShaderInput>(VERT_COLOR_NAME, sw::t_flt4);
	auto col_out_uv = std::make_shared<sw::node::ShaderOutput>(FRAG_COLOR_NAME, sw::t_flt4);
	sw::make_connecting({ col_in_uv, 0 }, { col_out_uv, 0 });
	vert_nodes.push_back(col_out_uv);

	// frag
	auto tex_sample = std::make_shared<sw::node::SampleTex2D>();
	auto frag_in_tex = std::make_shared<sw::node::ShaderUniform>("u_texture0", sw::t_tex2d);
	auto frag_in_uv = std::make_shared<sw::node::ShaderInput>(FRAG_TEXCOORD_NAME, sw::t_uv);
	sw::make_connecting({ frag_in_tex, 0 }, { tex_sample, sw::node::SampleTex2D::ID_TEX });
	sw::make_connecting({ frag_in_uv,  0 }, { tex_sample, sw::node::SampleTex2D::ID_UV });

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

    std::vector<std::string> attr_names = {
        "position",
        "texcoord",
        "color",
    };

	std::vector<unsigned int> _vs, _fs;
	try {
		shadertrans::ShaderTrans::HLSL2SpirV(shadertrans::ShaderStage::VertexShader, vs, _vs);
		shadertrans::ShaderTrans::HLSL2SpirV(shadertrans::ShaderStage::PixelShader, fs, _fs);
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
		return;
	}

	if (_vs.empty() || _fs.empty()) 
	{
		m_shaders.resize(1);
		m_shaders[0] = nullptr;
		return;
	}

	//std::string vs_glsl, fs_glsl;
	//shadertrans::ShaderTrans::SpirV2GLSL(shadertrans::ShaderStage::VertexShader, _vs, vs_glsl);
	//shadertrans::ShaderTrans::SpirV2GLSL(shadertrans::ShaderStage::PixelShader, _fs, fs_glsl);
	//printf("++++++++++++++++++ vs\n%s\n++++++++++++++++++ fs\n%s\n", vs_glsl.c_str(), fs_glsl.c_str());

    auto shader = dev.CreateShaderProgram(_vs, _fs);

    shader->AddUniformUpdater(std::make_shared<pt0::ModelMatUpdater>(*shader, "u_mvp.model", &ubo_vs.model));
    shader->AddUniformUpdater(std::make_shared<pt2::ViewMatUpdater>(*shader, "u_mvp.view", &ubo_vs.view));
    shader->AddUniformUpdater(std::make_shared<pt2::ProjectMatUpdater>(*shader, "u_mvp.projection", &ubo_vs.projection));

    m_shaders.resize(1);
    m_shaders[0] = shader;
}

void SpriteRenderer::InitRenderer(const ur::Device& dev, const ur::Context& ctx)
{
	m_uniform_buf = dev.CreateUniformBuffer(&ubo_vs, sizeof(ubo_vs));

	std::vector<std::shared_ptr<ur::DescriptorSetLayout>> desc_layouts = { dev.GetDescriptorSetLayout("single_ubo_single_img") };
	std::vector<ur::Descriptor> desc_descriptors = { { ur::DescriptorType::UniformBuffer, m_uniform_buf } };
	auto tex = m_palette->GetTexture();
	if (tex) {
		desc_descriptors.push_back(ur::Descriptor(ur::DescriptorType::CombinedImageSampler, tex));
	}
	m_desc_set = dev.CreateDescriptorSet(*dev.GetDescriptorPool(), desc_layouts, desc_descriptors);

	auto vert_buf = dev.CreateVertexBuffer(ur::BufferUsageHint::DynamicDraw, 0);
	m_va->SetVertexBuffer(vert_buf);
	m_va->SetVertexBufferAttrs({
        std::make_shared<ur::VertexInputAttribute>(0, ur::ComponentDataType::Float,        2,  0, 20),		// position
        std::make_shared<ur::VertexInputAttribute>(1, ur::ComponentDataType::Float,        2,  8, 20),		// texcoords
		std::make_shared<ur::VertexInputAttribute>(2, ur::ComponentDataType::UnsignedByte, 4, 16, 20)		// color
    });

	m_pipeline_layout = dev.GetPipelineLayout("single_ubo_single_img");
	m_pipeline = ctx.CreatePipeline(false, true, *m_pipeline_layout, *vert_buf, *m_shaders[0]);
}

}