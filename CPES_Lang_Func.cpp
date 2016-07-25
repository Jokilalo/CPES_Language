
#include "stdafx.h"
#include "CPES_Language.h"
#include <vector>
#include <map>
#include "CPES_Lang_Func.h"

DWORD CALL_Test(BYTE argc, DWORD argv[]);
DWORD CALL_Sleep(BYTE argc, DWORD argv[]);
DWORD CALL_Print(BYTE argc, DWORD argv[]);

stFunctionDef arFunctions[] = {
	"test", "NIXX",
	"print", "V",
	"sleep", "X",
	NULL, NULL
};

fnctCALL *arFunctionsCALL[] = {
	CALL_Test,
	CALL_Print,
	CALL_Sleep,
	NULL,
};

DWORD CALL_Test(BYTE argc, DWORD argv[])
{
	DWORD dwRet = 0;
	dwRet = argv[0] + argv[1] + argv[2] + argv[3];
	return dwRet;
}

DWORD CALL_Sleep(BYTE argc, DWORD argv[])
{
	DWORD dwRet = 0;
	Sleep(argv[0]);
	return dwRet;
}

DWORD CALL_Print(BYTE argc, DWORD argv[])
{
	DWORD dwRet = 0;

	printf("PRT : ");
	for (dwRet = 0; dwRet < argc; dwRet++) {
		if (dwRet) {
			printf(",%d (0x%08X)", argv[dwRet], argv[dwRet]);
		}
		else {
			printf("%d (0x%08X)", argv[dwRet], argv[dwRet]);
		}
	}
	printf(".\n");
	return dwRet;
}
