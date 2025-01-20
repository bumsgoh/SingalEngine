
cbuffer constantBuffer : register(b0)
{
    float4x4 worldMatrix;
};

struct VertexIn
{
    float3 position : POSITION;
};

struct VertexOut
{
    float4 position : SV_POSITION;
};


VertexOut main(VertexIn vin, uint vertexID : SV_VertexID)
{
    VertexOut vout;
    //z�� �׸��� ���� ����, world mat ����
    float4x4 w = { { 0.5, 0, 0, 0 }, { 0, 0.5, 0, 0 }, { 0, 0, 0.5, 0 }, { 0, 0, 0, 1 } };
    float4x4 w1 = { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } };
    vout.position = mul(float4(vin.position.xyz, 1), worldMatrix);
    return vout;

}