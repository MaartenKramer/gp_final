#version 330 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;
in vec2 TexCoords;
in vec3 Normals;
in vec4 FragPos;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_roughness1;
uniform sampler2D texture_ao1;
uniform vec3 cameraPosition;
uniform vec3 lightDirection;

vec4 lerp(vec4 a, vec4 b, float t) {
    return a + (b - a) * t;
}
vec3 lerp(vec3 a, vec3 b, float t) {
    return a + (b - a) * t;
}
float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

void clip( float x) {
    if (x < 0) discard;
}

void main()
{
    vec4 diffuse = texture(texture_diffuse1, TexCoords);
    vec4 specTex = texture(texture_specular1, TexCoords);
    float light = max(dot(-lightDirection, Normals), 0.0);
    vec3 viewDir = normalize(FragPos.rgb - cameraPosition);
    vec3 refl = reflect(lightDirection, Normals);
    float ambientOcclusion = texture(texture_ao1, TexCoords).r;
    
    float roughness = texture(texture_roughness1, TexCoords).r;
    float spec = pow(max(dot(-viewDir, refl), 0.0), lerp(1, 128, roughness));
    vec3 specular = spec * specTex.rgb;

    vec3 lighting = diffuse.rgb * (light * 1.0 + 0.01) + specular;
    

    vec3 topColor = vec3(68.0 / 255.0, 118.0 / 255.0, 189.0 / 255.0);
    vec3 botColor = vec3(188.0 / 255.0, 214.0 / 255.0, 231.0 / 255.0);
    float dist = length(FragPos.xyz - cameraPosition);
    float fog = pow( clamp((dist - 250) / 1000, 0, 1), 2);
    vec3 fogColor = lerp(botColor, topColor, max(viewDir.y, 0.0));
    
    vec4 outputFC = vec4(lerp(lighting, fogColor, fog), diffuse.a);
    

    FragColor = outputFC;
    

    vec3 finalColor = outputFC.rgb;
    float brightness = dot(finalColor, vec3(0.2126, 0.7152, 0.0722));
    
    // treshold
    if(brightness > 0.3) 
        BrightColor = vec4(finalColor, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}