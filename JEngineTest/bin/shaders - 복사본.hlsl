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

//PSInput VSMain(float4 position : POSITION, float4 color : COLOR)
PSInput VSMain(VertexInput input)
{
    PSInput result;

    result.position = input.position;
    result.color = input.color;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return input.color;
}
