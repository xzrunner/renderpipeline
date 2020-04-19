#pragma once

#include <unirender2/typedef.h>

namespace ur2 { class Device; class Context; }

namespace rp
{

// convert HDR equirectangular environment map to cubemap equivalent
ur2::TexturePtr HDREquirectangularToCubemap(const ur2::Device& dev, ur2::Context& ctx, const ur2::TexturePtr& equirectangular_map);

}