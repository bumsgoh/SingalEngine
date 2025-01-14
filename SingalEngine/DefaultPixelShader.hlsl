

struct VertexOut
{
    float4 position : SV_POSITION;
};

float4 main(VertexOut pin) : SV_Target
{
	return float4(1.0f, 1.0f, 0.0f, 1.0f);
}