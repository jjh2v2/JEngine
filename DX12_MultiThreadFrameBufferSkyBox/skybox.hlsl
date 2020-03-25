//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

cbuffer SceneConstantBuffer : register(b0)
{
	float4x4 worldViewProjection;
	float4x4 world;
};

struct VertexInput
{
	float4 position : POSITION;
	float2 uv		: TEXCOORD;
	float3 normal	: NORMAL;
	float3 tangent	: TANGENT;
	float4 color	: COLOR;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float2 uv		: TEXCOORD;
	float3 normal	: NORMAL;
	float3 tangent	: TANGENT;
	float4 color	: COLOR;
};

//Texture2D DiffuseTexture1            : register( t0 );
//SamplerState LinearRepeatSampler1    : register( s0 );
Texture2D    gDiffuseMap : register(t0);
SamplerState gsamLinear  : register(s0);
//
//Texture2D DiffuseTexture2            : register(t1);
//SamplerState LinearRepeatSampler2    : register(s1);

//PSInput VSMain(float4 position : POSITION, float4 color : COLOR)
PSInput VSMain(VertexInput input)
{
	PSInput result;

	//result.position = input.position +offset;
	result.position = mul(float4(input.position.xyz, 1.0f), worldViewProjection);
	result.color = input.color;
	result.uv = input.uv;

	return result;
}

float4 PSMain(PSInput input) : SV_TARGET0
{
	float4 diffuseAlbedo = gDiffuseMap.Sample(gsamLinear, input.uv);
	float4 color = float4(1.0f, 1.0f, 0.0f, 1.0f);
	diffuseAlbedo.a = 0.5f;
	return diffuseAlbedo;// input.color;
	//return input.color;
}


/*
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

*/