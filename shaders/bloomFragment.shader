#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D scene;
uniform sampler2D bloomBlur;

void main()
{             
    vec3 hdrColor = texture(scene, TexCoords).rgb;      
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    
    // additive blending
    hdrColor = hdrColor + bloomColor * 0.3;
    
    // tone mapping
    vec3 outputFC = hdrColor / (hdrColor + vec3(1.0));
    
    // gamma
    outputFC = pow(outputFC, vec3(1.0 / 2.2));
    
    FragColor = vec4(outputFC, 1.0);
}