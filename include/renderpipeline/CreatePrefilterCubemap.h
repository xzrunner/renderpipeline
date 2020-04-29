#pragma once

#include <unirender/typedef.h>

namespace ur { class Device; class Context; }

namespace rp
{

ur::TexturePtr CreatePrefilterCubemap(const ur::Device& dev, ur::Context& ctx, const ur::TexturePtr& cubemap);

}