#version 330 core
uniform sampler2D font;
in vec2 TexCoord;
void main()
{
    gl_FragColor = texture(font, TexCoord);
}
