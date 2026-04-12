#version 330 core
in vec2 uv; // where the fragment is positioned relative to the where the quad is rendered (0,1) left to right (0,1) bottom to top
in vec3 resCol; // color of resource

// because of uv, we can assume the entire rectangle is the bar

out vec4 color;

// global variables
vec3 backgroundColor = vec3(0.0f);
vec3 borderColor = vec3(36.0/255.0, 82.0/255.0, 59.0/255.0);
vec3 healthColor = vec3(1.0f, 252.0/255.0, 89.0f/255.0);
vec3 boostBGColor = vec3(19.0/255.0, 38.0/255.0, 30.0/255.0);
vec3 boostColor = vec3(176.0/255.0, 217.0/255.0, 51.0/255.0);
vec3 lowHealthColor = vec3(1.0f, 0.0, 0.0);
float padding = 0.05;
float sliverPercentage = 0.1;


// p is test point
// wi is the half width of the base rectangle
// he is the half height of the base rectangle
// skew is the length of the slant 
// (so from the base rectangle + skew) is the length of that edge
// -skew slants in the opposite direction

// important about skew
// it shifts the bottom edge -skew units in x
// and it shifts top edge +skew units in x
float paragramSDF( in vec2 p, float wi, float he, float sk )
{
    // slanted edge 
    vec2  e  = vec2(sk,he);
    
    // length of slanted edge
    float e2 = sk*sk + he*he;

    // always have a positive point (remember we only care about distance)
    p = (p.y<0.0)?-p:p;
    // horizontal edge
    vec2  w = p - e; w.x -= clamp(w.x,-wi,wi);
    vec2  d = vec2(dot(w,w), -w.y);
    // vertical edge
    float s = p.x*e.y - p.y*e.x;
    p = (s<0.0)?-p:p;
    vec2  v = p - vec2(wi,0); v -= e*clamp(dot(v,e)/e2,-1.0,1.0);
    d = min( d, vec2(dot(v,v), wi*he-abs(s)));
    return sqrt(d.x)*sign(-d.y);
}



float borderPara(vec2 uvOg, vec3 borderParams){

    float d = paragramSDF(uvOg, borderParams.x, borderParams.y, borderParams.z);
    float w = fwidth(d);
    float alpha = smoothstep(w,-w,d);
    
    return alpha;
}

// health is shifted half over from origin, so center is width of border/2 shifted left
float healthPara(vec2 uvOg, vec3 borderParams, vec3 healthParams){

    vec2 c = vec2(-borderParams.x+healthParams.x, 0.0); //same y axis
    
    vec2 UV = vec2(uvOg.x - c.x, uvOg.y - c.y); // translate the test point into local space

    float paddedWidth = healthParams.x-padding;
    float paddedHeight = healthParams.y-padding;

    // we want the skew to remain the same
    // but because we changed our height, the skew is no longer what we had originally
    // old skew/old height = new skew/new height   (this is basically the property of similar triangles)
    // solve for new skew   old skew * new height/oldHeight
    float paddedSkew = (healthParams.z * (healthParams.y-padding)/healthParams.y); 

    
    float d = paragramSDF(UV, paddedWidth, paddedHeight, paddedSkew);
    float w = fwidth(d);
    float alpha = smoothstep(w,-w,d);
    
    return alpha;
}

float boostPara(vec2 uvOg, vec3 borderParams, vec3 boostParams, float bW, float bRatio){
    vec2 c = vec2(borderParams.x-boostParams.x, 0.0); 
    // 0 y means vertically centered
    //half down and half to the right
    
    vec2 UV = vec2(uvOg.x - c.x, uvOg.y - c.y); // translate the test point into local space

    float paddedWidth = boostParams.x-padding;
    float paddedHeight = boostParams.y-padding;

    // we want the skew to remain the same
    // but because we changed our height, the skew is no longer what we had originally
    // old skew/old height = new skew/new height   (this is basically the property of similar triangles)
    // solve for new skew   old skew * new height/oldHeight
    float paddedSkew = (boostParams.z * (boostParams.y-padding)/boostParams.y); 
    


    float d = paragramSDF(UV, paddedWidth, paddedHeight, paddedSkew);
    float w = fwidth(d);
    float alpha = smoothstep(w,-w,d);
    
    //I MADE AN OOPSIE and did this for health instead of boost (but it applies to boost)
    
    // health ratio from 0 to 1
    // if 1, then all of para gram (full width)
    // if 0, then none of paragram (0 width)
    
    // but we only want to decrease from one side
    
    // so instead of shrinking the width, we limit the width from right side
    
   
    // right edge depends on the y
    // we want to map -height to 0, height to 1
    // naturally the slant is a line, so we just need a linear interpolation
    float normY = (UV.y+paddedHeight)/(2.0*paddedHeight); // y [0, 1] now
    normY = clamp(normY, 0.0, 1.0); // [0,1] frfr
    
    // now we use the above to find which x value is allowable for this y value
    // essentially we get f(y) (a function of y in terms of x)
    // f(-height) should be just x - 0.5*slant
    // f(height) should be just x + 0.5*slant
    // this above is quite literally also a linear interpolation
    // if y = 0 (i.e -height, then map x to just x)
    // if y = 1 (i.e height, then map x to x+0.5*slant) the maximum possible x value
    float rightX = mix(paddedWidth-paddedSkew, paddedWidth+paddedSkew, normY);
    // same thing for left
    float leftX = mix(-paddedWidth-paddedSkew, -paddedWidth+paddedSkew, normY);
    
    
    // not a si
    float healthX = mix(leftX, rightX, bRatio);
   
    float edgeChange = fwidth(UV.x); // how much x changes across the pixels

    float edgeCutoff = smoothstep(healthX + edgeChange, healthX - edgeChange, UV.x); 
    // if the change of X is in between our cutoff, then we went to smoothly blend between them
    // returns 0 if it's "closer" to the unfilled region
    // returns 1 if it's "closer" to the colored region

    edgeCutoff = edgeCutoff*2.0; // map from [0,2]
    edgeCutoff -= 1.0; // map from [-1,1]
    
    return alpha*edgeCutoff;
}


// uniforms
uniform float currentBoost; // current usable boost
uniform float maxBoost; // current maxBoost (aka availableBoost)
uniform float currentHealth;
uniform float maxHealth; // current maxHealth 
// we know that maxBoost is tied to health so no need to worry about the ratio


void main(){
    
    vec2 UV = 2.0*uv - vec2(1.0, 1.0);

    vec3 col = backgroundColor;
    float hRatio = currentHealth/maxHealth;
    float bRatio = (maxBoost >= 1e-6 && currentBoost >= 1e-6) ? currentBoost/maxBoost : 0.0;

    // clamp hRatio to 0,1
    hRatio = clamp(hRatio, 0.0, 1.0);
    // clamp bRatio to 0,1
    bRatio = clamp(bRatio, 0.0, 1.0);
    
    // outermost parallelogram parameters
    vec3 borderParams = vec3(1.45/2.0, 0.8, 0.2); // centered at 0,0
    
    // health initial width
    float hW = hRatio; 
    hW = (currentHealth >= 1.0 ) ? clamp(hW, sliverPercentage, 1.0) : 0.0; // always show a sliver of health (unless you have 0 health)
    // boost initial width
    float bW = 1.0-hW;

    
    // healthbar parameters roughly hW times the width of the border
    vec3 healthParams = vec3(
    borderParams.x*hW,
    borderParams.y, 
    borderParams.z);
    // centered at approximately -borderWidth/2
    // parameters of the boost bar 
    vec3 boostParams = vec3(
    borderParams.x*bW, 
    borderParams.y/1.25, 
    borderParams.z/1.25); // centered at approximately borderWidth/2
    // we scaled the height, this affects the slant, so we scale the slant as well
    // 1.25 means 80%  or well 0.8^-1


    
    float borderAlpha = borderPara(UV, borderParams);
    float healthAlpha = healthPara(UV, borderParams, healthParams);

    float boostAlpha;

    if (hRatio != 0.0) boostAlpha = boostPara(UV, borderParams, boostParams, bW, bRatio);

    else boostAlpha = boostPara(UV, borderParams, boostParams, bW, 0.0); // visual glitch fix

    float changeThreshold = smoothstep(0.2, 0.8, hRatio); // between 0.2 health and 0.8 health we gradient
    vec3 hColor = mix(lowHealthColor, healthColor, changeThreshold); // 0.2 health it stays red, and 0.8 health it stays og health color
    
    col = mix(backgroundColor, borderColor, borderAlpha);
    col = mix(col, hColor, healthAlpha);
    
    if (boostAlpha < 0.0){
        col = mix(col, boostBGColor, -boostAlpha);
    } else{
        col = mix(col, boostColor, boostAlpha);
    }
    
    float bgAlpha = max(borderAlpha, healthAlpha);
    bgAlpha = max(bgAlpha, boostAlpha);
    

    // Output to screen
    color = vec4(col, bgAlpha);
}