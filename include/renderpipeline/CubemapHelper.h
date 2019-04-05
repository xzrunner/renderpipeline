#pragma once

#include <glm/glm.hpp>

namespace rp
{

class CubemapHelper
{
public:
    static glm::mat4 CalcCaptureProjection();

    // view matrices for capturing data onto the 6 cubemap face directions
    static void CalcCaptureViews(glm::mat4 capture_views[]);

}; // CubemapHelper

}