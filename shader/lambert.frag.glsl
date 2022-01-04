precision mediump float;

// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
varying vec4 fs_Pos;
varying vec4 fs_Nor;
varying vec4 fs_LightVec;
varying vec4 fs_Col;

uniform sampler2D u_Texture;
uniform int u_Time;
//
float random1(vec3 p) {
    return fract(sin(dot(p,vec3(127.1, 311.7, 191.999)))
    *43758.5453);
}

float myMix(float x, float y, float a) {
    return x * (1.0 - a) + y * a;
}

float myClamp(float x, float min, float max) {
    if (x < max && x > min) {
        return x;
    } else if (x < min) {
        return min;
    } else {
        return max;
    }
}

float mySmoothstep1(float edge0, float edge1, float x) {
    float t = myClamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

float mySmoothStep(float a, float b, float t) {
    t = mySmoothstep1(0.0, 1.0, t);
    return myMix(a, b, t);
}

float cubicTriMix(vec3 p) {
    vec3 pFract = fract(p);
    float llb = random1(floor(p) + vec3(0,0,0));
    float lrb = random1(floor(p) + vec3(1,0,0));
    float ulb = random1(floor(p) + vec3(0,1,0));
    float urb = random1(floor(p) + vec3(1,1,0));

    float llf = random1(floor(p) + vec3(0,0,1));
    float lrf = random1(floor(p) + vec3(1,0,1));
    float ulf = random1(floor(p) + vec3(0,1,1));
    float urf = random1(floor(p) + vec3(1,1,1));

    float mixLoBack = mySmoothStep(llb, lrb, pFract.x);
    float mixHiBack = mySmoothStep(ulb, urb, pFract.x);
    float mixLoFront = mySmoothStep(llf, lrf, pFract.x);
    float mixHiFront = mySmoothStep(ulf, urf, pFract.x);

    float mixLo = mySmoothStep(mixLoBack, mixLoFront, pFract.z);
    float mixHi = mySmoothStep(mixHiBack, mixHiFront, pFract.z);

    return mySmoothStep(mixLo, mixHi, pFract.y);
}
//
float fbm(vec3 p) {
    float amp = 0.5;
    float freq = 4.0;
    float sum = 0.0;
    for(int i = 0; i < 8; i++) {
        sum += cubicTriMix(p * freq) * amp;
        amp *= 0.5;
        freq *= 2.0;
    }
    return sum;
}

void main() {

    float offset = float(u_Time) * 0.01;
    offset += fs_Col.x * 16.0;
    offset += floor(fs_Pos.x);
    offset -= fs_Col.w * 16.0;
    offset = mod(offset, 3.0);
    offset += fs_Col.w * 16.0;
    offset /= 16.0;

    vec4 diffuseColor;
    if (fs_Col.z == 1.0) {
        diffuseColor = texture2D(u_Texture, vec2(offset, fs_Col.y));
    } else {
        diffuseColor = texture2D(u_Texture, vec2(fs_Col));
    }

    // Calculate the diffuse term for Lambert shading
    float diffuseTerm = dot(normalize(fs_Nor), normalize(fs_LightVec));
    // Avoid negative lighting values
    diffuseTerm = clamp(diffuseTerm, 0.0, 1.0);

    float ambientTerm = 0.2;

    float lightIntensity = diffuseTerm + ambientTerm;   //Add a small float value to the color multiplier
    //to simulate ambient lighting. This ensures that faces that are not
    //lit by our point light are not completely black.

    gl_FragColor = vec4(vec3(diffuseColor) * lightIntensity, diffuseColor.a);
//    gl_FragColor = vec4(vec3(normalize(diffuseColor)) * lightIntensity, 1);
//
//
//    vec4 diffuseColor = fs_Col;
//    diffuseColor = diffuseColor * (0.5 * fbm(fs_Pos.xyz) + 0.5);
//
//    // Calculate the diffuse term for Lambert shading
//    float diffuseTerm = dot(normalize(fs_Nor), normalize(fs_LightVec));
//    // Avoid negative lighting values
//    diffuseTerm = clamp(diffuseTerm, 0.0, 1.0);
//
//    float ambientTerm = 0.2;
//
//    float lightIntensity = diffuseTerm + ambientTerm;   //Add a small float value to the color multiplier
//    //to simulate ambient lighting. This ensures that faces that are not
//    //lit by our point light are not completely black.
//
//    // Compute final shaded color
//    gl_FragColor = vec4(diffuseColor.rgb * lightIntensity, 1.0);
}

////precision mediump float;
////varying vec4 fs_Col;
//
//void main()
//{
//    float f = fs_Col.x;
//    gl_FragColor = fs_Col;
//}