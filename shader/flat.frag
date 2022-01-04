precision mediump float;
varying vec4 fs_Col;

void main()
{
    float f = fs_Col.x;
    gl_FragColor = fs_Col;
}
//
//precision mediump float;

//void main() {
//  gl_FragColor[0] = gl_FragCoord.x/640.0;
//  gl_FragColor[1] = gl_FragCoord.y/480.0;
//  gl_FragColor[2] = 0.5;
//}
