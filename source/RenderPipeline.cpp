#include "renderpipeline/RenderPipeline.h"

namespace rp
{

CU_SINGLETON_DEFINITION(RenderPipeline);

extern void regist_rttr();

RenderPipeline::RenderPipeline()
{
	regist_rttr();
}

}