#pragma once

#include <unirender2/typedef.h>

namespace ur2 { class Device; class Context; }

namespace rp
{

ur2::TexturePtr CreateBrdfLutTex(const ur2::Device& dev, ur2::Context& ctx);

}