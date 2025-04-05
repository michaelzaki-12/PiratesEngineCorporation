#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D text_diffuse1;

uniform bool gamma;
uniform float gamma1;
uniform bool hdr;

uniform float exposure;


void main(){
    
    vec3 TexColor = texture(text_diffuse1, TexCoords).rgb;
    vec3 result;
     if(hdr)
    {
        // reinhard
        // vec3 result = hdrColor / (hdrColor + vec3(1.0));
        // exposure
        result = vec3(1.0) - exp(-TexColor * exposure);
        // also gamma correct while we're at it       
        result = pow(result, vec3(1.0 / gamma1));
    }
    else
    {
        result = pow(TexColor, vec3(1.0 / 2.2));
    }

    FragColor = vec4(result, 1.0);
    ////FragColor = vec4(TexColor, 1.0); // set all 4 vector values to 1.0
}