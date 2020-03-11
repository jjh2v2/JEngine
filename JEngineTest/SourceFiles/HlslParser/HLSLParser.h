#pragma once
#include "HLSLTokenizer.h"

class HLSLParser
{
public:
	HLSLParser();
    HLSLParser(std::wstring& _FileName);

public:
	void Reset();
	void SetFileName(std::wstring& _FileName);
    bool Parse(std::wstring& _FileName);
	void ReadFile();
	bool SkipLine(string *_Str);

public:
	std::wstring			mFileName;
	size_t					mHLSLTokenCount[HLSLToken_Max];
	std::list<string>		mMakeStrlineList;
private:
	std::string				mFileData;
	std::vector<string>		mOriginStrlineArr;
};