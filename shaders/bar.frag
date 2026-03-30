#version 330 core
in vec2 uv; // where the fragment is positioned relative to the where the quad is rendered (0,1) left to right (0,1) bottom to top
in vec3 resCol; // color of resource

// because of uv, we can assume the entire rectangle is the bar

out vec4 col;

uniform float resource1;
uniform float maxResource;


vec2 delta = vec2(2*fwidth(uv.x), 2*fwidth(uv.y)); // derivative of pixel


// checks if a fragment is inside the border of the rectangle
bool insideBorder(vec2 p){
    
    if (p.x < delta.x || p.x > 1.0 - delta.x) return false; 
    
    if (p.y > 1.0 - delta.y || p.y < delta.y) return false;
    
    return true;

}

// checks if a fragment is part of the health part of the bar or not
bool insideHealth(vec2 p){
    float ratio = resource1/maxResource; // how much of the inside of the border should be filled
    
    // map the ratio to delta distance
    // left edge is delta
    // right edge is 1-delta
    // and we have a ratio, we'll just linearly interpolate here
    // start at left edge (delta) + whatever the ratio is in between right edge (1-delta)
    float fillHealth = (delta.x) + (1-delta.x)*ratio;

    return p.x <= fillHealth;
}


void main(){
    
    if (!insideBorder(uv)){
        col = vec4(vec3(0.0f), 1.0f);
        return;
    }

    if (!insideHealth(uv)){
        col = vec4(vec3(0.5f), 1.0f);
    } else {
        col = vec4(resCol, 1.0f);
    }


}