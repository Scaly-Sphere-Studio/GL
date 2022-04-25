#include "SSS/GL/Window.hpp"

__SSS_GL_BEGIN;

static void _planeShadersData(std::string& vertex, std::string& fragment, uint32_t size)
{
    std::string const max_size = std::to_string(size);
    vertex = "\
#version 330 core\n\
layout(location = 0) in vec3 a_Pos;\n\
layout(location = 1) in vec2 a_UV;\n\
\n\
uniform mat4 u_Models[" + max_size + "];\n\
uniform mat4 u_VPs[" + max_size + "];\n\
\n\
out vec2 UV;\n\
flat out int instanceID;\n\
\n\
void main()\n\
{\n\
    gl_Position = u_VPs[gl_InstanceID] * u_Models[gl_InstanceID] * vec4(a_Pos, 1);\n\
    UV = a_UV;\n\
    instanceID = gl_InstanceID;\n\
}";
    fragment = "\
#version 330 core\n\
out vec4 FragColor;\n\
\n\
in vec2 UV;\n\
flat in int instanceID;\n\
\n\
uniform sampler2D u_Textures[" + max_size + "];\n\
\n\
void main()\n\
{\n\
    FragColor = texture(u_Textures[instanceID], UV);\n\
}";
}

void Window::loadPreSetShaders() try
{
    Context const context(_window.get());

    std::string vertex_data, fragment_data;
    
    // Plane shader
    {
        // Retrieve shader pointer
        Shaders::Ptr& shader =
            _objects.shaders[static_cast<uint32_t>(Shaders::Preset::Plane)];
        // Allocate new shader
        shader.reset(new Shaders(weak_from_this()));
        // Retrieve shader data
        _planeShadersData(vertex_data, fragment_data, Plane::Renderer::glsl_max_array_size);
        // Load shader
        shader->loadFromStrings(vertex_data, fragment_data);
    }

}
__CATCH_AND_RETHROW_METHOD_EXC;

__SSS_GL_END;