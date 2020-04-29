#pragma once

#include <unirender/typedef.h>

namespace ur { class Device; class Context; }

namespace rp
{

ur::TexturePtr CreateBrdfLutTex(const ur::Device& dev, ur::Context& ctx);

}