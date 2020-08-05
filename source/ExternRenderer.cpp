#include "renderpipeline/ExternRenderer.h"
#include "renderpipeline/Utility.h"

#include <unirender/Device.h>
#include <unirender/Context.h>
#include <unirender/DrawState.h>
#include <unirender/ShaderProgram.h>
#include <unirender/VertexArray.h>
#include <unirender/VertexInputAttribute.h>
#include <unirender/IndexBuffer.h>
#include <unirender/VertexBuffer.h>
#include <painting0/ModelMatUpdater.h>

namespace rp
{

ExternRenderer::ExternRenderer(const ur::Device& dev)
{
    m_va_tex = CreateVertexArray(dev);
    m_va_no_tex = CreateVertexArray(dev);
}

void ExternRenderer::DrawTexSpr(ur::Context& ctx,
                                const std::shared_ptr<ur::ShaderProgram>& shader,
                                const sm::mat4& mat) const
{
    auto model_updater = shader->QueryUniformUpdater(ur::GetUpdaterTypeID<pt0::ModelMatUpdater>());
    if (model_updater) {
        std::static_pointer_cast<pt0::ModelMatUpdater>(model_updater)->Update(mat);
    }

    ur::DrawState draw;
    draw.program = shader;
    draw.vertex_array = m_va_tex;
    ctx.Draw(ur::PrimitiveType::Triangles, draw, nullptr);
}

void ExternRenderer::DrawNoTexSpr(ur::Context& ctx,
                                  const std::shared_ptr<ur::ShaderProgram>& shader,
                                  const sm::mat4& mat) const
{
    auto model_updater = shader->QueryUniformUpdater(ur::GetUpdaterTypeID<pt0::ModelMatUpdater>());
    if (model_updater) {
        std::static_pointer_cast<pt0::ModelMatUpdater>(model_updater)->Update(mat);
    }

    ur::DrawState draw;
    draw.program = shader;
    draw.vertex_array = m_va_no_tex;
    ctx.Draw(ur::PrimitiveType::Triangles, draw, nullptr);
}

void ExternRenderer::InitRenderData(const ur::Device& dev)
{
    auto usage = ur::BufferUsageHint::StaticDraw;

	// tex
	{
        m_va_tex = dev.CreateVertexArray();

        unsigned short indices[] = { 0, 1, 2, 0, 2, 3 };
        size_t ibuf_sz = sizeof(indices);
        auto ibuf = dev.CreateIndexBuffer(usage, ibuf_sz);
        ibuf->ReadFromMemory(indices, ibuf_sz, 0);
        m_va_tex->SetIndexBuffer(ibuf);

		float vertices[] = {
			// pos          // tex
			-0.5f, -0.5f,   0.0f, 0.0f,
			 0.5f, -0.5f,   1.0f, 0.0f,
			 0.5f,  0.5f,   1.0f, 1.0f,
			-0.5f,  0.5f,   0.0f, 1.0f,
		};
        size_t vbuf_sz = sizeof(vertices);
        auto vbuf = dev.CreateVertexBuffer(usage, vbuf_sz);
        vbuf->ReadFromMemory(vertices, vbuf_sz, 0);
        m_va_tex->SetVertexBuffer(vbuf);

        std::vector<std::shared_ptr<ur::VertexInputAttribute>> vbuf_attrs(2);
        // pos
        vbuf_attrs[0] = std::make_shared<ur::VertexInputAttribute>(
            0, ur::ComponentDataType::Float, 2, 0, 16
        );
        // tex
        vbuf_attrs[1] = std::make_shared<ur::VertexInputAttribute>(
            1, ur::ComponentDataType::Float, 2, 8, 16
        );
        m_va_tex->SetVertexBufferAttrs(vbuf_attrs);
	}
	// no tex
	{
        m_va_no_tex = dev.CreateVertexArray();

        unsigned short indices[] = { 0, 1, 2, 0, 2, 3 };
        size_t ibuf_sz = sizeof(indices);
        auto ibuf = dev.CreateIndexBuffer(usage, ibuf_sz);
        ibuf->ReadFromMemory(indices, ibuf_sz, 0);
        m_va_no_tex->SetIndexBuffer(ibuf);

		float vertices[] = {
            -0.5f, -0.5f,
             0.5f, -0.5f,
             0.5f,  0.5f,
            -0.5f,  0.5f,
		};
        size_t vbuf_sz = sizeof(vertices);
        auto vbuf = dev.CreateVertexBuffer(usage, vbuf_sz);
        vbuf->ReadFromMemory(vertices, vbuf_sz, 0);
        m_va_no_tex->SetVertexBuffer(vbuf);

        std::vector<std::shared_ptr<ur::VertexInputAttribute>> vbuf_attrs(1);
        // pos
        vbuf_attrs[0] = std::make_shared<ur::VertexInputAttribute>(
            0, ur::ComponentDataType::Float, 2, 0, 8
        );
        m_va_no_tex->SetVertexBufferAttrs(vbuf_attrs);
	}
}

}