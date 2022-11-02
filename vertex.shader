#version 330 core
layout(location = 0) in vec2 apos;
layout(location = 1) in vec2 chpos;
uniform float scale;
out vec2 TexCoord;
void main()
{
    vec2 uv = vec2(float(gl_VertexID&1), float((gl_VertexID>>1)&1));
    vec2 charsize = vec2(13,26);
    TexCoord = vec2(float(chpos.x+uv.x)/16.0, float(6-chpos.y-1+uv.y)/6.0);
    gl_Position = vec4(((apos+uv)*charsize*scale)/vec2(400.0,300.0), 0.0f, 1.0);
}
