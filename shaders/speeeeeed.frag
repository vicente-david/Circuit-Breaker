#version 330 core
in vec2 uv; // where the fragment is positioned relative to the where the quad is rendered (0,1) left to right (0,1) bottom to top
in vec3 resCol; // color of resource

// because of uv, we can assume the entire square/rectangle is the speedometer

out vec4 col;

uniform float currentAngle;
uniform float prevAngle;

uniform float isBoosting;
uniform float timeBoosting;

float ringThickness = 0.025f;
float r = 0.9f;

vec3 ringColor = vec3(1.0f);
vec3 bgColor = vec3(19.0/255.0, 38.0/255.0, 30.0/255.0);
vec3 pointColor = vec3(1.0, 1.0, 0.0);

const float PI = 3.1415926535897932384626433832795;

// wavy sdf of a circle
float wavyCircleSDF(vec2 p, vec2 c, float r){
    // translate point wrt circle center
    vec2 P = p-c;
    // define circle radius
    float radius = r;
    
    // nab the angle (because the radius gets scaled based on angle)
    float uvAngle = atan(P.y, P.x); 
    
    // amplitude of the ripples
    float amplitude = sin(timeBoosting)*0.0125+0.025;

    // ripple rotation speed
    float rotSpeed = 1.0f*timeBoosting;
    
    // wave function (adds this much to the radius depending on the direction)
    // it will have 8 ripples
    // note the frequency mathematically can't be any real number, it must be an integer
    float wave = sin(8.0*(uvAngle-rotSpeed))*amplitude;
    
    radius += wave;
  
    return length(P)-radius;
}


// sdf of a circle
// p is test point, c is center of circle
// r is radius of circle
float circleSDF(vec2 p, vec2 c, float r){
    return length(p-c)-r;
}

float ringSDF(vec2 UV){
    float d = circleSDF(UV, vec2(0,0), r);

    if (isBoosting >= 1.0f){
        d = wavyCircleSDF(UV, vec2(0,0), r);
    }

    float w = fwidth(d);
    float alpha = smoothstep(ringThickness+w, -w+ringThickness, abs(d));
    // alpha tells us if it's part of the ring or not
    return alpha;

}

float pointSDF(vec2 UV){
     // point on circle
    vec2 point = r*vec2(cos(currentAngle), sin(currentAngle));
    float pD = circleSDF(UV, point, ringThickness*3.0);
    float pW = fwidth(pD);
    float pAlpha = smoothstep(pW, -pW, pD);

    return pAlpha;
}

vec3 tailCheck(vec2 UV, vec3 color, float ringAlpha){
    float uvAngle = atan(UV.y, UV.x);
    float start = prevAngle; // [0,2pi]
    float end = currentAngle; // [0, 2pi]

    // so if uv angle is in between start and end we're good

    if (uvAngle < 0.0f){
        uvAngle += 2.0f*PI;
    }

    // issue:
    // if uv angle is in between start and end, but start is [3pi/2, 2pi]
    // and end is [0, pi/2]
    // if start > end then we need to wrap around
    // in this case we want to see if uv > start or uv < end
        
    // otherwise it's just a normal comparison
    // if uv < end and uv > start
       
    bool inArc = false;

    // handle issue
    if (start > end){
        inArc = (uvAngle >= start) || (uvAngle <= end); 
    } else{
    // handle normal
        inArc = (uvAngle <= end) && (uvAngle >= start);
    }
        
        
    color = mix(color, vec3(1.0f, 1.0f, 0.0f), float(inArc)*ringAlpha);
    return color;
}

void main(){
    // translate uv into [-1,1]
    vec2 UV = 2.0*uv - vec2(1.0, 1.0);  
    
    // 2*uv [0,2] -> 2*uv - (1,1) [-1,1]

    // base ring
    vec3 color = ringColor; // base color of the ring

    float ringAlpha = ringSDF(UV); 
    // ringAlpha determines if this color is part of the ring or not

    // need to do a secondary comparison to determine if d is outside or inside
    float d = circleSDF(UV, vec2(0.0, 0.0), r);
    float w = fwidth(d);
    float bgAlpha = 1.0f; // assume bg is always shown
    if (d > 0.0){
        // outside of circle will be hidden
        bgAlpha = smoothstep(ringThickness+10.0*w, ringThickness+2.0*w, d);
    }

    // multiply the base ring color by this alpha
    color = mix(bgColor, ringColor, ringAlpha);
    // will be black if not part of the ring

   
    float pointAlpha = pointSDF(UV);
    // pointAlpha tells us if it's a part of the point on the circle or not

    // check if in tail 
    color = tailCheck(UV, color, ringAlpha);



    color = mix(color, vec3(1.0, 1.0, 0.0), pointAlpha); // mix between (black or white or gray) with yellow

    bgAlpha *= 0.9;

    col = vec4(color, max(bgAlpha, pointAlpha)); // take the max of alpha or pAlpha
    // so if alpha is 0 but pAlpha is higher, then it's likely yellow or somethin

}