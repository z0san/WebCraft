precision mediump float;

uniform mat4 u_ViewProj;    // We're actually passing the inverse of the viewproj
// from our CPU, but it's named u_ViewProj so we don't
// have to bother rewriting our ShaderProgram class

uniform vec3 u_Eye; // Camera pos

uniform int u_Time;

// palettes
uniform vec3 sunset[5];
uniform vec3 sunrise[5];
uniform vec3 nightSky[5];
uniform vec3 blueSky[5];
uniform vec3 dusk[5];

const float PI = 3.14159265359;
const float TWO_PI = 6.28318530718;
const float dayCycle = 1200.0;

const vec3 sunColor = vec3(255, 255, 190) / 255.0;

float modTime(float x){return x - floor(x * (1.0 / dayCycle)) * dayCycle;}

// Convert (x, y, z) to (u, v)
// v follow the y vector, where the top(y=1) is 1, the horizon is 0.5, and the bottom is 0.
// u is on x-z plane, where when unit vector x = 1, u = 1, x = -1, u = 0, and z = 1 or -1, u = 0.5.
vec2 sphereToUV(vec3 p) {
    float phi = atan(p.z, p.x);
    if(phi < 0.0) {
        phi += TWO_PI;
    }
    float theta = acos(p.y);
    return vec2(1.0 - phi / TWO_PI, 1.0 - theta / PI);
}

vec3 uvToNightSky(vec2 uv) {
    if(uv.y < 0.5) {
        return nightSky[0];
    }
    else if(uv.y < 0.55) {
        return mix(nightSky[0], nightSky[1], (uv.y - 0.5) / 0.05);
    }
    else if(uv.y < 0.6) {
        return mix(nightSky[1], nightSky[2], (uv.y - 0.55) / 0.05);
    }
    else if(uv.y < 0.65) {
        return mix(nightSky[2], nightSky[3], (uv.y - 0.6) / 0.05);
    }
    else if(uv.y < 0.75) {
        return mix(nightSky[3], nightSky[4], (uv.y - 0.65) / 0.1);
    }
    return nightSky[4];
}

vec3 uvToBlueSky(vec2 uv) {
    if(uv.y < 0.5) {
        return blueSky[0];
    }
    else if(uv.y < 0.6) {
        return mix(blueSky[0], blueSky[1], (uv.y - 0.5) / 0.1);
    }
    else if(uv.y < 0.7) {
        return mix(blueSky[1], blueSky[2], (uv.y - 0.6) / 0.1);
    }
    else if(uv.y < 0.75) {
        return mix(blueSky[2], blueSky[3], (uv.y - 0.7) / 0.05);
    }
    else if(uv.y < 0.85) {
        return mix(blueSky[3], blueSky[4], (uv.y - 0.75) / 0.1);
    }
    return blueSky[4];
}

vec3 uvToSunrise(vec2 uv){
    if(uv.y < 0.5) {
        return sunrise[0];
    }
    else if(uv.y < 0.55) {
        return mix(sunrise[0], sunrise[1], (uv.y - 0.5) / 0.05);
    }
    else if(uv.y < 0.6) {
        return mix(sunrise[1], sunrise[2], (uv.y - 0.55) / 0.05);
    }
    else if(uv.y < 0.65) {
        return mix(sunrise[2], sunrise[3], (uv.y - 0.6) / 0.05);
    }
    else if(uv.y < 0.75) {
        return mix(sunrise[3], sunrise[4], (uv.y - 0.65) / 0.1);
    }
    return sunrise[4];
}

vec3 uvToSunset(vec2 uv) {
    if(uv.y < 0.5) {
        return sunset[0];
    }
    else if(uv.y < 0.55) {
        return mix(sunset[0], sunset[1], (uv.y - 0.5) / 0.05);
    }
    else if(uv.y < 0.6) {
        return mix(sunset[1], sunset[2], (uv.y - 0.55) / 0.05);
    }
    else if(uv.y < 0.65) {
        return mix(sunset[2], sunset[3], (uv.y - 0.6) / 0.05);
    }
    else if(uv.y < 0.75) {
        return mix(sunset[3], sunset[4], (uv.y - 0.65) / 0.1);
    }
    return sunset[4];
}

vec3 dayTime(){
    float time = modTime(float(u_Time));
    vec4 phi = vec4(PI/2.0, PI, 3.0*PI/2.0, TWO_PI);
    vec4 theta = vec4(PI/2.0, PI/12.0, PI/2.0, PI - PI/12.0);
    float t = 0.0;
    float p = 0.0;
    float interval = dayCycle/4.0;

    if(time < interval){
        t = mix(theta[0], theta[1], (time) / interval);
        p = mix(phi[0], phi[1], (time) / interval);
    }
    else if(time < 2.0*interval){
        t = mix(theta[1], theta[2], (time - interval) / interval);
        p = mix(phi[1], phi[2], (time - interval) / interval);
    }
    else if(time < 3.0*interval){
        t = mix(theta[2], theta[3], (time - 2.0*interval) / interval);
        p = mix(phi[2], phi[3], (time - 2.0*interval) / interval);
    }
    else{
        t = mix(theta[3], theta[0], (time - 3.0*interval) / interval);
        p = mix(phi[3] - TWO_PI, phi[0], (time - 3.0*interval) / interval);
    }

    return vec3(sin(t)*cos(p), cos(t), sin(t)*sin(p));
}


float noise1D(int x) {
    int tester = (x * 8192);
    float xf = float(tester);
    return (1.0 - (xf * (xf * xf * 15731.0 + 789221.0)
    + 1376312589.0))
    / 10737741824.0;
}


float noise1D( vec2 p ) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) *
    43758.5453);
}

float noise3D(vec3 p){
    return fract(sin(dot(p, vec3(26.1, 246.9, 526.9))) * 30123.227);
}

float noise3D(vec4 p){
    return fract(sin(dot(p, vec4(96.3, 126.9, 421.4, 42.7))) * 10483.827);
}

vec2 random2( vec2 p ) {
    return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
}

vec3 random3( vec3 p ) {
    return fract(sin(vec3(dot(p,vec3(127.1, 311.7, 191.999)),
    dot(p,vec3(269.5, 183.3, 765.54)),
    dot(p, vec3(420.69, 631.2,109.21))))
    *43758.5453);
}

float surflet(vec2 P, vec2 gridPoint) {
    // Compute falloff function by converting linear distance to a polynomial
    float distX = abs(P.x - gridPoint.x);
    float distY = abs(P.y - gridPoint.y);

    float tX = 1.0 - 6.0 * pow(distX, 5.0) + 15.0 * pow(distX, 4.0) - 10.0 * pow(distX, 3.0);
    float tY = 1.0 - 6.0 * pow(distY, 5.0) + 15.0 * pow(distY, 4.0) - 10.0 * pow(distY, 3.0);
    // Get the random vector for the grid point
    vec2 gradient = random2(gridPoint);
    // Get the vector from the grid point to P
    vec2 diff = P - gridPoint;
    // Get the value of our height field by dotting grid->P with our gradient
    float height = dot(diff, gradient);
    // Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * tX * tY;

}

float surflet(vec3 p, vec3 gridPoint) {
    // Compute the distance between p and the grid point along each axis, and warp it with a
    // quintic function so we can smooth our cells
    vec3 t2 = abs(p - gridPoint);
    vec3 t = vec3(1.0) - 6.0 * pow(t2, vec3(5.0)) + 15.0 * pow(t2, vec3(4.0)) - 10.0 * pow(t2, vec3(3.0));
    // Get the random vector for the grid point (assume we wrote a function random2
    // that returns a vec2 in the range [0, 1])
    vec3 gradient = random3(gridPoint) * 2. - vec3(1., 1., 1.);
    // Get the vector from the grid point to P
    vec3 diff = p - gridPoint;
    // Get the value of our height field by dotting grid->P with our gradient
    float height = dot(diff, gradient);
    // Scale our height field (i.e. reduce it) by our polynomial falloff function
    return height * t.x * t.y * t.z;
}


float perlinNoise(vec2 uv) {
    float surfletSum = 0.0;
    // Iterate over the four integer corners surrounding uv
    for(int dx = 0; dx <= 1; dx+=1) {
        for(int dy = 0; dy <= 1; dy+=1) {
            surfletSum += surflet(uv, floor(uv) + vec2(dx, dy));
        }
    }
    return surfletSum;
}

float perlinNoise3D(vec3 p) {
    float surfletSum = 0.0;
    // Iterate over the four integer corners surrounding uv
    for(int dx = 0; dx <= 1; ++dx) {
        for(int dy = 0; dy <= 1; ++dy) {
            for(int dz = 0; dz <= 1; ++dz) {
                surfletSum += surflet(p, floor(p) + vec3(dx, dy, dz));
            }
        }
    }
    return surfletSum;
}


float interpNoise1D(float x) {
    int intX = int(floor(x));
    float fractX = fract(x);

    float v1 = noise1D(intX);
    float v2 = noise1D(intX + 1);
    return mix(v1, v2, fractX);
}


float fbm(float x) {
    float total = 0.0;
    float persistence = 0.5;

    float freq = 2.0;
    float amp = 0.5;
    for(int i = 1; i <= 8; i+=1) {
        freq *= 2.0;
        amp *= persistence;

        total += interpNoise1D(x * freq) * amp;
    }
    return total;
}

void main()
{
    float time = modTime(float(u_Time));
    //    float time = dayCycle * 23 / 24; /* Night */
    //    float time = 0; /* Sun Rise */

    vec2 ndc = (gl_FragCoord.xy / vec2(1280.0, 720.0)) * 2.0 - 1.0; // -1 to 1 NDC

    vec4 p = vec4(ndc.xy, 1.0, 1.0); // Pixel at the far clip plane
    float noise = fbm(perlinNoise3D(p.xyz * 7.5));
    float n2 = noise3D(p);

    bool starOccur = ((noise > 0.0) && n2 > 0.999) || (fbm(n2) > 0.996);

    p *= 1000.0; // Times far clip plane value

    p = /*Inverse of*/ u_ViewProj * p; // Convert from unhomogenized screen to world

    vec3 rayDir = normalize(p.xyz - u_Eye);

    vec3 outColor = 0.5 * (rayDir + vec3(1.0, 1.0, 1));

    vec2 uv = sphereToUV(rayDir);

    vec3 sunDir = normalize(dayTime());
//        vec3 sunDir = normalize(vec3(0, 0.1, 1));

    float angle = acos(dot(rayDir, sunDir)) * 360.0 / PI;
    float sunSize = 30.0;
    float eggYellow = 7.5;
    float sunLight = 15.0;
    float minHalo = 6.0;
    float minSunSize = 15.0;
//
    vec3 sunsetColor = uvToSunrise(uv);
    vec3 skyColor;


    if(time < dayCycle / 24.0){
        float t = (time + dayCycle/24.0) / (dayCycle/12.0);
        skyColor = mix(nightSky[4], uvToBlueSky(uv), t);

        // The end of Sun Rise
        sunsetColor = mix(uvToSunrise(uv), sunrise[0], (time) / (dayCycle / 24.0));

        sunLight = mix(sunLight, minHalo, (time) / (dayCycle / 24.0));
    }
    else if(time < dayCycle * 3.0 / 8.0){
        skyColor = uvToBlueSky(uv - (time - dayCycle/24.0) / (dayCycle*3.0/8.0 - dayCycle/24.0));

        sunsetColor = sunrise[0];
        sunLight = minHalo;
    }
    else if(time < dayCycle / 2.0 - dayCycle / 24.0){
        skyColor = blueSky[0];

        sunsetColor = mix(sunrise[0], sunset[0], (time - dayCycle * 3.0 / 8.0) / (dayCycle / 12.0));
        sunLight = minHalo;
    }
    else if(time < dayCycle / 2.0 + dayCycle / 24.0){
        float t = (time - dayCycle/2.0 + dayCycle/24.0) / (dayCycle/12.0);
        skyColor = mix(blueSky[0], uvToNightSky(uv), t);

        // The biginning of Sunset
        if(time < dayCycle / 2.0){
            sunsetColor = mix(sunset[0], uvToSunset(uv), (time - dayCycle/2.0 + dayCycle/24.0) / (dayCycle/24.0));
            sunLight = mix(minHalo, sunLight, (time - dayCycle/2.0 + dayCycle/24.0) / (dayCycle/24.0));
        }
        // The end of Sunset
        else{
            sunsetColor = mix(uvToSunset(uv), sunset[0], (time - dayCycle/2.0) / (dayCycle/24.0));
            sunLight = mix(sunLight, minHalo, (time - dayCycle/2.0) / (dayCycle/24.0));
        }

        if(starOccur){
            skyColor = mix(skyColor, vec3(1, 1, 1), t);
        }
    }
    else if(time < dayCycle * 7.0 / 8.0){
        skyColor = uvToNightSky(uv + (time - dayCycle/2.0 - dayCycle/24.0) / (dayCycle*7./8. - dayCycle/24.));

        sunsetColor = sunset[0];
        sunLight = minHalo;

        if(starOccur){
            skyColor = vec3(1, 1, 1);
        }
    }
    else if(time < dayCycle - dayCycle / 24.0){
        skyColor = nightSky[4];

        sunsetColor = mix(sunset[0], sunrise[0], (time - dayCycle * 7.0 / 8.0) / (dayCycle/12.0));
        sunLight = minHalo;

        if(starOccur){
            skyColor = vec3(1, 1, 1);
        }
    }
    else{
        float t = (time - dayCycle + dayCycle/24.0) / (dayCycle/12.0);
        skyColor = mix(nightSky[4], uvToBlueSky(uv), t);

        sunsetColor = mix(sunrise[0], uvToSunrise(uv), (time - dayCycle + dayCycle/24.0) / (dayCycle/24.0));

        sunLight = mix(minHalo, sunLight, (time - dayCycle + dayCycle/24.0) / (dayCycle/24.0));

        if(starOccur){
            skyColor = mix(vec3(1, 1, 1), skyColor, t);
        }
    }

    sunSize = (sunLight / 15.0) * sunSize;

    if(angle < sunSize){
        if(angle < eggYellow){
            outColor = sunColor;
        }
        else{
            outColor = mix(sunColor, sunsetColor, (angle - eggYellow) / (sunSize - eggYellow));
        }
    }
    else{
        if(angle < sunSize + sunLight){
            outColor = sunsetColor;
        }
        else if(angle < sunSize + 9.0*sunLight){
            outColor = mix(sunsetColor, skyColor, (angle - sunSize - sunLight) / (8.0*sunLight));
        }
        else{
            outColor = skyColor;
        }
    }

    gl_FragColor = vec4(outColor, 1);
}