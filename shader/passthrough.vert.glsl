attribute vec4 vs_Pos;
attribute vec4 vs_Col;

varying vec4 fs_Col;
varying vec4 fs_Pos;

void main()
{
    fs_Col = vs_Col;
    fs_Pos = vs_Pos;
    gl_Position = vs_Pos;
}