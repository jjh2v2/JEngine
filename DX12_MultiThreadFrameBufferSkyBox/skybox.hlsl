#define GRootSignature \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
    "CBV(b0, visibility = SHADER_VISIBILITY_VERTEX), " \
    "DescriptorTable(SRV(t0), visibility = SHADER_VISIBILITY_PIXEL), " \
    "StaticSampler(s0, " \
    "filter = FILTER_MIN_MAG_MIP_LINEAR, " \
    "visibility = SHADER_VISIBILITY_PIXEL, " \
    "addressU = TEXTURE_ADDRESS_BORDER, " \
    "addressV = TEXTURE_ADDRESS_BORDER, " \
    "addressW = TEXTURE_ADDRESS_BORDER)"

//struct FPerDrawConstantData
//{
//	float4x4 ObjectToClip;
//	float4x3 ObjectToWorld;
//	float3 Albedo;
//	float Metallic;
//	float Roughness;
//	float AO;
//	float Padding[30];
//};

struct SceneConstantBuffer
{
	float4x4 ObjectToClip;
	float4x4 ObjectToWorld;
};

ConstantBuffer<SceneConstantBuffer> GPerDrawCB : register(b0);

TextureCube GCubeMap : register(t0);
SamplerState GSampler : register(s0);

[RootSignature(GRootSignature)]
void VSMain(
	in float3 InPosition : _Position,
	in float3 InNormal : _Normal,
	out float4 OutPosition : SV_Position,
	out float3 OutTexcoords : _Texcoords)
{
	OutPosition = mul(float4(InPosition, 1.0f), GPerDrawCB.ObjectToClip).xyww;
	OutTexcoords = InPosition;
}

[RootSignature(GRootSignature)]
void PSMain(
	in float4 InPosition : SV_Position,
	in float3 InTexcoords : _Texcoords,
	out float4 OutColor : SV_Target0)
{
	OutColor = GCubeMap.Sample(GSampler, InTexcoords);
}
