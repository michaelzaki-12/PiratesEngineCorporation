#version 330 core

struct Material {
    vec3 ambient;
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    float shininess;
};
uniform sampler2D texture_diffuse1;
uniform sampler2D shadowMap;
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
uniform Light light[NR_POINT_LIGHT];
out vec4 FragColor;  

in vec3 Normal;
in vec2 TexCoords;
in vec3 FragPos;
in vec4 FragPosLightSpace;
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
vec3 DirectionalLight(Light light, vec3 norm, vec3 viewDir);


//vec4 DiffuseLight(){
//    // ambient
//    float ambientStrength = 0.25;
//    vec3 ambient = material.ambient * lightColor;
//  	
//    // diffuse 
//    vec3 norm = normalize(Normal);
//    vec3 lightDir = normalize(light.position - FragPos);
//    float diff = max(dot(norm, lightDir), 0.0);
//    vec3 diffuse = diff * lightColor;
//            
//    vec3 result = (ambient + diffuse) * objectColor;
//    return vec4(result, 1.0);
//}

vec3 SpecularLight(Light light);
vec4 SpotLight(Light light){
    vec3 lightDir = normalize(light.position - FragPos);
    // check if lighting is inside the spotlight cone
    //ambient part
    vec3 ambient = light.ambient * texture(material.texture_diffuse1, TexCoords).rgb;

    //diffuse part
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.texture_diffuse1, TexCoords).rgb;

    //specular part
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.texture_specular1, TexCoords).rgb; 
    
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
float ShadowCalculation(vec4 fragPosLightSpace);


void main(){
    vec3 Light = vec3(0.0);
    for(int i = 0; i < 1; i++)
        Light += SpecularLight(light[i]);
    FragColor = vec4(Light, 1.0);;
    //FragColor = vec4(vec3(gl_FragCoord.z), 1.0);
}

vec3 PointLight(Pointlight light, vec3 norm, vec3 viewDir){
    //ambient part
    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords).rgb);

    //diffuse part
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords).rgb);

    //specular part
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoords).rgb); 
    
    // attenuation
    float distance    = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

    ambient  *= attenuation;  
    diffuse   *= attenuation;
    specular *= attenuation;   

    return (ambient + diffuse + specular) * objectColor;
}

vec3 DirectionalLight(Light light, vec3 norm, vec3 viewDir){
    //ambient part
    vec3 ambient = light.ambient * texture(material.texture_diffuse1, TexCoords).rgb;

    //diffuse part
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.texture_diffuse1, TexCoords).rgb;

    //specular part
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.texture_specular1, TexCoords).rgb; 

    return (ambient + diffuse + specular) * objectColor;
}

vec3 SpecularLight(Light light){
    //ambient part
    vec3 ambient = light.ambient * texture(material.texture_diffuse1, TexCoords).rgb;

    //diffuse part
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.texture_diffuse1, TexCoords).rgb;

    //specular part
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(material.texture_specular1, TexCoords).rgb;
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (distance * distance);

    diffuse *= attenuation;
    specular *= attenuation;

    float shadow = ShadowCalculation(FragPosLightSpace);                      
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular));    
    //vec3 result = (ambient + diffuse + specular);
    return lighting;
}

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; 

    float closestDepth = texture(shadowMap, projCoords.xy).r;
    
    float currentDepth = projCoords.z;  
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;  

    return shadow;
}
