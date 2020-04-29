#pragma once

#include <unirender/typedef.h>

#include "renderpipeline/HeightfieldRenderer.h"

//#define BUILD_NORMAL_MAP

namespace rp
{

class HeightfieldGrayRenderer : public HeightfieldRenderer
{
public:
    HeightfieldGrayRenderer(const ur::Device& dev);

    virtual void Flush(ur::Context& ctx) override {}

    virtual void Clear() override;
    virtual void Setup(const ur::Device& dev, ur::Context& ctx,
        const std::shared_ptr<hf::HeightField>& hf) override;

private:
    void InitShader(const ur::Device& dev);

private:
    ur::TexturePtr m_height_map = nullptr;
#ifdef BUILD_NORMAL_MAP
    ur::TexturePtr m_normal_map = nullptr;
#endif // BUILD_NORMAL_MAP

}; // HeightfieldGrayRenderer

}