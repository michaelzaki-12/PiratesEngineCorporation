#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture_diffuse1;
void main(){
    vec4 TexColor = texture(texture_diffuse1, TexCoords);
    FragColor = TexColor; // set all 4 vector values to 1.0
}