#include "GL/Window.hpp"
#include "GL/Objects/Shaders.hpp"

SSS_GL_BEGIN;

static void _planeShadersData(std::string& vertex, std::string& fragment)
{
    vertex = R"(
#version 330 core
layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec2 a_UV;

uniform mat4 u_Models[gl_MaxTextureImageUnits];
uniform mat4 u_VPs[gl_MaxTextureImageUnits];
uniform float u_TextureOffsets[gl_MaxTextureImageUnits];
uniform float u_Alphas[gl_MaxTextureImageUnits];

out vec3 UVW;
out float Alpha;
flat out int instanceID;

void main()
{
    gl_Position = u_VPs[gl_InstanceID] * u_Models[gl_InstanceID] * vec4(a_Pos, 1);
    UVW = vec3(a_UV, u_TextureOffsets[gl_InstanceID]);
    Alpha = u_Alphas[gl_InstanceID];
    instanceID = gl_InstanceID;
}
)";

    fragment = R"(
#version 330 core
out vec4 FragColor;

in vec3 UVW;
in float Alpha;
flat in int instanceID;

uniform sampler2DArray u_Textures[gl_MaxTextureImageUnits];

void main()
{
    FragColor = texture(u_Textures[instanceID], UVW);
    FragColor.w *= Alpha;
}
)";
}

static void _lineShadersData(std::string& vertex, std::string& fragment)
{
    vertex = R"(
#version 440 core

//Coordinates and colors data input
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec4 model_colors;

//Color output for the fragment shader
out vec4 fragmentColor;

// Projection matrix
uniform mat4 u_MVP;


void main(){
    //Transform the vertex position using the ortho projection matrix
    gl_Position =  u_MVP * vec4(vertexPosition_modelspace, 1);

    //Color output for the fragment shader
    fragmentColor = model_colors;
}
)";

    fragment = R"(
#version 440 core

//Color input from the vertex shader
in vec4 fragmentColor;

//Color output using vertices data
out vec4 l_Color;


void main(){
  l_Color = fragmentColor;
}
)";
}

void Window::_loadPresetShaders() try
{
    std::string vertex_data, fragment_data;
    
    // Plane shader
    {
        uint32_t const id = static_cast<uint32_t>(Shaders::Preset::Plane);
        // Retrieve shader pointer
        auto& shader = _main._preset_shaders[id];
        // Allocate new shader
        shader.reset(new Shaders());
        // Retrieve shader data
        _planeShadersData(vertex_data, fragment_data);
        // Load shader
        shader->loadFromStrings(vertex_data, fragment_data);
    }

    // Line shader
    {
        uint32_t const id = static_cast<uint32_t>(Shaders::Preset::Line);
        // Retrieve shader pointer
        auto& shader = _main._preset_shaders[id];
        // Allocate new shader
        shader.reset(new Shaders());
        // Retrieve shader data
        _lineShadersData(vertex_data, fragment_data);
        // Load shader
        shader->loadFromStrings(vertex_data, fragment_data);
    }

}
CATCH_AND_RETHROW_FUNC_EXC;

SSS_GL_END;