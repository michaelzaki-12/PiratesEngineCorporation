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
struct Cluster
{
    vec4 minPoint;
    vec4 maxPoint;
    uint count;
    uint lightIndices[100];
};

layout(std430, binding = 1) restrict buffer clusterSSBO {
    Cluster clusters[];
};

struct PointLight
{
    vec4 position;
    vec4 color;

    float constant;
    float linear;
    float quadratic;
    float radius;
};

layout(std430, binding = 2) restrict buffer lightSSBO
{
    PointLight pointLight[];
};

uniform Material material;
uniform Light light[4];

// out section
layout(location = 0) out vec4 FragColor;  
layout(location = 1) out vec4 BrightColor;  

in vec3 Normal;
in vec2 TexCoords;
in vec3 FragPos;
//uniform sampler2D ourTexture;
uniform vec3 lightPos;
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform float far_plane;

// Tangents
in vec3 TangentLightPos;
in vec3 TangentViewPos;
in vec3 TangentFragPos;

uniform float heightScale;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform samplerCube depthMap;
uniform sampler2D ParallaxMap;
uniform sampler2D normalMap;
uniform bool normalMapON;

//lightCULL
uniform float zNear;
uniform float zFar;
uniform uvec3 gridSize;
uniform uvec2 screenDimensions;

uniform bool parallaxmappingEnabled;
vec3 DirectionalLight(Light light, vec3 norm, vec3 viewDir);
const float offset = 1.0 / 300.0;  
float ShadowCalculation(vec3 fragPos);

vec3 PointLightFunc(PointLight pointLight,vec3 norm, vec3 viewDir);
float sqr(float x);
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

vec3 SpecularLight(PointLight light);

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir)
{ 
 // number of depth layers
    const float minLayers = 8.0;
    const float maxLayers = 32.0;
    float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0));
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * heightScale; 
    vec2 deltaTexCoords = P / numLayers;
     
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(ParallaxMap, currentTexCoords).r;
  
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(ParallaxMap, currentTexCoords).r;  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }

 // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(ParallaxMap, prevTexCoords).r - currentLayerDepth + layerDepth;
 
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}


void main(){
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec2 texcoords = TexCoords;
    if(parallaxmappingEnabled == true){
    viewDir  = normalize(TangentViewPos - TangentFragPos); 
    texcoords = ParallaxMapping(TexCoords,  viewDir);
    // then sample textures with new texture coords
    }

    if(normalMapON == true){
    norm = texture(normalMap, texcoords).rgb;
    // transform normal vector to range [-1,1]
    norm = normalize(norm * 2.0 - 1.0); 
    viewDir  = normalize(TangentViewPos - TangentFragPos); 
    }
    uint zTile = uint((log(abs(FragPos.z) / zNear) * gridSize.z) / log(zFar / zNear));
    vec2 tileSize = screenDimensions / gridSize.xy;
    uvec3 tile = uvec3(gl_FragCoord.xy / tileSize, zTile);
    uint tileIndex =
        tile.x + (tile.y * gridSize.x) + (tile.z * gridSize.x * gridSize.y);

    uint lightCount = clusters[tileIndex].count;
    vec3 LighT = vec3(0.0);
    for (uint i = 0; i < lightCount; ++i)
    {
        uint lightIndex = clusters[tileIndex].lightIndices[i];
        PointLight light = pointLight[lightIndex];
        // do cool lighting
        LighT += PointLightFunc(light, norm, viewDir);
    }

    FragColor = vec4(LighT, 1.0);
    //FragColor = vec4(vec3(gl_FragCoord.z), 1.0);

}

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
vec3 PointLightFunc(PointLight pointLight, vec3 norm, vec3 viewDir){
    //ambient part
    if(pointLight.position.w == 0.0){
        return vec3(0.0);
    }
    vec3 lightDir = normalize(vec3(pointLight.position) - FragPos);
    if(normalMapON == true){
        lightDir = normalize(TangentLightPos - TangentFragPos);
    }
    vec2 texCoords = TexCoords;
    if(parallaxmappingEnabled == true){
    texCoords = ParallaxMapping(TexCoords, viewDir);
    }
   if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0){
        discard;
}
    vec3 ambient = 0.1 * vec3(pointLight.color) * vec3(texture(texture_diffuse1, texCoords).rgb);
    //diffuse part
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, norm);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    if (diff == 0.0f) {
        spec = 0.0f;
    }
    vec3 diffuse = vec3(pointLight.color) * diff * vec3(texture(texture_diffuse1, texCoords).rgb);

    //specular part
    //float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 specular = vec3(pointLight.color) * spec * vec3(texture(texture_specular1, texCoords).rgb); 
    // attenuation
    float distance = length(vec3(pointLight.position) - FragPos);
    if(normalMapON == true){
        distance = length(TangentLightPos - TangentFragPos);;
    }
    float attenuation = 1.0 / (pointLight.constant + (pointLight.linear * distance) + (pointLight.quadratic * (distance * distance)));

    float s = distance / pointLight.radius;
    float s2 = sqr(s);
    //float attenuation = sqr(1 - s2) / (1 + (4 * s));
    //float attenuation = 1 / sqr(distance);
    if (s >= 1.0){
        attenuation = 0.0;
    }
    //vec3 attenuation = vec3(pointLight.color) / sqr(((distance + pointLight.radius) + 1));
    //float attenuation = 1.0 / distance * distance;
    ambient  *= attenuation;  
    diffuse   *= attenuation;
    specular *= attenuation;   
    
    float shadow = ShadowCalculation(FragPos);                      
    vec3 lighting = ((ambient + (1.0 - shadow)) * (diffuse + specular));
    return lighting;
}

vec3 DirectionalLight(Light light, vec3 norm, vec3 viewDir){
    //ambient part
    vec3 ambient = light.ambient * texture(texture_diffuse1, TexCoords).rgb;

    vec2 texCoords = TexCoords;
    if(parallaxmappingEnabled == true){
        texCoords = ParallaxMapping(TexCoords, viewDir);
    }
   if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0){
        discard;
        }
    //diffuse part
    //vec3 lightDir = normalize(light.position - FragPos);
    vec3 lightDir = normalize(-light.direction);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(texture_diffuse1, TexCoords).rgb;

    //specular part
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * texture(texture_specular1, TexCoords).rgb;
    float distance = length(light.position - FragPos);
    if(normalMapON == true){
        distance = length(TangentLightPos - TangentFragPos);
    }
    //float attenuation = 1.0 / (distance * distance);
    //
    //diffuse *= attenuation;
    //specular *= attenuation;

    float shadow = ShadowCalculation(FragPos);                      
    vec3 result= ((ambient * objectColor) + (1.0 - shadow) * (diffuse + specular));    
        
    return result;
}

vec3 SpecularLight(PointLight light){
    //ambient part
    vec3 ambient = vec3(light.color) * texture(texture_diffuse1, TexCoords).rgb;

    //diffuse part
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(vec3(light.position) - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = vec3(light.color) * diff * texture(texture_diffuse1, TexCoords).rgb;

    //specular part
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    vec3 specular = vec3(light.color) * spec * texture(texture_specular1, TexCoords).rgb;
    float distance = length(vec3(light.position) - FragPos);
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
    if(normalMapON == true){
    //fragToLight = TangentFragPos - TangentLightPos;
    }
    // ise the fragment to light vector to sample from the depth map    
    float closestDepth = texture(depthMap, fragToLight).r;
    // it is currently in linear range between [0,1], let's re-transform it back to original depth value
    closestDepth *= far_plane;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // test for shadows
    float bias = 0.35; // we use a much larger bias since depth is now in [near_plane, far_plane] range
    float shadow = 0.0;        
    // display closestDepth as debug (to visualize depth cubemap)
    // FragColor = vec4(vec3(closestDepth / far_plane), 1.0);    
    float samples = 20.0;
    float offset  = 0.1;

    vec3 sampleOffsetDirections[20] = vec3[]
    (
       vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
       vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
       vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
       vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
       vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
    );   
    float diskRadius = 0.05;

    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(depthMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= far_plane;   // undo mapping [0;1]
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);  
    return shadow;
}

float sqr(float x)
{
    return x * x;
}
