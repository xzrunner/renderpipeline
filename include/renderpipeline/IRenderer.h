#pragma once

#include <memory>
#include <vector>

namespace ur { class Context; class ShaderProgram; }

namespace rp
{

class IRenderer
{
public:
	virtual ~IRenderer() {}
	virtual void Flush(ur::Context& ctx) = 0;

    auto& GetAllShaders() const { return m_shaders; }

protected:
    std::vector<std::shared_ptr<ur::ShaderProgram>> m_shaders;

}; // IRenderer

}