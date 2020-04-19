#include "renderpipeline/ExternRenderer.h"
#include "renderpipeline/Utility.h"

#include <unirender2/Device.h>
#include <unirender2/Context.h>
#include <unirender2/DrawState.h>
#include <unirender2/ShaderProgram.h>
#include <unirender2/VertexArray.h>
#include <unirender2/VertexBufferAttribute.h>
#include <unirender2/IndexBuffer.h>
#include <unirender2/VertexBuffer.h>
#include <painting0/ModelMatUpdater.h>

namespace rp
{

ExternRenderer::ExternRenderer(const ur2::Device& dev)
{
    m_va_tex = CreateVertexArray(dev);
    m_va_no_tex = CreateVertexArray(dev);
}

void ExternRenderer::DrawTexSpr(ur2::Context& ctx,
                                const std::shared_ptr<ur2::ShaderProgram>& shader,
                                const sm::mat4& mat) const
{
    auto model_updater = shader->QueryUniformUpdater(ur2::GetUpdaterTypeID<pt0::ModelMatUpdater>());
    if (model_updater) {
        std::static_pointer_cast<pt0::ModelMatUpdater>(model_updater)->Update(mat);
    }

    ur2::DrawState draw;
    draw.program = shader;
    draw.vertex_array = m_va_tex;
    ctx.Draw(ur2::PrimitiveType::Triangles, draw, nullptr);
}

void ExternRenderer::DrawNoTexSpr(ur2::Context& ctx,
                                  const std::shared_ptr<ur2::ShaderProgram>& shader,
                                  const sm::mat4& mat) const
{
    auto model_updater = shader->QueryUniformUpdater(ur2::GetUpdaterTypeID<pt0::ModelMatUpdater>());
    if (model_updater) {
        std::static_pointer_cast<pt0::ModelMatUpdater>(model_updater)->Update(mat);
    }

    ur2::DrawState draw;
    draw.program = shader;
    draw.vertex_array = m_va_no_tex;
    ctx.Draw(ur2::PrimitiveType::Triangles, draw, nullptr);
}

void ExternRenderer::InitRenderData(const ur2::Device& dev)
{
    auto usage = ur2::BufferUsageHint::StaticDraw;

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

        std::vector<std::shared_ptr<ur2::VertexBufferAttribute>> vbuf_attrs(2);
        // pos
        vbuf_attrs[0] = std::make_shared<ur2::VertexBufferAttribute>(
            ur2::ComponentDataType::Float, 2, 0, 16
        );
        // tex
        vbuf_attrs[1] = std::make_shared<ur2::VertexBufferAttribute>(
            ur2::ComponentDataType::Float, 2, 8, 16
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

        std::vector<std::shared_ptr<ur2::VertexBufferAttribute>> vbuf_attrs(1);
        // pos
        vbuf_attrs[0] = std::make_shared<ur2::VertexBufferAttribute>(
            ur2::ComponentDataType::Float, 2, 0, 8
        );
        m_va_no_tex->SetVertexBufferAttrs(vbuf_attrs);
	}
}

}