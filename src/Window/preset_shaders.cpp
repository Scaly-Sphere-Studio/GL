#include "SSS/GL/Window.hpp"
#include "SSS/GL/Objects/Shaders.hpp"

SSS_GL_BEGIN;

static void _planeShadersData(std::string& vertex, std::string& fragment)
{
    vertex = 
"#version 330 core\n"
"layout(location = 0) in vec3 a_Pos;\n"
"layout(location = 1) in vec2 a_UV;\n"

"uniform mat4 u_Models[gl_MaxTextureImageUnits];\n"
"uniform mat4 u_VPs[gl_MaxTextureImageUnits];\n"

"out vec2 UV;\n"
"flat out int instanceID;\n"

"void main()\n"
"{\n"
"    gl_Position = u_VPs[gl_InstanceID] * u_Models[gl_InstanceID] * vec4(a_Pos, 1);\n"
"    UV = a_UV;\n"
"    instanceID = gl_InstanceID;\n"
"}";

    fragment = 
"#version 330 core\n"
"out vec4 FragColor;\n"

"in vec2 UV;\n"
"flat in int instanceID;\n"

"uniform sampler2D u_Textures[gl_MaxTextureImageUnits];\n"

"void main()\n"
"{\n"
"    FragColor = texture(u_Textures[instanceID], UV);\n"
"}";

}

void Window::_loadPresetShaders() try
{
    Context const context(_window.get());

    std::string vertex_data, fragment_data;
    
    // Plane shader
    {
        uint32_t const id = static_cast<uint32_t>(Shaders::Preset::Plane);
        // Retrieve shader pointer
        auto& shader = _shaders[id];
        // Allocate new shader
        shader.reset(new Shaders(weak_from_this(), id));
        // Retrieve shader data
        _planeShadersData(vertex_data, fragment_data);
        // Load shader
        shader->loadFromStrings(vertex_data, fragment_data);
    }

}
CATCH_AND_RETHROW_METHOD_EXC;

SSS_GL_END;