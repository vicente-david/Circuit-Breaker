#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 iuv;
layout (location = 2) in vec3 col;

out vec2 uv;
out vec3 resCol;


uniform mat4 projection;

void main(){
    uv = iuv;
    resCol = col;
    gl_Position = projection * vec4(pos, 1.0);
}