#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D text_diffuse1;
uniform bool gamma;
void main(){
    vec3 TexColor = texture(text_diffuse1, TexCoords).rgb;
    
    if(gamma == true)
        TexColor = pow(TexColor, vec3(1.0 / 2.2));
        
    FragColor = vec4(TexColor, 1.0); // set all 4 vector values to 1.0
}