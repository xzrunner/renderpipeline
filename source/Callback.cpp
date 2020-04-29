#include "renderpipeline/Callback.h"

namespace rp
{

static Callback::Funs FUNS;

void Callback::RegisterCallback(const Callback::Funs& funs)
{
	FUNS = funs;
}

const float* Callback::QueryCachedTexQuad(size_t tex_id, const sm::irect& rect, ur::TexturePtr& out_tex)
{
	return FUNS.query_cached_tex_quad(tex_id, rect, out_tex);
}

void Callback::AddCacheSymbol(const ur::TexturePtr& tex, const sm::irect& rect)
{
	FUNS.add_cache_symbol(tex, rect);
}

}