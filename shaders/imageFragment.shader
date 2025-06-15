#version 330 core
out vec4 FragColor;

in vec2 fragCoord;
uniform sampler2D _MainTex;

void main()
{
	FragColor = vec4(1 - texture(_MainTex, fragCoord).rgb, 1.0);
}