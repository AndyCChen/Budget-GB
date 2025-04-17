float4 vs_main(float3 pos : Position) : SV_Position
{
    return float4(pos.x, pos.y, pos.z, 1.0f);

}

float4 ps_main() : SV_Target
{
    return float4(52 / 255.0f, 104 / 255.0f, 86 / 255.0f, 1.0f);
}

