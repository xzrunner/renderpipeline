#include "renderpipeline/HeightfieldRenderer.h"

#include <heightfield/HeightField.h>
#include <painting0/Shader.h>
#include <painting0/ModelMatUpdater.h>
#include <unirender2/ShaderProgram.h>

namespace rp
{

HeightfieldRenderer::HeightfieldRenderer(const ur2::Device& dev)
    : RendererImpl(dev)
{
}

void HeightfieldRenderer::Clear()
{
    m_hf.reset();
}

void HeightfieldRenderer::Draw(ur2::Context& ctx, const sm::mat4& mt) const
{
    if (m_shaders.empty() || !m_hf) {
        return;
    }

    auto model_updater = m_shaders[0]->QueryUniformUpdater(ur2::GetUpdaterTypeID<pt0::ModelMatUpdater>());
    if (model_updater) {
        std::static_pointer_cast<pt0::ModelMatUpdater>(model_updater)->Update(mt);
    }

    DrawVertBuf(ctx);
}

void HeightfieldRenderer::BuildVertBuf(ur2::Context& ctx)
{
    const auto w = m_hf->Width();
    const auto h = m_hf->Height();
    assert(w > 0 && h > 0);

    m_buf.Clear();
    m_buf.Reserve((w - 1) * (h - 1) * 6, w * h);

    for (size_t y = 0; y < h; ++y) {
        for (size_t x = 0; x < w; ++x) {
            *m_buf.vert_ptr++ = HeightfieldVertex(x, y, w, h);
        }
    }

    for (size_t y = 0; y < h - 1; ++y) {
        for (size_t x = 0; x < w - 1; ++x) {
            size_t ll = y * w + x;
            size_t rl = ll + 1;
            size_t lh = ll + w;
            size_t rh = lh + 1;
            *m_buf.index_ptr++ = ll;
            *m_buf.index_ptr++ = lh;
            *m_buf.index_ptr++ = rh;
            *m_buf.index_ptr++ = ll;
            *m_buf.index_ptr++ = rh;
            *m_buf.index_ptr++ = rl;
        }
    }

    assert(m_shaders.size() == 1);
    ur2::RenderState rs;
    FlushBuffer(ctx, ur2::PrimitiveType::Triangles, rs, m_shaders[0]);
}

void HeightfieldRenderer::DrawVertBuf(ur2::Context& ctx) const
{
    assert(m_shaders.size() == 1);

    ur2::DrawState draw;
    draw.program = m_shaders[0];
    draw.vertex_array = m_va;
    ctx.Draw(ur2::PrimitiveType::Triangles, draw, nullptr);
}

}