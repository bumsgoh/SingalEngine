
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
    vout.position = mul(float4(vin.position, 1), worldMatrix);
    
    float2 positions[4] =
    {
        float2(-1.0f, -1.0f), // Bottom-left
        float2(-1.0f, 1.0f), // Top-left
        float2(1.0f, -1.0f), // Bottom-right
        float2(1.0f, 1.0f) // Top-right
    };
    vout.position = float4(positions[vertexID], 0, 1.0f);
    //return mul(float4(vin.position, 1), worldMatrix);
    //vout.position = float4(vin.position, 1);
    return vout;

}