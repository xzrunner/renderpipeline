#include "renderpipeline/CreateIrradianceCubemap.h"
#include "renderpipeline/CubemapHelper.h"
#include "renderpipeline/RenderMgr.h"

#include "shader/cubemap.vert"
#include "shader/irradiance_convolution.frag"

#include <unirender/Device.h>
#include <unirender/Context.h>
#include <unirender/TextureDescription.h>
#include <unirender/ShaderProgram.h>
#include <unirender/Uniform.h>
#include <unirender/Texture.h>
#include <unirender/Framebuffer.h>
#include <unirender/ClearState.h>
#include <unirender/DrawState.h>

namespace rp
{

//ur::TexturePtr CreateIrradianceCubemap(const ur::Device& dev, ur::Context& ctx, const ur::TexturePtr& cubemap)
//{
//    unsigned int irr_cubemap = 0;
//
//    auto& rc = ur::Blackboard::Instance()->GetRenderContext();
//    ur::Sandbox sb(rc);
//
//    irr_cubemap = rc.CreateTextureCube(32, 32);
//
//    //// todo
//    //glEnable(GL_DEPTH_TEST);
//    //glDepthFunc(GL_LEQUAL); // set depth function to less than AND equal for skybox depth trick.
//
//    RenderMgr::Instance()->SetRenderer(RenderType::EXTERN);
//
//    std::vector<std::string> textures;
//    CU_VEC<ur::VertexAttrib> va_list;
//    auto shader = std::make_shared<ur::Shader>(&rc, cubemap_vs, irradiance_convolution_fs, textures, va_list, true);
//
//    shader->Use();
//
//    shader->SetInt("environmentMap", 0);
//
//    glm::mat4 capture_proj = CubemapHelper::CalcCaptureProjection();
//    shader->SetMat4("projection", &capture_proj[0][0]);
//
//    rc.BindTexture(cubemap, 0);
//    rc.SetViewport(0, 0, 32, 32);
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
//        rc.BindRenderTargetTex(irr_cubemap, ur::ATTACHMENT_COLOR0, static_cast<ur::TEXTURE_TARGET>(ur::TEXTURE_CUBE0 + i));
//        rc.SetClearFlag(ur::MASKC | ur::MASKD);
//        rc.SetClearColor(0x88888888);
//        rc.Clear();
//
//        rc.RenderCube(ur::RenderContext::VertLayout::VL_POS);
//    }
//    rc.UnbindRenderTarget();
//    rc.ReleaseRenderTarget(rt);
//
//    return irr_cubemap;
//}

ur::TexturePtr CreateIrradianceCubemap(const ur::Device& dev, ur::Context& ctx,
                                        const ur::TexturePtr& cubemap)
{
    ur::TextureDescription desc;
    desc.target = ur::TextureTarget::TextureCubeMap;
    desc.width  = 32;
    desc.height = 32;
    desc.format = ur::TextureFormat::RGB16F;
    auto irr_cubemap = dev.CreateTexture(desc);

    auto shader = dev.CreateShaderProgram(cubemap_vs, irradiance_convolution_fs);

    ur::DrawState ds;
    ds.program = shader;

    auto u_env_map = shader->QueryUniform("environmentMap");
    assert(u_env_map);
    int i = 0;
    u_env_map->SetValue(&i);

    glm::mat4 capture_proj = CubemapHelper::CalcCaptureProjection();
    auto u_proj = shader->QueryUniform("projection");
    assert(u_proj);
    u_proj->SetValue(&capture_proj[0][0], 4 * 4);

    cubemap->Bind();

    ctx.SetViewport(0, 0, 32, 32);

    glm::mat4 capture_views[6];
    CubemapHelper::CalcCaptureViews(capture_views);

    auto fbo = dev.CreateFramebuffer();
    ctx.SetFramebuffer(fbo);

    for (int i = 0; i < 6; ++i)
    {
        auto u_view = shader->QueryUniform("view");
        assert(u_view);
        u_view->SetValue(&capture_views[i][0][0], 4 * 4);

        const auto target = static_cast<ur::TextureTarget>(static_cast<int>(ur::TextureTarget::TextureCubeMap0) + i);
        fbo->SetAttachment(ur::AttachmentType::Color0, target, irr_cubemap, nullptr);

        ur::ClearState clear;
        clear.buffers = ur::ClearBuffers::ColorAndDepthBuffer;
        clear.color.FromRGBA(0x88888888);
        ctx.Clear(clear);

        ds.vertex_array = dev.GetVertexArray(ur::Device::PrimitiveType::Cube, ur::VertexLayoutType::Pos);
        ctx.Draw(ur::PrimitiveType::Triangles, ds, nullptr);
    }

    return irr_cubemap;
}

}