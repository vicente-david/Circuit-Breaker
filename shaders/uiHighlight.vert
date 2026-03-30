#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 col;
layout (location = 2) in float textFlag;
layout (location = 3) in vec3 highlightCol;

out vec3 color;
out vec3 hLight;
flat out float textureFlag;

uniform mat4 projection;

void main(){
    hLight = highlightCol;
    textureFlag = textFlag; // x determines if texture, y is a value of highlight (1.0 if not highlighted)

    gl_Position = projection * vec4(pos, 1.0);
    color = col;
}