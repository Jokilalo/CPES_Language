#include "CPES_Type.h"

#ifndef _CPES_FUNC_VERSION
#define _CPES_FUNC_VERSION 0x00010000
#endif

typedef struct _stFunctionDef {
	char *funcName;
	char *argdef; // 'N' for numbers 'I' for identifier 'X' Identifier or Number
} stFunctionDef;

extern stFunctionDef arFunctions[];

typedef DWORD(fnctCALL)(BYTE argc, DWORD argv[]);

extern fnctCALL *arFunctionsCALL[];
