uniform mat4 u_DepthMVP;
uniform mat4 u_Model;

attribute vec4 vs_Pos;             // The array of vertex positions passed to the shader

void main()
{
    gl_Position = u_DepthMVP * u_Model * vs_Pos;
}