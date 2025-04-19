struct PS_INPUT
{
	float4 pos : SV_Position;
	float3 color : Color;
	float2 texCoord : TextureCoord;
};

Texture2D viewportTexture : register(t0);
SamplerState viewportTextureSampler : register(s0);

float4 ps_main(PS_INPUT input) : SV_Target
{
	return viewportTexture.Sample(viewportTextureSampler, input.texCoord);
}