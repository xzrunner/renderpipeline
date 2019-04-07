#include "renderpipeline/RenderMgr.h"
#include "renderpipeline/IRenderer.h"
#include "renderpipeline/SpriteRenderer.h"
#include "renderpipeline/MeshRenderer.h"
#include "renderpipeline/SkinRenderer.h"
#include "renderpipeline/Shape3Renderer.h"
#include "renderpipeline/VolumeRenderer.h"
#include "renderpipeline/BSPRenderer.h"
#include "renderpipeline/MorphRenderer.h"
#include "renderpipeline/SkyboxRenderer.h"
#include "renderpipeline/ExternRenderer.h"

#include <painting2/Shader.h>
#include <painting3/Shader.h>

namespace rp
{

CU_SINGLETON_DEFINITION(RenderMgr);

RenderMgr::RenderMgr()
{
}

std::shared_ptr<IRenderer> RenderMgr::SetRenderer(RenderType type)
{
	if (m_curr_render != type)
	{
		if (m_curr_render > RenderType::NIL) {
			m_renderers[static_cast<int>(m_curr_render)]->Flush();
		}
		m_curr_render = type;
	}

	if (!m_renderers[static_cast<int>(m_curr_render)])
	{
		switch (m_curr_render)
		{
		case RenderType::NIL:
			m_renderers[static_cast<int>(RenderType::SPRITE)] = nullptr;
			break;
		case RenderType::SPRITE:
			m_renderers[static_cast<int>(RenderType::SPRITE)]
				= std::make_shared<SpriteRenderer>();
			break;
        case RenderType::MESH:
            m_renderers[static_cast<int>(RenderType::MESH)]
                = std::make_shared<MeshRenderer>();
            break;
        case RenderType::SKIN:
            m_renderers[static_cast<int>(RenderType::SKIN)]
                = std::make_shared<SkinRenderer>();
            break;
        case RenderType::SHAPE3D:
            m_renderers[static_cast<int>(RenderType::SHAPE3D)]
                = std::make_shared<Shape3Renderer>();
            break;
		case RenderType::TEX3D:
			m_renderers[static_cast<int>(RenderType::TEX3D)]
				= std::make_shared<VolumeRenderer>();
			break;
        case RenderType::BSP:
            m_renderers[static_cast<int>(RenderType::BSP)]
                = std::make_shared<BSPRenderer>();
            break;
        case RenderType::MORPH:
            m_renderers[static_cast<int>(RenderType::MORPH)]
                = std::make_shared<MorphRenderer>();
            break;
        case RenderType::SKYBOX:
            m_renderers[static_cast<int>(RenderType::SKYBOX)]
                = std::make_shared<SkyboxRenderer>();
            break;
		case RenderType::EXTERN:
			m_renderers[static_cast<int>(RenderType::EXTERN)]
				= std::make_shared<ExternRenderer>();
			break;
		}
	}
	return m_renderers[static_cast<int>(m_curr_render)];
}

std::shared_ptr<IRenderer> RenderMgr::GetRenderer(RenderType type)
{
    return m_renderers[static_cast<int>(type)];
}

void RenderMgr::BindWndCtx2D(std::shared_ptr<pt2::WindowContext>& wc) const
{
    for (int i = 0; i < static_cast<int>(RenderType::MAX_COUNT); ++i)
    {
        auto& rd = m_renderers[i];
        if (!rd) {
            continue;
        }
        for (auto& shader : rd->GetAllShaders()) {
            if (shader->get_type() == rttr::type::get<pt2::Shader>()) {
                std::static_pointer_cast<pt2::Shader>(shader)->AddNotify(wc);
            }
        }
    }
}

void RenderMgr::BindWndCtx3D(std::shared_ptr<pt3::WindowContext>& wc) const
{
    for (int i = 0; i < static_cast<int>(RenderType::MAX_COUNT); ++i)
    {
        auto& rd = m_renderers[i];
        if (!rd) {
            continue;
        }
        for (auto& shader : rd->GetAllShaders()) {
            if (shader->get_type() == rttr::type::get<pt3::Shader>()) {
                std::static_pointer_cast<pt3::Shader>(shader)->AddNotify(wc);
            }
        }
    }
}

void RenderMgr::UnbindWndCtx2D(std::shared_ptr<pt2::WindowContext>& wc) const
{
    for (int i = 0; i < static_cast<int>(RenderType::MAX_COUNT); ++i)
    {
        auto& rd = m_renderers[i];
        if (!rd) {
            continue;
        }
        for (auto& shader : rd->GetAllShaders()) {
            if (shader->get_type() == rttr::type::get<pt2::Shader>()) {
                std::static_pointer_cast<pt2::Shader>(shader)->RemoveNotify(wc);
            }
        }
    }
}

void RenderMgr::UnbindWndCtx3D(std::shared_ptr<pt3::WindowContext>& wc) const
{
    for (int i = 0; i < static_cast<int>(RenderType::MAX_COUNT); ++i)
    {
        auto& rd = m_renderers[i];
        if (!rd) {
            continue;
        }
        for (auto& shader : rd->GetAllShaders()) {
            if (shader->get_type() == rttr::type::get<pt3::Shader>()) {
                std::static_pointer_cast<pt3::Shader>(shader)->RemoveNotify(wc);
            }
        }
    }
}

void RenderMgr::Flush()
{
	auto shader = SetRenderer(m_curr_render);
	if (shader) {
		shader->Flush();
	}
}

}