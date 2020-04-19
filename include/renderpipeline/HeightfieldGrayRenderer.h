#pragma once

#include <unirender2/typedef.h>

#include "renderpipeline/HeightfieldRenderer.h"

//#define BUILD_NORMAL_MAP

namespace rp
{

class HeightfieldGrayRenderer : public HeightfieldRenderer
{
public:
    HeightfieldGrayRenderer(const ur2::Device& dev);

    virtual void Flush(ur2::Context& ctx) override {}

    virtual void Clear() override;
    virtual void Setup(const ur2::Device& dev, ur2::Context& ctx,
        const std::shared_ptr<hf::HeightField>& hf) override;

private:
    void InitShader(const ur2::Device& dev);

private:
    ur2::TexturePtr m_height_map = nullptr;
#ifdef BUILD_NORMAL_MAP
    ur2::TexturePtr m_normal_map = nullptr;
#endif // BUILD_NORMAL_MAP

}; // HeightfieldGrayRenderer

}