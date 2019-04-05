#include "renderpipeline/Callback.h"

namespace rp
{

static Callback::Funs FUNS;

void Callback::RegisterCallback(const Callback::Funs& funs)
{
	FUNS = funs;
}

const float* Callback::QueryCachedTexQuad(size_t tex_id, const sm::irect& rect, int& out_tex_id)
{
	return FUNS.query_cached_tex_quad(tex_id, rect, out_tex_id);
}

void Callback::AddCacheSymbol(size_t tex_id, int tex_w, int tex_h, const sm::irect& rect)
{
	FUNS.add_cache_symbol(tex_id, tex_w, tex_h, rect);
}

}