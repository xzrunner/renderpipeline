#include "renderpipeline/CreateBrdfLutTex.h"
#include "renderpipeline/RenderMgr.h"

#include "shader/brdf.vert"
#include "shader/brdf.frag"

#include <unirender/Device.h>
#include <unirender/Context.h>
#include <unirender/TextureDescription.h>
#include <unirender/ShaderProgram.h>
#include <unirender/Framebuffer.h>
#include <unirender/DrawState.h>
#include <unirender/ClearState.h>

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

ur::TexturePtr CreateBrdfLutTex(const ur::Device& dev, ur::Context& ctx)
{
    ur::TextureDescription desc;
    desc.width  = 512;
    desc.height = 512;
    desc.target = ur::TextureTarget::Texture2D;
    desc.format = ur::TextureFormat::RG16F;
    auto brdf_lut_tex = dev.CreateTexture(desc);

    ctx.SetViewport(0, 0, 512, 512);

    auto fbo = dev.CreateFramebuffer();
    ctx.SetFramebuffer(fbo);

    fbo->SetAttachment(ur::AttachmentType::Color0, ur::TextureTarget::Texture2D, brdf_lut_tex, nullptr);

    ur::ClearState clear;
    clear.buffers = ur::ClearBuffers::ColorAndDepthBuffer;
    clear.color.FromRGBA(0x88888888);
    ctx.Clear(clear);

    ur::DrawState ds;
    ds.program = dev.CreateShaderProgram(brdf_vs, brdf_fs);
    ds.vertex_array = dev.GetVertexArray(ur::Device::PrimitiveType::Quad, ur::VertexLayoutType::PosTex);
    ctx.Draw(ur::PrimitiveType::TriangleStrip, ds, nullptr);

    return brdf_lut_tex;
}

}
