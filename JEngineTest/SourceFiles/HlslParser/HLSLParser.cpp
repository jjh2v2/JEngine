#pragma once
#include "HLSLParser.h"

HLSLParser::HLSLParser() : 
	mFileName(L""),
	mFileData("")
{
	Reset();
}

HLSLParser::HLSLParser(std::wstring& _FileName) :
	mFileName(_FileName),
	mFileData("")
{
	Reset();
}


void HLSLParser::Reset()
{
	for (size_t i=0; i< HLSLToken_Max; i++)
	{
		mHLSLTokenCount[i] = 0;
	}

	mFileName = L"";
	mFileData = "";
	mMakeStrlineList.clear();
	mOriginStrlineArr.clear();
}

void HLSLParser::SetFileName(std::wstring& _FileName)
{
	mFileName = _FileName;
}

bool HLSLParser::Parse(std::wstring& _FileName)
{
	bool _is = true;

	mOriginStrlineArr.clear();
	mMakeStrlineList.clear();
	SetFileName(_FileName);
	ReadFile();

	return _is;
}

void HLSLParser::ReadFile()
{
	// read File
	std::ifstream openFile(mFileName);
	if (openFile.is_open()) {
		std::string line;
		while (getline(openFile, line)) {
			mOriginStrlineArr.push_back(line);
			mMakeStrlineList.push_back(line);
		}
		openFile.close();
	}

	std::list<std::string>::iterator itr;
	itr = mMakeStrlineList.begin();
	while (itr != mMakeStrlineList.end())
	{
		if (SkipLine(&(*itr)))
		{
			itr++;
		}
		else {
			mMakeStrlineList.erase(itr++);
		}
	}

	itr = mMakeStrlineList.begin();
	while (itr != mMakeStrlineList.end())
	{
		itr->erase(std::remove(itr->begin(), itr->end(), ';'), itr->end());
		itr++;
	}
	int isdf = 0;
	int sdfe = isdf;
}

bool HLSLParser::SkipLine(string *_Str)
{
	bool _is = false;
	
	for (size_t i=0; i< HLSLToken_Max; i++)
	{
		int iStrlen = strlen(gcHLSLTokenStr[i]);
		std::string sTemp = _Str->c_str();
		// 공백 제거
		sTemp.erase(std::remove_if(sTemp.begin(), sTemp.end(), isspace), sTemp.end());
		sTemp = sTemp.substr(0, iStrlen);
		// 공백 제거후 검사
		if (sTemp.compare(gcHLSLTokenStr[i]) == 0)
		{
			mHLSLTokenCount[i]++;
			_is = true;
			break;
		}
	}

	return _is;
}



