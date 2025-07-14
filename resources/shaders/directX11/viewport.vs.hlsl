struct VS_INPUT
{
    float3 pos : Position;
    float2 texCoord : TextureCoord;
};

struct VS_OUTPUT
{
    float4 pos : SV_Position;
    float2 texCoord : TextureCoord;
};

VS_OUTPUT vs_main(VS_INPUT input)
{
    VS_OUTPUT output;
    output.pos = float4(input.pos, 1.0f);
    output.texCoord = input.texCoord;
    
    return output;
}

