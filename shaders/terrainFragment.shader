#version 330 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;
in vec2 uv;
in vec3 worldPosition;
uniform sampler2D mainTex;
uniform sampler2D normalTex;
uniform sampler2D dirt, sand, grass, rock, snow;
uniform vec3 lightDirection;
uniform vec3 cameraPosition;

vec3 lerp(vec3 a, vec3 b, float t) {
    return a + (b - a) * t;
}

void main()
{
    // normal Map
    vec3 normal = texture(normalTex, uv).rgb; 
    normal = normalize(normal * 2.0 - 1.0);
    normal.gb = normal.bg;
    normal.r = -normal.r;
    normal.b = -normal.b;
    
    // specular data
    vec3 viewDir = normalize(worldPosition.rgb - cameraPosition);
    // vec3 reflDir = normalize (reflect(lightDirection, normal));
    
    // lighting
    float lightValue = max(-dot(normal, lightDirection), 0.0f);
    // float specular = pow(max(-dot(reflDir, viewDir), 0.0), 8);
    
    // build color
    float y = worldPosition.y;
    float ds = clamp((y - 20) / 10, -1, 1) * .5 + .5;
    float sg = clamp((y - 40) / 10, -1, 1) * .5 + .5;
    float gr = clamp((y - 60) / 10, -1, 1) * .5 + .5;
    float rs = clamp((y - 80) / 10, -1, 1) * .5 + .5;
    
    float dist = length(worldPosition.xyz - cameraPosition);
    float uvLerp = clamp((dist - 250) / 150, -1, 1) * .5 + .5;
    
    vec3 dirtColorClose = texture(dirt, uv * 100).rgb;
    vec3 sandColorClose = texture(sand, uv * 100).rgb;
    vec3 grassColorClose = texture(grass, uv * 100).rgb;
    vec3 rockColorClose = texture(rock, uv * 100).rgb;
    vec3 snowColorClose = texture(snow, uv * 100).rgb;
    
    vec3 dirtColorFar = texture(dirt, uv * 10).rgb;
    vec3 sandColorFar = texture(sand, uv * 10).rgb;
    vec3 grassColorFar = texture(grass, uv * 10).rgb;
    vec3 rockColorFar = texture(rock, uv * 10).rgb;
    vec3 snowColorFar = texture(snow, uv * 10).rgb;
    
    vec3 dirtColor = lerp(dirtColorClose, dirtColorFar, uvLerp);
    vec3 sandColor = lerp(sandColorClose, sandColorFar, uvLerp);
    vec3 grassColor = lerp(grassColorClose, grassColorFar, uvLerp);
    vec3 rockColor = lerp(rockColorClose, rockColorFar, uvLerp);
    vec3 snowColor = lerp(snowColorClose, snowColorFar, uvLerp);
    
    vec3 diffuse = lerp(lerp(lerp(lerp(dirtColor, sandColor, ds), grassColor, sg), rockColor, gr), snowColor, rs);
    
    float fog = pow(clamp((dist - 250) / 1000, 0, 1), 2);
    vec3 topColor = vec3(68.0 / 255.0, 118.0 / 255.0, 189.0 / 255.0);
    vec3 botColor = vec3(188.0 / 255.0, 214.0 / 255.0, 231.0 / 255.0);
    vec3 fogColor = (lerp(botColor, topColor, max(viewDir.y, 0.0)));
    
    vec3 finalColor = lerp(diffuse * min(lightValue + 0.01, 1.0), fogColor, fog);
    FragColor = vec4(finalColor, 1.0);

    float brightness = dot(finalColor, vec3(0.2126, 0.7152, 0.0722));

    //treshold
    if(brightness > 0.3)
        BrightColor = vec4(finalColor, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}