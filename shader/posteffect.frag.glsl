precision mediump float;
varying vec4 fs_Col;

uniform int u_Time;
uniform int u_Surrounding;
uniform sampler2D u_RenderedTexture;

void main()
{
    vec4 color = texture2D(u_RenderedTexture, vec2(fs_Col));

    if(u_Surrounding == 1){
        color.b += 0.3;
    }
    else if(u_Surrounding == 2){
        color = vec4(1.0, 0.0, 0.0, 1.0);
    }

    gl_FragColor = color;
}