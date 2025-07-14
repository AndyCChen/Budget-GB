in vec2 textureCoords;

out vec4 FragColor;

uniform vec3[4] PALETTE;
uniform usampler2D mainViewportTexture;

void main()
{
	uint colorIndex = texture(mainViewportTexture, textureCoords).r;
	FragColor = vec4(PALETTE[colorIndex].rgb, 1.0f);
}
