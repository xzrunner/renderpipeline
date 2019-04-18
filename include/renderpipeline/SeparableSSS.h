#pragma once

#include <SM_Vector.h>

#include <vector>

namespace rp
{

class SeparableSSS
{
public:
    static void CalculateKernel(std::vector<sm::vec4>& kernel, int samples_num,
        const sm::vec3& strength, const sm::vec3& falloff);

private:
    static sm::vec3 Profile(float r, const sm::vec3& falloff);
    static sm::vec3 Gaussian(float variance, float r, const sm::vec3& falloff);

}; // SeparableSSS

}