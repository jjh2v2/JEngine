#include "DXAssetsManager.h"

std::wstring GetExeFileName()
{
	wchar_t buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	return std::wstring(buffer);
}

std::wstring GetExePath()
{
	std::wstring f = GetExeFileName();
	return f.substr(0, f.find_last_of(L"\\/"));
}

// find all matched file in specified directory
// [INPUT]
//   >> const std::string& _path        Search path        ex) c:/directory/
//   >> const std::string& _filter        Search filter    ex) *.exe or *.*
// [RETURN]
//   >> std::vector<std::string>        All matched file name & extension
std::vector<std::string> get_files_inDirectory(const std::string& _path, const std::string& _filter)
{
	std::string searching = _path + _filter;

	std::vector<std::string> return_;


	__finddata64_t fd;
	intptr_t handle = _findfirsti64(searching.c_str(), &fd);  //현재 폴더 내 모든 파일을 찾는다.

	if (handle == -1)    return return_;

	int result = 0;
	do
	{
		return_.push_back(fd.name);
		result = _findnexti64(handle, &fd);
	} while (result != -1);

	_findclose(handle);

	return return_;
}

void DXAssetsManager::Init()
{
	std::wstring sd = GetExePath();
	sd += L"\\shader\\";
	std::string wstr_turned_to_str = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(sd);
	std::vector<std::string> v;
	v = get_files_inDirectory(wstr_turned_to_str, "*.hlsl");
	// Map Key
	std::vector<std::string> MapKey;
	std::vector<std::wstring> FilePath;
	for (int iIndex=0; iIndex < v.size(); iIndex++)
	{
		std::wstring wstr_turned_to_str = std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(v[iIndex]);
		std::wstring wStrFilePath = sd + wstr_turned_to_str;
		FilePath.push_back(wStrFilePath);
		MapKey.push_back(v[iIndex].substr(0, v[iIndex].find_last_of("./")));

		std::string strKey = MapKey[iIndex];
		CreateAssetsShader(strKey, FilePath[iIndex]);
		CreateAssetsRootSignature(strKey);
		CreateAssetsPipelineState(strKey, GetAssetsRootSignature(strKey).GetRootSignature(), GetAssetsShader(strKey));
		CreateAssetsConstantBuffer(strKey);
	}
}

void DXAssetsManager::CreateAssetsShader(std::string sKey, std::wstring sFilePath)
{
	mMapAssetsShader.insert(std::make_pair(sKey, DXAssetsShader(sKey, sFilePath)));
}

void DXAssetsManager::CreateAssetsRootSignature(std::string sKey)
{
	mMapAssetsRootSignature.insert(std::make_pair(sKey, DXAssetsRootSignature(sKey, GetAssetsShader(sKey).GetHLSLParser())));
}

void DXAssetsManager::CreateAssetsPipelineState(std::string sKey, ComPtr<ID3D12RootSignature> pRootSignature, DXAssetsShader& rAssetsShader)
{
	mMapAssetsPipelineState.insert(std::make_pair(sKey, DXAssetsPipelineState(sKey, pRootSignature, rAssetsShader)));
}


DXAssetsShader&	DXAssetsManager::GetAssetsShader(std::string sKey)
{
	std::unordered_map<std::string, DXAssetsShader>::iterator itr = mMapAssetsShader.find(sKey);
	return itr->second;
};

DXAssetsRootSignature& DXAssetsManager::GetAssetsRootSignature(std::string sKey)
{
	std::unordered_map<std::string, DXAssetsRootSignature>::iterator itr = mMapAssetsRootSignature.find(sKey);
	return itr->second;
}

DXAssetsPipelineState& DXAssetsManager::GetAssetsPipelineState(std::string sKey)
{
	std::unordered_map<std::string, DXAssetsPipelineState>::iterator itr = mMapAssetsPipelineState.find(sKey);
	return itr->second;
}

void DXAssetsManager::CreateAssetsConstantBuffer(std::string sKey)
{
	mMapAssetsConstantBuffer.insert(std::make_pair(sKey, DXAssetsConstantBuffer(sKey)));
}

DXAssetsConstantBuffer& DXAssetsManager::GetAssetsConstantBuffer(std::string sKey)
{
	std::unordered_map<std::string, DXAssetsConstantBuffer>::iterator itr = mMapAssetsConstantBuffer.find(sKey);
	return itr->second;
}