
#include "stdafx.h"
#include "CPES_Language.h"
#include <vector>
#include <map>

typedef struct _stFunctionDef {
	char *funcName;
	char *argdef; // 'N' for numbers 'I' for identifier 'X' Identifier or Number
} stFunctionDef;

extern stFunctionDef arFunctions[];

typedef DWORD(fnctCALL)(BYTE argc, DWORD argv[]);

extern fnctCALL *arFunctionsCALL[];
