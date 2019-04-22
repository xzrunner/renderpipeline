#include "renderpipeline/SeparableSSS.h"
#include "renderpipeline/GlobalIllumination.h"

#define REGIST_NODE_TYPE(name)                                \
	rttr::registration::class_<rp::node::name>("rp::"#name)   \
		.constructor<>()                                      \
	;

RTTR_REGISTRATION
{

REGIST_NODE_TYPE(SeparableSSS)
REGIST_NODE_TYPE(GlobalIllumination)

}

namespace rp
{

void regist_rttr()
{
}

}