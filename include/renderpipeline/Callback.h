#pragma once

#include <SM_Rect.h>

#include <functional>

namespace rp
{

class Callback
{
public:
	struct Funs
	{
		std::function<const float*(size_t, const sm::irect&, int&)>  query_cached_tex_quad;
		std::function<void(size_t, int, int, const sm::irect& rect)> add_cache_symbol;
	};

	static void RegisterCallback(const Funs& funs);

	//////////////////////////////////////////////////////////////////////////

	static const float* QueryCachedTexQuad(size_t tex_id, const sm::irect& rect, int& out_tex_id);
	static void AddCacheSymbol(size_t tex_id, int tex_w, int tex_h, const sm::irect& rect);

}; // Callback

}