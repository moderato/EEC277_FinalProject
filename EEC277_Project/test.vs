#version 410 core
layout (location = 0) in vec2 position;

uniform mat4 projection;
uniform mat4 view;


void main()
{
//    gl_Position =  projection * view * vec4(position, 0.0f, 1.0f);
    gl_Position =  vec4(position, 0.0f, 1.0f);
}
