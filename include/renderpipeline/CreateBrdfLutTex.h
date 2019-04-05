#pragma once

#include <unirender/typedef.h>

namespace rp
{

static const int BRDF_LUT_TEX_SIZE = 512;
static const ur::TEXTURE_FORMAT BRDF_LUT_TEX_FMT = ur::TEXTURE_RG16F;

unsigned int CreateBrdfLutTex();

}