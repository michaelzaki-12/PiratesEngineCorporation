#version 460 core
out vec4 FragColor;  
uniform vec4 color;

in vec2 TexCoord;

uniform sampler2D ourTexture;

void main()
{
    FragColor = texture(ourTexture, TexCoord);
}