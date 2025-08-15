// This shader is the zfast lcd shader from -> https://github.com/libretro/slang-shaders/blob/master/handheld/shaders/zfast_lcd.slang

in vec2 textureCoords;

out vec4 FragColor;

uniform sampler2D screenTexture;

const vec2 inputRes = vec2(160, 144);
const float borderMultiplier = 14.0;

void main()
{
	vec2 texelCoord = textureCoords;
	texelCoord.y = 1.0f - texelCoord.y;

	vec2 pixel = textureCoords.xy * inputRes;
	vec2 center = floor(pixel) + vec2(0.5, 0.5);
	vec2 distFromCenter = abs(center - pixel);

	float Y = max(distFromCenter.x, distFromCenter.y);

	Y = Y * Y;
    float YY = Y * Y;
    float YYY = YY * Y;
    
    float lineWeight = YY - 2.7 * YYY;
    lineWeight = 1.0 - borderMultiplier * lineWeight;

	vec4 color = texture(screenTexture, texelCoord) * lineWeight;

	FragColor = vec4(color.rgb, 1.0f);
}