#version 330 core
in vec3 color;
flat in vec2 textureFlag;
out vec4 col;

uniform sampler2D uiTexture;

void main(){

    if (textureFlag.x >= 1.0f){
        vec4 colBefore = texture(uiTexture, vec2(color.x, color.y));
        col = vec4(colBefore.rgb*textureFlag.y, colBefore.w); // textureFlag.y is the amount to multiply to highlight
    } else {
        col = vec4(color, 1.0);
    }
}