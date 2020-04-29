#pragma once

#include "renderpipeline/IRenderer.h"
#include "renderpipeline/RendererImpl.h"

#include <SM_Vector.h>
#include <SM_Matrix.h>

#include <boost/noncopyable.hpp>

namespace hf { class HeightField; }

namespace rp
{

struct HeightfieldVertex
{
    HeightfieldVertex() {}
	HeightfieldVertex(size_t ix, size_t iz, size_t size_x, size_t size_z)
	{
        position.Set(
            ix / static_cast<float>(size_x),
            0,
            iz / static_cast<float>(size_z)
        );
		texcoords.Set(
            ix / static_cast<float>(size_x),
            iz / static_cast<float>(size_z)
        );
	}

	sm::vec3 position;
	sm::vec2 texcoords;
};

class HeightfieldRenderer : public rp::IRenderer, public rp::RendererImpl<HeightfieldVertex, uint32_t>, private boost::noncopyable
{
public:
    HeightfieldRenderer(const ur::Device& dev);

    virtual void Clear();
    virtual void Setup(const ur::Device& dev, ur::Context& ctx,
        const std::shared_ptr<hf::HeightField>& hf) = 0;

    void Draw(ur::Context& ctx, const sm::mat4& mt = sm::mat4()) const;

protected:
    void BuildVertBuf(ur::Context& ctx);
    void DrawVertBuf(ur::Context& ctx) const;

protected:
    std::shared_ptr<hf::HeightField> m_hf = nullptr;

}; // HeightfieldRenderer

}