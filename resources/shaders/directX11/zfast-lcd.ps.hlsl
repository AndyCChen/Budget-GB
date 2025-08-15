// This shader is the zfast lcd shader from -> https://github.com/libretro/slang-shaders/blob/master/handheld/shaders/zfast_lcd.slang

struct PS_INPUT
{
    float4 pos : SV_Position;
    float2 texCoord : TextureCoord;
};

Texture2D<float4> screenTexture : register(t0);

static const float2 inputRes = float2(160, 144);
static const float borderMultiplier = 14.0;

float4 ps_main(PS_INPUT input) : SV_Target
{
    float textureWidth, textureHeight;
    screenTexture.GetDimensions(textureWidth, textureHeight);
    int2 texelCoord = int2(input.texCoord.x * textureWidth, (1.0f - input.texCoord.y) * textureHeight);
    
    float2 pixel = float2(input.texCoord.x, 1.0f - input.texCoord.y) * inputRes;
    float2 center = floor(pixel) + float2(0.5, 0.5);
    float2 distFromCenter = abs(center - pixel);
    
    float Y = max(distFromCenter.x, distFromCenter.y);
    
    Y = Y * Y;
    float YY = Y * Y;
    float YYY = YY * Y;
    
    float lineWeight = YY - 2.7 * YYY;
    lineWeight = 1.0 - borderMultiplier * lineWeight;
    
    float4 color = screenTexture.Load(int3(texelCoord.xy, 0)) * lineWeight;
    
    return float4(color.rgb, 1.0f);
}