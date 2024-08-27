#version 330 core

struct Material {
    vec3 ambient;
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    float shininess;
};
uniform sampler2D texture_diffuse1;
struct Light {
    vec3 position;
    float outerCutOff;
    float cutOff;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};
struct Pointlight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
#define NR_POINT_LIGHT 4
uniform Pointlight pointlight[NR_POINT_LIGHT];

uniform Material material;
uniform Light light;
out vec4 FragColor;  

in vec3 Normal;
in vec2 TexCoord;
in vec3 FragPos;
//uniform sampler2D ourTexture;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 viewPos;

vec4 ambientLight(){
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    vec3 result = ambient * objectColor;
    return vec4(result, 1.0);
}
vec3 PointLight(Pointlight light,vec3 norm, vec3 viewDir);
vec3 DirectionalLight(vec3 norm, vec3 viewDir);


vec4 DiffuseLight(){
    // ambient
    float ambientStrength = 0.25;
    vec3 ambient = material.ambient * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
            
    vec3 result = (ambient + diffuse) * objectColor;
    return vec4(result, 1.0);
}

vec4 SpecularLight(){
    //ambient part
    vec3 ambient = light.ambient * texture(material.texture_diffuse1, TexCoord).rgb;

    //diffuse part
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.texture_diffuse1, TexCoord).rgb;

    //specular part
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.texture_specular1, TexCoord).rgb; 

    vec3 result = (ambient + diffuse + specular) * objectColor;
    return vec4(result, 1.0);
}
vec4 SpotLight(){
    vec3 lightDir = normalize(light.position - FragPos);
    // check if lighting is inside the spotlight cone
    //ambient part
    vec3 ambient = light.ambient * texture(material.texture_diffuse1, TexCoord).rgb;

    //diffuse part
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.texture_diffuse1, TexCoord).rgb;

    //specular part
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.texture_specular1, TexCoord).rgb; 
    
    // spotlight (soft edges)
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = (light.cutOff - light.outerCutOff);
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);   

    ambient  *= intensity;  
    diffuse   *= intensity;
    specular *= intensity;   

    vec3 result = (ambient + diffuse + specular) * objectColor;
    return vec4(result, 1.0);
}
const float offset = 1.0 / 300.0;  


void main(){
    //vec3 norm = normalize(Normal);
    //vec3 viewDir = normalize(viewPos - FragPos);

    //vec3 result = DirectionalLight(norm, viewDir);

        vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right    
    );
    float kernel[9] = float[](
        1.0 , 1.0, 1.0,
        1.0 , -8.0, 1.0,
        1.0 , 1.0, 1.0  
    );
     vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(texture_diffuse1, TexCoord.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];

    FragColor = vec4(vec3(texture(texture_diffuse1, TexCoord)), 1.0);;
    //FragColor = vec4(vec3(gl_FragCoord.z), 1.0);
}

vec3 PointLight(Pointlight light, vec3 norm, vec3 viewDir){
    //ambient part
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoord).rgb);

    //diffuse part
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoord).rgb);

    //specular part
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoord).rgb); 
    
    // attenuation
    float distance    = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

    ambient  *= attenuation;  
    diffuse   *= attenuation;
    specular *= attenuation;   

    return (ambient + diffuse + specular) * objectColor;
}

vec3 DirectionalLight(vec3 norm, vec3 viewDir){
    //ambient part
    vec3 ambient = light.ambient * texture(material.texture_diffuse1, TexCoord).rgb;

    //diffuse part
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.texture_diffuse1, TexCoord).rgb;

    //specular part
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.texture_specular1, TexCoord).rgb; 

    return (ambient + diffuse + specular) * objectColor;
}