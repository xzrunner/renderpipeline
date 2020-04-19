#pragma once

#include <cu/cu_macro.h>

#include <memory>
#include <array>

namespace pt2 { class WindowContext; }
namespace pt3 { class WindowContext; }
namespace ur2 { class Device; class Context; }

namespace rp
{

class IRenderer;

enum class RenderType
{
	NIL = 0,

	SPRITE,
    MESH,
    SKIN,
    SHAPE3D,
	TEX3D,
    BSP,
    MORPH,
    SKYBOX,
	EXTERN,

	MAX_COUNT
};

class RenderMgr
{
public:
	std::shared_ptr<IRenderer>
        SetRenderer(const ur2::Device& dev, ur2::Context& ctx, RenderType type);
    std::shared_ptr<IRenderer>
        GetRenderer(RenderType type);

    //void BindWndCtx2D(std::shared_ptr<pt2::WindowContext>& wc) const;
    //void BindWndCtx3D(std::shared_ptr<pt3::WindowContext>& wc) const;
    //void UnbindWndCtx2D(std::shared_ptr<pt2::WindowContext>& wc) const;
    //void UnbindWndCtx3D(std::shared_ptr<pt3::WindowContext>& wc) const;

	void Flush(const ur2::Device& dev, ur2::Context& ctx);

private:
	std::array<std::shared_ptr<IRenderer>, static_cast<int>(RenderType::MAX_COUNT)> m_renderers;

	RenderType m_curr_render = RenderType::NIL;

	CU_SINGLETON_DECLARATION(RenderMgr);

}; // RenderMgr

}