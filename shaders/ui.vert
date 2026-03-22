#version 330 core
layout (location = 0) in vec2 pos;
layout (location = 1) in vec3 col;
layout (location = 2) in int floatFlag;

out vec3 color;
flat out int textureFlag;

uniform mat4 projection;

void main(){

    textureFlag = floatFlag;

    gl_Position = projection * vec4(pos, 0.0, 1.0);
    color = col;
}