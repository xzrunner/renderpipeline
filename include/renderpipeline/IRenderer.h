#pragma once

#include <memory>
#include <vector>

namespace pt0 { class Shader; }

namespace rp
{

class IRenderer
{
public:
	virtual ~IRenderer() {}
	virtual void Flush() = 0;

    auto& GetAllShaders() const { return m_shaders; }

protected:
    std::vector<std::shared_ptr<pt0::Shader>> m_shaders;

}; // IRenderer

}