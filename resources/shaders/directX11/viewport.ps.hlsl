struct PS_INPUT
{
    float4 pos : SV_Position;
    float2 texCoord : TextureCoord;
};

Texture2D<uint> viewportTexture : register(t0);

cbuffer PALETTE_BUFFER : register(b0)
{
    float4 colors[4];
};

float4 ps_main(PS_INPUT input) : SV_Target
{    
    uint sampledColorIndex = viewportTexture.Load(int3(input.texCoord.x, input.texCoord.y, 0)).r;
    
    return float4(colors[sampledColorIndex].rgb, 1.0f);
}