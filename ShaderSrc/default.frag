#version 460 core

struct Material {
    vec3 ambient;
    float shininess;
};
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
//uniform sampler2D ourTexture;
uniform vec3 lightPos;
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform float far_plane;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform samplerCube depthMap;

vec4 ambientLight(){
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    vec3 result = ambient * objectColor;
    return vec4(result, 1.0);
}
vec3 PointLight(Pointlight light,vec3 norm, vec3 viewDir);
vec3 DirectionalLight(Light light, vec3 norm, vec3 viewDir);
const float offset = 1.0 / 300.0;  
float ShadowCalculation(vec3 fragPos);

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
    vec3 ambient = light.ambient * texture(texture_diffuse1, TexCoords).rgb;
    //diffuse part
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(texture_diffuse1, TexCoords).rgb;

    //specular part
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(texture_specular1, TexCoords).rgb; 
    
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

void main(){
    //vec3 Light = vec3(0.0);
    //Light = SpecularLight(light[0]);
    //FragColor = vec4(Light, 1.0);;
    //FragColor = vec4(vec3(gl_FragCoord.z), 1.0);

    vec3 color = texture(texture_diffuse1, TexCoords).rgb;
    vec3 normal = normalize(Normal);
    vec3 lightColor = vec3(0.3);
    // ambient
    vec3 ambient = 0.3 * color;
    // diffuse
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // calculate shadow
    float shadow = ShadowCalculation(FragPos);                      
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    
    FragColor = vec4(lighting, 1.0);
}

vec3 PointLight(Pointlight light, vec3 norm, vec3 viewDir){
    //ambient part
    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoords).rgb);

    //diffuse part
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, TexCoords).rgb);

    //specular part
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * vec3(texture(texture_specular1, TexCoords).rgb); 
    
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
    vec3 ambient = light.ambient * texture(texture_diffuse1, TexCoords).rgb;

    //diffuse part
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(texture_diffuse1, TexCoords).rgb;

    //specular part
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(texture_specular1, TexCoords).rgb; 

    return (ambient + diffuse + specular) * objectColor;
}

vec3 SpecularLight(Light light){
    //ambient part
    vec3 ambient = light.ambient * texture(texture_diffuse1, TexCoords).rgb;

    //diffuse part
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(texture_diffuse1, TexCoords).rgb;

    //specular part
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(texture_specular1, TexCoords).rgb;
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (distance * distance);

    diffuse *= attenuation;
    specular *= attenuation;

    float shadow = ShadowCalculation(FragPos);                      
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular));    
    //vec3 result = (ambient + diffuse + specular);
    return lighting;
}

float ShadowCalculation(vec3 fragPos)
{
    // perform perspective divide
    // get vector between fragment position and light position
    vec3 fragToLight = fragPos - lightPos;
    // ise the fragment to light vector to sample from the depth map    
    float closestDepth = texture(depthMap, fragToLight).r;
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    closestDepth *= far_plane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // test for shadows
    float bias = 0.05; // we use a much larger bias since depth is now in [near_plane, far_plane] range
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;        
    // display closestDepth as debug (to visualize depth cubemap)
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);    
        
    return shadow;
}
