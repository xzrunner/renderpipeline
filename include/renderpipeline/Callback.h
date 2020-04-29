#pragma once

#include <SM_Rect.h>
#include <unirender/typedef.h>

#include <functional>

namespace rp
{

class Callback
{
public:
	struct Funs
	{
		std::function<const float*(size_t, const sm::irect&, ur::TexturePtr&)>  query_cached_tex_quad;
		std::function<void(const ur::TexturePtr&, const sm::irect& rect)> add_cache_symbol;
	};

	static void RegisterCallback(const Funs& funs);

	//////////////////////////////////////////////////////////////////////////

	static const float* QueryCachedTexQuad(size_t tex_id, const sm::irect& rect, ur::TexturePtr& out_tex);
	static void AddCacheSymbol(const ur::TexturePtr& tex, const sm::irect& rect);

}; // Callback

}