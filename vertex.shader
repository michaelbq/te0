#version 330 core
out vec2 TexCoord;
void main()
{
    TexCoord = vec2(float(gl_VertexID&1), float((gl_VertexID>>1)&1));
    gl_Position = vec4(TexCoord, 0.0f, 1.0);
}
