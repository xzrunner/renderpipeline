#include "renderpipeline/CreateBrdfLutTex.h"
#include "renderpipeline/RenderMgr.h"

#include "shader/brdf.vert"
#include "shader/brdf.frag"

#include <unirender/Blackboard.h>
#include <unirender/RenderContext.h>
#include <unirender/Shader.h>
#include <unirender/Sandbox.h>

namespace rp
{

unsigned int CreateBrdfLutTex()
{
    unsigned brdf_lut_tex = 0;

    auto& rc = ur::Blackboard::Instance()->GetRenderContext();
    ur::Sandbox sb(rc);

    brdf_lut_tex = rc.CreateTexture(nullptr, 512, 512, ur::TEXTURE_RG16F);

    RenderMgr::Instance()->SetRenderer(RenderType::EXTERN);

    std::vector<std::string> textures;
    CU_VEC<ur::VertexAttrib> va_list;
    auto shader = std::make_shared<ur::Shader>(&rc, brdf_vs, brdf_fs, textures, va_list, true);
    shader->Use();

    rc.SetViewport(0, 0, 512, 512);

    auto rt = rc.CreateRenderTarget(0);
    rc.BindRenderTarget(rt);

    rc.BindRenderTargetTex(brdf_lut_tex);
    rc.SetClearFlag(ur::MASKC | ur::MASKD);
    rc.SetClearColor(0x88888888);
    rc.Clear();

    rc.RenderQuad();

    rc.UnbindRenderTarget();
    rc.ReleaseRenderTarget(rt);

    return brdf_lut_tex;
}

}