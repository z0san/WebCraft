precision mediump float;

// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
varying vec4 fs_Pos;
varying vec4 fs_Nor;
varying vec4 fs_LightVec;
varying vec4 fs_Col;

uniform sampler2D u_Texture;
uniform sampler2D u_RenderedTexture;
uniform int u_Time;


void main() {

    mat4 biasMatrix = mat4(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0
    );


    float offset = float(u_Time) * 0.01;
    offset += fs_Col.x * 16.0;
    offset += floor(fs_Pos.x);
    offset -= fs_Col.w * 16.0;
    offset = mod(offset, 3.0);
    offset += fs_Col.w * 16.0;
    offset /= 16.0;

    vec4 color;
    if (fs_Col.z == 1.0) {
        color = texture2D(u_Texture, vec2(offset, fs_Col.y));
    } else {
        color = texture2D(u_Texture, vec2(fs_Col));

        vec4 renderLocation = biasMatrix * fs_LightVec;

        // debug
        if (renderLocation.x <= 0.0) {
            color = vec4(1.0, 0, 0, 1.0);
        } else if (renderLocation.x >= 1.0) {
            color = vec4(0, 1.0, 0, 1.0);
        }

        if (renderLocation.y <= 0.0) {
            color = vec4(0, 0, 1.0, 1.0);
        } else if (renderLocation.y >= 1.0) {
            color = vec4(1.0, 1.0, 0, 1.0);
        }

        if (renderLocation.x > 0.0 && renderLocation.x < 1.0 && renderLocation.y > 0.0 && renderLocation.y < 1.0) {
            vec4 shadowCoord = texture2D(u_RenderedTexture, renderLocation.xy);
            if (shadowCoord.z < renderLocation.z - 0.005) {
                // color = vec4(vec3(renderLocation.z - shadowDepth), 1);

                color = vec4(vec3(color) * 0.5, 1.0);
            }
        }
    }

    gl_FragColor = color;
}
