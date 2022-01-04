attribute vec4 vs_Pos;
attribute vec4 vs_Col;

varying vec4 fs_Col;

void main() {
    fs_Col = vs_Col;

    gl_Position = vs_Pos;
}
