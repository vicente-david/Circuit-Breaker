#version 330 core
in vec3 color;
flat in float textureFlag;
out vec4 col;

uniform sampler2D uiTexture;

void main(){

    // will encode the highlight in the texture flag

    // if texture flag is 1.0, then no highlight
    if (textureFlag >= 1.0f){
        vec4 textureColor = texture(uiTexture, vec2(color.x, color.y));
        col = vec4(textureColor.xyz * textureFlag, 1.0);
    } else {
    // don't use texture but color could still be highlighted
    // assume solid color flag is negative 
    // -1.0 means no highlight
        col = vec4(-1.0*textureFlag*color, 1.0);
    }
}