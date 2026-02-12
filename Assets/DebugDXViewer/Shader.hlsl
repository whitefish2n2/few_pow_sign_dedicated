cbuffer TransformData : register(b0)
{
    matrix WVPMatrix; // 64바이트 (float 4x4)
};
struct VS_INPUT {
    float3 pos : POSITION;
    float4 color : COLOR;
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

PS_INPUT VS(VS_INPUT input) {
    PS_INPUT output;

    output.pos = mul(float4(input.pos, 1.0f), WVPMatrix);

    output.color = input.color;
    return output;
}

// 픽셀 셰이더 (진입점: PS)
float4 PS(PS_INPUT input) : SV_TARGET {
    return input.color;
}