#version 330 core
in vec2 uv; // where the fragment is positioned relative to the where the quad is rendered (0,1) left to right (0,1) bottom to top
in vec3 resCol; // color of resource

// because of uv, we can assume the entire rectangle is the bar

out vec4 color;

// box sdf

// b means height and width (half width/half height)
float boxSDF( in vec2 p, in vec2 b )
{
    vec2 d = abs(p)-b;
    return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
}



// uniforms
uniform float currentVol; // current vol
uniform float maxVol; // maxVol
uniform float isHighlighted; // if highlighted then change border color

// globals
vec3 borderColor1 = vec3(46.0/255.0, 139.0/255.0, 77.0/255.0);
vec3 borderColor2 = vec3(1.0); // highlighted color
vec3 selectedColor = vec3(1.0, 252.0/255.0, 89.0/255.0); // where slider filled
vec3 unfilled = vec3(13.0/255.0, 20.0/255.0, 18.0/255.0); // unfilled bar color

void main(){
    
    vec2 UV = 2.0*uv - vec2(1.0, 1.0);
    
    vec3 col = (isHighlighted >= 1.0) ? mix(borderColor1, borderColor2, 0.2) : borderColor1;

    float volRatio = currentVol/maxVol;

    float d = boxSDF(UV, vec2(0.9, 0.9));
    float w = fwidth(d);
    float alpha = smoothstep(-w, w, d); 


    float normX = UV.x*0.5 + 0.5;
    float xChange = fwidth(normX);
    float cutoff = smoothstep(volRatio - xChange, volRatio + xChange, normX);
    
    vec3 insideColor = mix(selectedColor, unfilled, cutoff);

    col = mix(col, insideColor, 1.0-alpha);

    // Output to screen
    color = vec4(col, 1.0);
}