#include "renderpipeline/SeparableSSS.h"

namespace rp
{

void SeparableSSS::CalculateKernel(std::vector<sm::vec4>& kernel, int samples_num,
                                   const sm::vec3& strength, const sm::vec3& falloff)
{
    const float RANGE = samples_num > 20 ? 3.0f : 2.0f;
    const float EXPONENT = 2.0f;

    kernel.resize(samples_num);

    // Calculate the offsets:
    float step = 2.0f * RANGE / (samples_num - 1);
    for (int i = 0; i < samples_num; i++) {
        float o = -RANGE + float(i) * step;
        float sign = o < 0.0f? -1.0f : 1.0f;
        kernel[i].w = RANGE * sign * abs(pow(o, EXPONENT)) / pow(RANGE, EXPONENT);
    }

    // Calculate the weights:
    for (int i = 0; i < samples_num; i++) {
        float w0 = i > 0? abs(kernel[i].w - kernel[i - 1].w) : 0.0f;
        float w1 = i < samples_num - 1 ? abs(kernel[i].w - kernel[i + 1].w) : 0.0f;
        float area = (w0 + w1) / 2.0f;
        auto t = Profile(kernel[i].w, falloff) * area;
        kernel[i].x = t.x;
        kernel[i].y = t.y;
        kernel[i].z = t.z;
    }

    // We want the offset 0.0 to come first:
    auto t = kernel[samples_num / 2];
    for (int i = samples_num / 2; i > 0; i--) {
        kernel[i] = kernel[i - 1];
    }
    kernel[0] = t;

    // Calculate the sum of the weights, we will need to normalize them below:
    sm::vec3 sum;
    for (auto& k : kernel) {
        sum += sm::vec3(k.x, k.y, k.z);
    }

    // Normalize the weights:
    for (auto& k : kernel) {
        k.x /= sum.x;
        k.y /= sum.y;
        k.z /= sum.z;
    }

    // Tweak them using the desired strength. The first one is:
    //     lerp(1.0, kernel[0].rgb, strength)
    kernel[0].x = (1.0f - strength.x) * 1.0f + strength.x * kernel[0].x;
    kernel[0].y = (1.0f - strength.y) * 1.0f + strength.y * kernel[0].y;
    kernel[0].z = (1.0f - strength.z) * 1.0f + strength.z * kernel[0].z;

    // The others:
    //     lerp(0.0, kernel[0].rgb, strength)
    for (int i = 1; i < samples_num; i++) {
        kernel[i].x *= strength.x;
        kernel[i].y *= strength.y;
        kernel[i].z *= strength.z;
    }
}

sm::vec3 SeparableSSS::Profile(float r, const sm::vec3& falloff)
{
    /**
     * We used the red channel of the original skin profile defined in
     * [d'Eon07] for all three channels. We noticed it can be used for green
     * and blue channels (scaled using the falloff parameter) without
     * introducing noticeable differences and allowing for total control over
     * the profile. For example, it allows to create blue SSS gradients, which
     * could be useful in case of rendering blue creatures.
     */
    return  // Gaussian(0.0064f, r) * 0.233f + /* We consider this one to be directly bounced light, accounted by the strength parameter (see @STRENGTH) */
               Gaussian(0.0484f, r, falloff) * 0.100f +
               Gaussian( 0.187f, r, falloff) * 0.118f +
               Gaussian( 0.567f, r, falloff) * 0.113f +
               Gaussian(  1.99f, r, falloff) * 0.358f +
               Gaussian(  7.41f, r, falloff) * 0.078f;
}

sm::vec3 SeparableSSS::Gaussian(float variance, float r, const sm::vec3& falloff)
{
    /**
     * We use a falloff to modulate the shape of the profile. Big falloffs
     * spreads the shape making it wider, while small falloffs make it
     * narrower.
     */
    sm::vec3 g;
    for (int i = 0; i < 3; i++) {
        float rr = r / (0.001f + falloff[i]);
        g[i] = exp((-(rr * rr)) / (2.0f * variance)) / (2.0f * 3.14f * variance);
    }
    return g;
}

}