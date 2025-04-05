#version 460 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;
in vec3 TexCoord;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(TexCoord)); // make sure to normalize localPos
    vec3 color = texture(equirectangularMap, uv).rgb;
    
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    FragColor = vec4(color, 1.0);

    if(brightness > 1.0)
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
	else
		BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}