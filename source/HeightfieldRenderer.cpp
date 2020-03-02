#include "renderpipeline/HeightfieldRenderer.h"

#include <heightfield/HeightField.h>
#include <painting0/Shader.h>

namespace rp
{

void HeightfieldRenderer::Clear()
{
    m_hf.reset();
}

void HeightfieldRenderer::Draw(const sm::mat4& mt) const
{
    if (m_shaders.empty() || !m_hf) {
        return;
    }

    m_shaders[0]->UpdateModelMat(mt);

    DrawVertBuf();
}

void HeightfieldRenderer::BuildVertBuf()
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
    FlushBuffer(ur::DRAW_TRIANGLES, m_shaders[0]);
}

void HeightfieldRenderer::DrawVertBuf() const
{
    assert(m_shaders.size() == 1);
    m_shaders[0]->Use();

    auto& rc = ur::Blackboard::Instance()->GetRenderContext();
    rc.BindBuffer(ur::INDEXBUFFER, m_ebo);
    rc.BindBuffer(ur::VERTEXBUFFER, m_vbo);
    size_t n = (m_hf->Width() - 1) * (m_hf->Height() - 1) * 6;
    rc.DrawElements(ur::DRAW_TRIANGLES, 0, n, false);
}

}