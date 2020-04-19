#include "renderpipeline/CreateBrdfLutTex.h"
#include "renderpipeline/RenderMgr.h"

#include "shader/brdf.vert"
#include "shader/brdf.frag"

#include <unirender2/Device.h>
#include <unirender2/Context.h>
#include <unirender2/TextureDescription.h>
#include <unirender2/ShaderProgram.h>
#include <unirender2/Framebuffer.h>
#include <unirender2/DrawState.h>
#include <unirender2/ClearState.h>

namespace rp
{

//unsigned int CreateBrdfLutTex()
//{
//    unsigned brdf_lut_tex = 0;
//
//    auto& rc = ur::Blackboard::Instance()->GetRenderContext();
//    ur::Sandbox sb(rc);
//
//    brdf_lut_tex = rc.CreateTexture(nullptr, 512, 512, ur::TEXTURE_RG16F);
//
//    RenderMgr::Instance()->SetRenderer(RenderType::EXTERN);
//
//    std::vector<std::string> textures;
//    CU_VEC<ur::VertexAttrib> va_list;
//    auto shader = std::make_shared<ur::Shader>(&rc, brdf_vs, brdf_fs, textures, va_list, true);
//    shader->Use();
//
//    rc.SetViewport(0, 0, 512, 512);
//
//    auto rt = rc.CreateRenderTarget(0);
//    rc.BindRenderTarget(rt);
//
//    rc.BindRenderTargetTex(brdf_lut_tex);
//    rc.SetClearFlag(ur::MASKC | ur::MASKD);
//    rc.SetClearColor(0x88888888);
//    rc.Clear();
//
//    rc.RenderQuad(ur::RenderContext::VertLayout::VL_POS_TEX);
//
//    rc.UnbindRenderTarget();
//    rc.ReleaseRenderTarget(rt);
//
//    return brdf_lut_tex;
//}

ur2::TexturePtr CreateBrdfLutTex(const ur2::Device& dev, ur2::Context& ctx)
{
    ur2::TextureDescription desc;
    desc.width  = 512;
    desc.height = 512;
    desc.target = ur2::TextureTarget::Texture2D;
    desc.format = ur2::TextureFormat::RG16F;
    auto brdf_lut_tex = dev.CreateTexture(desc);

    ctx.SetViewport(0, 0, 512, 512);

    auto fbo = dev.CreateFramebuffer();
    ctx.SetFramebuffer(fbo);

    fbo->SetAttachment(ur2::AttachmentType::Color0, ur2::TextureTarget::Texture2D, brdf_lut_tex, nullptr);

    ur2::ClearState clear;
    clear.buffers = ur2::ClearBuffers::ColorAndDepthBuffer;
    clear.color.FromRGBA(0x88888888);
    ctx.Clear(clear);

    ur2::DrawState draw;
    draw.program = dev.CreateShaderProgram(brdf_vs, brdf_fs);

    ctx.DrawQuad(ur2::Context::VertexLayout::PosTex, draw);

    return brdf_lut_tex;
}

}
