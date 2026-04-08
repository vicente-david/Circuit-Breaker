#version 330 core
in vec2 uv; // where the fragment is positioned relative to the where the quad is rendered (0,1) left to right (0,1) bottom to top
in vec3 resCol; // color of resource

// because of uv, we can assume the entire square/rectangle is the speedometer

out vec4 col;

uniform float currentAngle;

float ringThickness = 0.01f;
float r = 0.9f;

// sdf of a circle
// p is test point, c is center of circle
// r is radius of circle
float circleSDF(vec2 p, vec2 c, float r){
    return length(p-c)-r;
}

void main(){
    // translate uv into [-1,1]
    vec2 UV = 2.0*uv - vec2(1.0, 1.0);  
    
    // 2*uv [0,2] -> 2*uv - (1,1) [-1,1]

    // base ring
    vec3 color = vec3(1.0); // base color of the ring

    float d = circleSDF(UV, vec2(0,0), r);
    float w = fwidth(d);
    float alpha = smoothstep(ringThickness+w, -w+ringThickness, abs(d));
    // alpha tells us if it's part of the ring or not
    col *= alpha; // zeros out anything not part of the ring

    // point on circle
    vec2 point = r*vec2(cos(currentAngle), sin(currentAngle));
    float pD = circleSDF(UV, point, ringThickness*3.0);
    float pW = fwidth(pD);
    float pAlpha = smoothstep(pW, -pW, pD);

    // pAlpha tells us if it's a part of the point on the circle or not

    color = mix(color, vec3(1.0, 1.0, 0.0), pAlpha); // mix between (black or white or gray) with yellow

    col = vec4(color, max(alpha, pAlpha)); // take the max of alpha or pAlpha
    // so if alpha is 0 but pAlpha is higher, then it's likely yellow or somethin

}