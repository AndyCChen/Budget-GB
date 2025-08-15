in vec2 textureCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;

void main()
{
	vec2 texelCoord = textureCoords;
	texelCoord.y = 1.0f - texelCoord.y;
	vec4 color = texture(screenTexture, texelCoord);
	FragColor = vec4(color.rgb, 1.0f);
}