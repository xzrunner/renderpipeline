#include "renderpipeline/HDREquirectangularToCubemap.h"
#include "renderpipeline/CubemapHelper.h"

#include "shader/cubemap.vert"
#include "shader/equirectangular_to_cubemap.frag"

#include <unirender/Blackboard.h>
#include <unirender/RenderContext.h>
#include <unirender/Shader.h>

namespace rp
{

unsigned int HDREquirectangularToCubemap(unsigned int equirectangular_map)
{
    unsigned int cubemap = 0;

    auto& rc = ur::Blackboard::Instance()->GetRenderContext();
    cubemap = rc.CreateTextureCube(512, 512);

    //// todo
    //glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LEQUAL); // set depth function to less than AND equal for skybox depth trick.

    std::vector<std::string> textures;
    CU_VEC<ur::VertexAttrib> va_list;
    auto shader = std::make_shared<ur::Shader>(&rc, cubemap_vs, equirectangular_to_cubemap_fs, textures, va_list, true);
    shader->Use();

    shader->SetInt("equirectangularMap", 0);

    glm::mat4 capture_proj = CubemapHelper::CalcCaptureProjection();
    shader->SetMat4("projection", &capture_proj[0][0]);

    rc.BindTexture(equirectangular_map, 0);
    rc.SetViewport(0, 0, 512, 512);

    glm::mat4 capture_views[6];
    CubemapHelper::CalcCaptureViews(capture_views);

    auto rt = rc.CreateRenderTarget(0);
    rc.BindRenderTarget(rt);
    for (int i = 0; i < 6; ++i)
    {
        shader->SetMat4("view", &capture_views[i][0][0]);

        rc.BindRenderTargetTex(cubemap, ur::ATTACHMENT_COLOR0, static_cast<ur::TEXTURE_TARGET>(ur::TEXTURE_CUBE0 + i));
        rc.SetClearFlag(ur::MASKC | ur::MASKD);
        rc.SetClearColor(0x88888888);
        rc.Clear();

        rc.RenderCube();
    }
    rc.UnbindRenderTarget();
    rc.ReleaseRenderTarget(rt);

    return cubemap;
}

}