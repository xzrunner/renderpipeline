#pragma once

#include <unirender/Texture.h>

#include "renderpipeline/HeightfieldRenderer.h"

//#define BUILD_NORMAL_MAP

namespace rp
{

class HeightfieldGrayRenderer : public HeightfieldRenderer
{
public:
    HeightfieldGrayRenderer();

    virtual void Flush() override {}

    virtual void Clear() override;
    virtual void Setup(const std::shared_ptr<hf::HeightField>& hf) override;

private:
    void InitShader();

private:
    ur::TexturePtr m_height_map = nullptr;
#ifdef BUILD_NORMAL_MAP
    ur::TexturePtr m_normal_map = nullptr;
#endif // BUILD_NORMAL_MAP

}; // HeightfieldGrayRenderer

}