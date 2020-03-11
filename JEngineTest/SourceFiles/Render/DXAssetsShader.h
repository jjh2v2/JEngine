#pragma once
#include "../../stdafx.h"
#include "DXAssets.h"
#include "../HlslParser/HLSLParser.h"

enum eShaderStage
{
	//비트 OR 연산자(|)를 사용해 비트를 켤 수 있다.
	//myflags &= ~option4;
	eVertexxShader		= 0,
	ePixelShader		= 1,
	eGeometryShader		= 2,
	eHullShader			= 3,
	eTessellatorShader	= 4,
	eDomainShader		= 5,
	eComputeShader		= 6,
	eMAX				= 7
};

class DXAssetsShader : public DXAssets
{
private:
	DXAssetsShader() 
	{
		DXAssets();
	};

public:
	DXAssetsShader(std::string sAssetsName, std::wstring sFileName);

	std::wstring mcsFileName;
	void Load();

private:
	HLSLParser				mHLSLParser;

	ComPtr<ID3DBlob>		mpVertexShader;
	ComPtr<ID3DBlob>		mpPixelShader;
	ComPtr<ID3DBlob>		mpGeometryShader;
	ComPtr<ID3DBlob>		mpHullShader;
	ComPtr<ID3DBlob>		mpTessellatorShader;
	ComPtr<ID3DBlob>		mpDomainShader;
	ComPtr<ID3DBlob>		mpComputeShader;

	bool					mbOnVertexShader;
	bool					mbOnPixelShader;
	bool					mbOnGeometryShader;
	bool					mbOnHullShader;
	bool					mbOnTessellatorShader;
	bool					mbOnDomainShader;
	bool					mbOnComputeShader;

public:
	ComPtr<ID3DBlob>		GetVertexShader() { return mpVertexShader; };
	ComPtr<ID3DBlob>		GetPixelShader() { return mpPixelShader; };
	ComPtr<ID3DBlob>		GetGeometryShader() { return mpGeometryShader; };
	ComPtr<ID3DBlob>		GetHullShader() { return mpHullShader; };
	ComPtr<ID3DBlob>		GetTessellatorShader() { return mpTessellatorShader; };
	ComPtr<ID3DBlob>		GetDomainShader() { return mpDomainShader; };
	ComPtr<ID3DBlob>		GetComputeShader() { return mpComputeShader; };

	bool					GetOnVertexShader() { return mbOnVertexShader; };
	bool					GetOnPixelShader() { return mbOnPixelShader; };
	bool					GetOnGeometryShader() { return mbOnGeometryShader; };
	bool					GetOnHullShader() { return mbOnHullShader; };
	bool					GetOnTessellatorShader() { return mbOnTessellatorShader; };
	bool					GetOnDomainShader() { return mbOnDomainShader; };
	bool					GetOnComputeShader() { return mbOnComputeShader; };
	HLSLParser&				GetHLSLParser() { return mHLSLParser; };
};