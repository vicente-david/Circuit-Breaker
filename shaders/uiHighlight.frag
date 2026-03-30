#version 330 core
in vec3 color;
in vec3 hLight;
flat in float textureFlag;
out vec4 col;

uniform sampler2D uiTexture;

void main(){

    if (textureFlag >= 1.0f){
        vec4 colBefore = texture(uiTexture, vec2(color.x, color.y));
        col = vec4(mix(colBefore.xyz, hLight, 0.2), 1.0);
    } else {
        col = vec4(color+hLight, 1.0);
    }
}