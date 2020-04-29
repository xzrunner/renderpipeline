#pragma once

#include <unirender/typedef.h>

namespace ur { class Device; class Context; }

namespace rp
{

// convert HDR equirectangular environment map to cubemap equivalent
ur::TexturePtr HDREquirectangularToCubemap(const ur::Device& dev, ur::Context& ctx, const ur::TexturePtr& equirectangular_map);

}