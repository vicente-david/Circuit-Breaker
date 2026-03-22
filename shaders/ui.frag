#version 330 core
in vec3 color;
flat in int textureFlag;
out vec4 col;

uniform sampler2D uiTexture;

void main(){

    if (textureFlag >= 1){
        col = texture(uiTexture, color.xy);
    } else {
        col = vec4(color, 1.0);
    }
}