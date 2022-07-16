#version 440 core

//Coordinates and colors data input
layout(location = 0) in vec2 vertexPosition_modelspace;
layout(location = 1) in vec4 model_colors;

//Color output for the fragment shader
out vec4 fragmentColor;

// Projection matrix
uniform mat4 u_MVP;


void main(){
    //Transform the vertex position using the ortho projection matrix
    gl_Position =  u_MVP * vec4(vertexPosition_modelspace, 0, 1);

    //Color output for the fragment shader
    fragmentColor = model_colors;
}