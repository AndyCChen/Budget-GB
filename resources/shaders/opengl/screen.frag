in vec2 textureCoords;

out vec4 FragColor;

uniform sampler2D screenTexture;

void main()
{
	vec2 texCoord = textureCoords;
	texCoord.y = 1.0f - texCoord.y;

	vec4 color = texture(screenTexture, texCoord);

	FragColor = vec4(color.rgb, 1.0f);
}