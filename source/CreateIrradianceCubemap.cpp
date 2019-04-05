#include "renderpipeline/CreateIrradianceCubemap.h"
#include "renderpipeline/CubemapHelper.h"

#include "shader/cubemap.vert"
#include "shader/irradiance_convolution.frag"

#include <unirender/Blackboard.h>
#include <unirender/RenderContext.h>
#include <unirender/Shader.h>

namespace rp
{

unsigned int CreateIrradianceCubemap(unsigned int cubemap)
{
    unsigned int irr_cubemap = 0;

    auto& rc = ur::Blackboard::Instance()->GetRenderContext();
    irr_cubemap = rc.CreateTextureCube(32, 32);

    //// todo
    //glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LEQUAL); // set depth function to less than AND equal for skybox depth trick.

    std::vector<std::string> textures;
    CU_VEC<ur::VertexAttrib> va_list;
    auto shader = std::make_shared<ur::Shader>(&rc, cubemap_vs, irradiance_convolution_fs, textures, va_list, true);
    shader->Use();

    shader->SetInt("environmentMap", 0);

    glm::mat4 capture_proj = CubemapHelper::CalcCaptureProjection();
    shader->SetMat4("projection", &capture_proj[0][0]);

    rc.BindTexture(cubemap, 0);
    rc.SetViewport(0, 0, 32, 32);

    glm::mat4 capture_views[6];
    CubemapHelper::CalcCaptureViews(capture_views);

    auto rt = rc.CreateRenderTarget(0);
    rc.BindRenderTarget(rt);
    for (int i = 0; i < 6; ++i)
    {
        shader->SetMat4("view", &capture_views[i][0][0]);

        rc.BindRenderTargetTex(irr_cubemap, ur::ATTACHMENT_COLOR0, ur::TEXTURE_CUBE0 + i);
        rc.SetClearFlag(ur::MASKC | ur::MASKD);
        rc.SetClearColor(0x88888888);
        rc.Clear();

        rc.RenderCube();
    }
    rc.UnbindRenderTarget();
    rc.ReleaseRenderTarget(rt);

    return irr_cubemap;
}

}