#include "renderpipeline/SeparableSSS.h"

#define REGIST_NODE_TYPE(name)                                \
	rttr::registration::class_<rp::node::name>("rp::"#name)   \
		.constructor<>()                                      \
	;

RTTR_REGISTRATION
{

REGIST_NODE_TYPE(SeparableSSS)

}

namespace rp
{

void regist_rttr()
{
}

}