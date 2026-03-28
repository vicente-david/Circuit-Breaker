#version 330 core
layout (location = 0) in vec2 pos;
layout (location = 1) in vec3 col;
layout (location = 2) in vec2 textFlag;

out vec3 color;
flat out vec2 textureFlag;

uniform mat4 projection;

void main(){

    textureFlag = textFlag; // x determines if texture, y is a value of highlight (1.0 if not highlighted)

    gl_Position = projection * vec4(pos, 0.0, 1.0);
    color = col;
}