#pragma once
#include "ctype.h"
#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <list>
#include <algorithm>

using namespace std;

static const char* gcHLSLTokenStr[] =
{
	"cbuffer",
	"Texture2D",
	"SamplerState",
};

enum HLSLToken
{
    HLSLToken_Cbuffer = 0,
    HLSLToken_Texture2D,
    HLSLToken_SamplerState,
	HLSLToken_Max
};