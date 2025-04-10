in vec2 textureCoords;

out vec4 FragColor;

uniform sampler2D mainViewportTexture;

void main()
{
	FragColor = texture(mainViewportTexture, textureCoords);
}
