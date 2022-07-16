#version 440 core

//Color input from the vertex shader
in vec4 fragmentColor;

//Color output using vertices data
out vec4 l_Color;


void main(){
  l_Color = fragmentColor;
}