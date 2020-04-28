#pragma once

#include <SM_Rect.h>
#include <unirender2/typedef.h>

#include <functional>

namespace rp
{

class Callback
{
public:
	struct Funs
	{
		std::function<const float*(size_t, const sm::irect&, ur2::TexturePtr&)>  query_cached_tex_quad;
		std::function<void(const ur2::TexturePtr&, const sm::irect& rect)> add_cache_symbol;
	};

	static void RegisterCallback(const Funs& funs);

	//////////////////////////////////////////////////////////////////////////

	static const float* QueryCachedTexQuad(size_t tex_id, const sm::irect& rect, ur2::TexturePtr& out_tex);
	static void AddCacheSymbol(const ur2::TexturePtr& tex, const sm::irect& rect);

}; // Callback

}