uniform mat4 u_Model;

uniform mat4 u_ViewProj;    // The matrix that defines the camera's transformation.
                            // We've written a static matrix for you to use for HW2,
                            // but in HW3 you'll have to generate one yourself

attribute vec4 vs_Pos;             // The array of vertex positions passed to the shader
attribute vec4 vs_Nor;             // The array of vertex normals passed to the shader

attribute vec3 vs_ColInstanced;    // The array of vertex colors passed to the shader.
attribute vec3 vs_OffsetInstanced; // Used to position each instance of the cube

varying vec4 fs_Pos;
varying vec4 fs_Nor;            // The array of normals that has been transformed by u_ModelInvTr. This is implicitly passed to the fragment shader.
varying vec4 fs_LightVec;       // The direction in which our virtual light lies, relative to each vertex. This is implicitly passed to the fragment shader.
varying vec4 fs_Col;            // The color of each vertex. This is implicitly passed to the fragment shader.

const vec4 lightDir = normalize(vec4(0.5, 1, 0.75, 0));  // The direction of our virtual light, which is used to compute the shading of
                                        // the geometry in the fragment shader.

void main()
{
    vec4 offsetPos = vs_Pos + vec4(vs_OffsetInstanced, 0.);
    fs_Pos = offsetPos;
    fs_Col = vec4(vs_ColInstanced, 1.);                         // Pass the vertex colors to the fragment shader for interpolation

    fs_Nor = vs_Nor;

    vec4 modelposition = u_Model * vs_Pos;

    fs_LightVec = (lightDir);  // Compute the direction in which the light source lies

    gl_Position = u_ViewProj * modelposition;// gl_Position is a built-in variable of OpenGL which is
                                             // used to render the final positions of the geometry's vertices
}
