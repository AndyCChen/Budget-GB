struct VS_INPUT
{
    float3 pos : Position;
    float3 color : Color;
    float2 texCoord : TextureCoord;
};

struct VS_OUTPUT
{
    float4 pos : SV_Position;
    float3 color : Color;
    float2 texCoord : TextureCoord;
};

VS_OUTPUT vs_main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.pos = float4(input.pos, 1.0f);
    output.color = input.color;
    output.texCoord = input.texCoord;
    
    return output;
}

