#version 330 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;
in vec4 worldPosition;
uniform vec3 lightDirection;
uniform vec3 cameraPosition;

vec3 lerp(vec3 a, vec3 b, float t) {
    return a + (b - a) * t;
}

void main()
{
    vec3 topColor = vec3(68.0 / 255.0, 118.0 / 255.0, 189.0 / 255.0);
    vec3 botColor = vec3(188.0 / 255.0, 214.0 / 255.0, 231.0 / 255.0);
    vec3 sunColor = vec3(1.0, 200.0 / 255.0, 50.0 / 255.0);

    vec3 viewDir = normalize(worldPosition.xyz - cameraPosition);
    float sunDot = max(-dot(viewDir, lightDirection), 0.0);
    float sun = pow(sunDot, 64.0);
    vec3 color = lerp(botColor, topColor, max(viewDir.y, 0.0)) + sun * sunColor;

    FragColor = vec4(color, 1.0);

    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));

    //treshold
    if (brightness > 0.2) {
        BrightColor = vec4(color, 1.0);
    }
        
    else {
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}