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

cbuffer SceneConstantBuffer : register( b0 )
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
	//diffuseAlbedo.a = 0.5f;
	return diffuseAlbedo;// input.color;
	//return input.color;
}
