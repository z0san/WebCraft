uniform mat4 u_Model;
uniform mat4 u_ViewProj;

attribute vec4 vs_Pos;
attribute vec4 vs_Col;

varying vec4 fs_Col;

void main() {
    vec4 modelposition = u_Model * vs_Pos;
    fs_Col = vs_Col;

    gl_Position = u_ViewProj * modelposition;
}
