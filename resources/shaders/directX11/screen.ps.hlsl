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
    
    float4 colors = screenTexture.Load(int3(input.texCoord.x * textureWidth, (1.0f - input.texCoord.y) * textureHeight, 0));
    
    //return float4(0.5f, 0.5f, 0.5f, 1.0f);
    return float4(colors.rgb, 1.0f);
}