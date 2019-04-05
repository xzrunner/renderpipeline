#pragma once

#include <vector>

namespace rp
{

template<typename T>
class RenderBuffer
{
public:
    void Reserve(size_t idx_count, size_t vtx_count)
    {
	    size_t sz = vertices.size();
	    vertices.resize(sz + vtx_count);
	    vert_ptr = vertices.data() + sz;

	    sz = indices.size();
	    indices.resize(sz + idx_count);
	    index_ptr = indices.data() + sz;
    }

    void Clear()
    {
	    vertices.resize(0);
	    indices.resize(0);

	    curr_index = 0;
	    vert_ptr   = nullptr;
	    index_ptr  = nullptr;
    }

public:
    // for indices use unsigned short
    static const int MAX_VERTEX_NUM = 0xffff;

public:
    std::vector<T>              vertices;
    std::vector<unsigned short> indices;

    unsigned short  curr_index = 0;
    T*              vert_ptr   = nullptr;
    unsigned short* index_ptr  = nullptr;

}; // RenderBuffer

}