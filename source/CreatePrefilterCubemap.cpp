#include "renderpipeline/CreatePrefilterCubemap.h"
#include "renderpipeline/CubemapHelper.h"
#include "renderpipeline/RenderMgr.h"

#include "shader/cubemap.vert"
#include "shader/prefilter.frag"

#include <unirender/Blackboard.h>
#include <unirender/RenderContext.h>
#include <unirender/Shader.h>
#include <unirender/Sandbox.h>

namespace rp
{

unsigned int CreatePrefilterCubemap(unsigned int cubemap)
{
    unsigned int prefilter_map = 0;

    const unsigned int MAX_MIP_LEVELS = 5;

    auto& rc = ur::Blackboard::Instance()->GetRenderContext();
    ur::Sandbox sb(rc);

    prefilter_map = rc.CreateTextureCube(128, 128, MAX_MIP_LEVELS);

    //// todo
    //glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LEQUAL); // set depth function to less than AND equal for skybox depth trick.

    RenderMgr::Instance()->SetRenderer(RenderType::EXTERN);

    std::vector<std::string> textures;
    CU_VEC<ur::VertexAttrib> va_list;
    auto shader = std::make_shared<ur::Shader>(&rc, cubemap_vs, prefilter_fs, textures, va_list, true);
    shader->Use();

    shader->SetInt("environmentMap", 0);

    glm::mat4 capture_proj = CubemapHelper::CalcCaptureProjection();
    shader->SetMat4("projection", &capture_proj[0][0]);

    rc.BindTexture(cubemap, 0);

    glm::mat4 capture_views[6];
    CubemapHelper::CalcCaptureViews(capture_views);

    auto rt = rc.CreateRenderTarget(0);
    rc.BindRenderTarget(rt);
    for (unsigned int mip = 0; mip < MAX_MIP_LEVELS; ++mip)
    {
        // reisze framebuffer according to mip-level size.
        unsigned int mip_width  = static_cast<unsigned int>(128 * std::pow(0.5f, mip));
        unsigned int mip_height = static_cast<unsigned int>(128 * std::pow(0.5f, mip));
        rc.SetViewport(0, 0, mip_width, mip_height);

        float roughness = (float)mip / (float)(MAX_MIP_LEVELS - 1);
        shader->SetFloat("roughness", roughness);
        for (unsigned int i = 0; i < 6; ++i)
        {
            shader->SetMat4("view", &capture_views[i][0][0]);

            rc.BindRenderTargetTex(prefilter_map, ur::ATTACHMENT_COLOR0, static_cast<ur::TEXTURE_TARGET>(ur::TEXTURE_CUBE0 + i), mip);
            rc.SetClearFlag(ur::MASKC | ur::MASKD);
            rc.SetClearColor(0x88888888);
            rc.Clear();

            rc.RenderCube(ur::RenderContext::VertLayout::VL_POS);
        }
    }
    rc.UnbindRenderTarget();
    rc.ReleaseRenderTarget(rt);

    return prefilter_map;
}

}