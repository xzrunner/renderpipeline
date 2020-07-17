#include "renderpipeline/HDREquirectangularToCubemap.h"
#include "renderpipeline/CubemapHelper.h"
#include "renderpipeline/RenderMgr.h"

#include "shader/cubemap.vert"
#include "shader/equirectangular_to_cubemap.frag"

#include <unirender/Device.h>
#include <unirender/Context.h>
#include <unirender/TextureDescription.h>
#include <unirender/ShaderProgram.h>
#include <unirender/Uniform.h>
#include <unirender/Texture.h>
#include <unirender/Framebuffer.h>
#include <unirender/ClearState.h>
#include <unirender/DrawState.h>
#include <shadertrans/ShaderTrans.h>

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

ur::TexturePtr HDREquirectangularToCubemap(const ur::Device& dev, ur::Context& ctx, const ur::TexturePtr& equirectangular_map)
{
    ur::TextureDescription desc;
    desc.target = ur::TextureTarget::TextureCubeMap;
    desc.width  = 512;
    desc.height = 512;
    desc.format = ur::TextureFormat::RGB16F;
    auto cubemap = dev.CreateTexture(desc);

    std::vector<unsigned int> vs, fs;
    shadertrans::ShaderTrans::GLSL2SpirV(shadertrans::ShaderStage::VertexShader, cubemap_vs, vs);
    shadertrans::ShaderTrans::GLSL2SpirV(shadertrans::ShaderStage::PixelShader, equirectangular_to_cubemap_fs, fs);
    auto shader = dev.CreateShaderProgram(vs, fs);

    ur::DrawState ds;
    ds.program = shader;

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

        const auto target = static_cast<ur::TextureTarget>(static_cast<int>(ur::TextureTarget::TextureCubeMap0) + i);
        fbo->SetAttachment(ur::AttachmentType::Color0, target, cubemap, nullptr);

        ur::ClearState clear;
        clear.buffers = ur::ClearBuffers::ColorAndDepthBuffer;
        clear.color.FromRGBA(0x88888888);
        ctx.Clear(clear);

        ds.vertex_array = dev.GetVertexArray(ur::Device::PrimitiveType::Cube, ur::VertexLayoutType::Pos);
        ctx.Draw(ur::PrimitiveType::Triangles, ds, nullptr);
    }

    return cubemap;
}

}