#include "renderpipeline/HDREquirectangularToCubemap.h"
#include "renderpipeline/CubemapHelper.h"
#include "renderpipeline/RenderMgr.h"

#include "shader/cubemap.vert"
#include "shader/equirectangular_to_cubemap.frag"

#include <unirender2/Device.h>
#include <unirender2/Context.h>
#include <unirender2/TextureDescription.h>
#include <unirender2/ShaderProgram.h>
#include <unirender2/Uniform.h>
#include <unirender2/Texture.h>
#include <unirender2/Framebuffer.h>
#include <unirender2/ClearState.h>
#include <unirender2/DrawState.h>

namespace rp
{

//unsigned int HDREquirectangularToCubemap(unsigned int equirectangular_map)
//{
//    unsigned int cubemap = 0;
//
//    auto& rc = ur::Blackboard::Instance()->GetRenderContext();
//    ur::Sandbox sb(rc);
//
//    cubemap = rc.CreateTextureCube(512, 512);
//
//    //// todo
//    //glEnable(GL_DEPTH_TEST);
//    //glDepthFunc(GL_LEQUAL); // set depth function to less than AND equal for skybox depth trick.
//
//    RenderMgr::Instance()->SetRenderer(RenderType::EXTERN);
//
//    std::vector<std::string> textures;
//    CU_VEC<ur::VertexAttrib> va_list;
//    auto shader = std::make_shared<ur::Shader>(&rc, cubemap_vs, equirectangular_to_cubemap_fs, textures, va_list, true);
//    shader->Use();
//
//    shader->SetInt("equirectangularMap", 0);
//
//    glm::mat4 capture_proj = CubemapHelper::CalcCaptureProjection();
//    shader->SetMat4("projection", &capture_proj[0][0]);
//
//    rc.BindTexture(equirectangular_map, 0);
//    rc.SetViewport(0, 0, 512, 512);
//
//    glm::mat4 capture_views[6];
//    CubemapHelper::CalcCaptureViews(capture_views);
//
//    auto rt = rc.CreateRenderTarget(0);
//    rc.BindRenderTarget(rt);
//    for (int i = 0; i < 6; ++i)
//    {
//        shader->SetMat4("view", &capture_views[i][0][0]);
//
//        rc.BindRenderTargetTex(cubemap, ur::ATTACHMENT_COLOR0, static_cast<ur::TEXTURE_TARGET>(ur::TEXTURE_CUBE0 + i));
//        rc.SetClearFlag(ur::MASKC | ur::MASKD);
//        rc.SetClearColor(0x88888888);
//        rc.Clear();
//
//        rc.RenderCube(ur::RenderContext::VL_POS);
//    }
//    rc.UnbindRenderTarget();
//    rc.ReleaseRenderTarget(rt);
//
//    return cubemap;
//}

ur2::TexturePtr HDREquirectangularToCubemap(const ur2::Device& dev, ur2::Context& ctx, const ur2::TexturePtr& equirectangular_map)
{
    ur2::TextureDescription desc;
    desc.target = ur2::TextureTarget::TextureCubeMap;
    desc.width  = 512;
    desc.height = 512;
    desc.format = ur2::TextureFormat::RGB16F;
    auto cubemap = dev.CreateTexture(desc);

    auto shader = dev.CreateShaderProgram(cubemap_vs, equirectangular_to_cubemap_fs);

    ur2::DrawState draw;
    draw.program = shader;

    auto u_eq_map = shader->QueryUniform("equirectangularMap");
    assert(u_eq_map);
    int i = 0;
    u_eq_map->SetValue(&i);

    glm::mat4 capture_proj = CubemapHelper::CalcCaptureProjection();
    auto u_proj = shader->QueryUniform("projection");
    assert(u_proj);
    u_proj->SetValue(&capture_proj[0][0], 4 * 4);

    cubemap->Bind();

    glm::mat4 capture_views[6];
    CubemapHelper::CalcCaptureViews(capture_views);

    auto fbo = dev.CreateFramebuffer();
    ctx.SetFramebuffer(fbo);
    for (int i = 0; i < 6; ++i)
    {
        auto u_view = shader->QueryUniform("view");
        assert(u_view);
        u_view->SetValue(&capture_views[i][0][0], 4 * 4);

        const auto target = static_cast<ur2::TextureTarget>(static_cast<int>(ur2::TextureTarget::TextureCubeMap0) + i);
        fbo->SetAttachment(ur2::AttachmentType::Color0, target, cubemap, nullptr);

        ur2::ClearState clear;
        clear.buffers = ur2::ClearBuffers::ColorAndDepthBuffer;
        clear.color.FromRGBA(0x88888888);
        ctx.Clear(clear);

        ctx.DrawCube(ur2::Context::VertexLayout::Pos, draw);
    }

    return cubemap;
}

}