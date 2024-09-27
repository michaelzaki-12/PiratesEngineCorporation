#version 460 core
out vec4 FragColor;

in vec3 TexCoord;

in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform sampler2D equirectangularMap;

void main()
{    
    vec3 I = normalize(Position - cameraPos);
    vec3 R = reflect(I, normalize(Normal));
    FragColor = vec4(texture(equirectangularMap, TexCoord).rgb, 1.0);
}