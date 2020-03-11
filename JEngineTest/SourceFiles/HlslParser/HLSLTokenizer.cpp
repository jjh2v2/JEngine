#pragma once
#include "HLSLTokenizer.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// The order here must match the order in the Token enum.
static const char* _reservedWords[] =
    {
        "float",
        "float2",
        "float3",
        "float4",
        "float3x3",
        "float4x4",
        "half",
        "half2",
        "half3",
        "half4",
        "half3x3",
        "half4x4",
        "bool",
        "int",
        "int2",
        "int3",
        "int4",
        "uint",
        "uint2",
        "uint3",
        "uint4",
        "texture",
        "sampler2D",
        "samplerCUBE",
        "if",
        "else",
        "for",
        "while",
        "break",
        "true",
        "false",
        "void",
        "struct",
        "cbuffer",
        "tbuffer",
        "register",
        "return",
        "continue",
        "discard",
        "const",
        "packoffset",
        "uniform",
        "in",
        "inout",
    };

static bool GetIsSymbol(char c)
{
    switch (c)
    {
    case ';':
    case ':':
    case '(': case ')':
    case '[': case ']':
    case '{': case '}':
    case '-': case '+':
    case '*': case '/':
    case '?':
    case '!':
    case ',':
    case '=':
    case '.':
    case '<': case '>':
        return true;
    }
    return false;
}