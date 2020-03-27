#include "renderpipeline/SeparableSSS.h"
#include "renderpipeline/GlobalIllumination.h"

#define REGIST_NODE_TYPE(type, name)                          \
	rttr::registration::class_<rp::node::type>("rp::"#name)   \
		.constructor<>()                                      \
	;

RTTR_REGISTRATION
{

REGIST_NODE_TYPE(SeparableSSS, separable_sss)
REGIST_NODE_TYPE(GlobalIllumination, gi)

}

namespace rp
{

void regist_rttr()
{
}

}