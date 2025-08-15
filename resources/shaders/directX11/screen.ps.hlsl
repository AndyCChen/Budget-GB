struct PS_INPUT
{
    float4 pos : SV_Position;
    float2 texCoord : TextureCoord;
};

Texture2D<float4> screenTexture : register(t0);

float4 ps_main(PS_INPUT input) : SV_Target
{
    float textureWidth, textureHeight;
    screenTexture.GetDimensions(textureWidth, textureHeight);
    int2 texelCoord = int2(input.texCoord.x * textureWidth, (1.0f - input.texCoord.y) * textureHeight);
    
    float4 color = screenTexture.Load(int3(texelCoord.xy, 0));
    return float4(color.rgb, 1.0f);
}