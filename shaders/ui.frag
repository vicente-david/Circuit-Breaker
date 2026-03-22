#version 330 core
in vec3 color;
flat in float textureFlag;
out vec4 col;

uniform sampler2D uiTexture;

void main(){

    if (textureFlag >= 1.0f){
        col = texture(uiTexture, vec2(color.x, color.y));
    } else {
        col = vec4(color, 1.0);
    }
}