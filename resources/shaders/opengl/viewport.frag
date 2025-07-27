in vec2 textureCoords;

out vec4 FragColor;

uniform vec3[4] PALETTE;
uniform usampler2D mainViewportTexture;
uniform bool invertedYTextureCoord;

void main()
{
	vec2 texCoord = textureCoords;

	if (invertedYTextureCoord)
	{
		texCoord.y = 1 - texCoord.y;
	}

	uint colorIndex = texture(mainViewportTexture, texCoord).r;
	FragColor = vec4(PALETTE[colorIndex].rgb, 1.0f);
}
