#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclure les en-têtes Windows rarement utilisés
// Fichiers d'en-tête Windows :
#include <windows.h>

// Fichiers d'en-tête C RunTime
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include "CPES_Lang_Func.h"

DWORD CALL_Test(BYTE argc, DWORD argv[]);
DWORD CALL_Sleep(BYTE argc, DWORD argv[]);
DWORD CALL_Print(BYTE argc, DWORD argv[]);
DWORD CALL_WaitInputPin(BYTE argc, DWORD argv[]);
DWORD CALL_GetInputPinValue(BYTE argc, DWORD argv[]);
DWORD CALL_SetOutputPin(BYTE argc, DWORD argv[]);
DWORD CALL_DisplayCounter(BYTE argc, DWORD argv[]);
DWORD CALL_GetTemperature(BYTE argc, DWORD argv[]);
DWORD CALL_GetGaugePower(BYTE argc, DWORD argv[]);

stFunctionDef arFunctions[] = {
	"test", "NIXX",
	"print", "V",
	"sleep", "X",
	"WaitInputPin", "XXV",
	"GetInputPinValue", "X",
	"SetOuputPin", "XXV",
	"DisplayCounter", "X",
	"GetTemperature", "",
	"GetGaugePower", "X",
	NULL, NULL
};
DWORD CALL_WaitInputPin(BYTE CPESThId, BYTE argc, DWORD argv[]);
DWORD CALL_GetInputPinValue(BYTE CPESThId, BYTE argc, DWORD argv[]);
DWORD CALL_SetOutputPin(BYTE CPESThId, BYTE argc, DWORD argv[]);
DWORD CALL_DisplayCounter(BYTE CPESThId, BYTE argc, DWORD argv[]);
DWORD CALL_GetTemperature(BYTE CPESThId, BYTE argc, DWORD argv[]);
DWORD CALL_GetGaugePower(BYTE CPESThId, BYTE argc, DWORD argv[]);

fnctCALL *arFunctionsCALL[] = {
	CALL_Test,
	CALL_Print,
	CALL_Sleep,
	CALL_WaitInputPin,
	CALL_GetInputPinValue,
	CALL_SetOutputPin,
	CALL_DisplayCounter,
	CALL_GetTemperature,
	CALL_GetGaugePower,
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

DWORD CALL_WaitInputPin(BYTE argc, DWORD argv[])
{
	BYTE byLoop;
	printf("WaitInputPin(timeout = %d, ", argv[0]);
	for (byLoop = 1; byLoop < argc; byLoop++) {
		if (byLoop > 1)
			printf(", ");
		printf("%d", argv[byLoop]);
	}
	printf(")\n", argv[byLoop]);
	fgetc(stdin);

	return 0;
}
DWORD CALL_GetInputPinValue(BYTE argc, DWORD argv[])
{
	printf("GetInputPinValue(%d)\n", argv[0]);
	//getc();

	return 0;
}
DWORD CALL_SetOutputPin(BYTE argc, DWORD argv[])
{
	DWORD dwRet = 0;
	int iLoop;

	printf("GPIO OUT : ");
	for (iLoop = 0; iLoop < argc; iLoop += 2) {
		printf("Pin[%d]=%d)", argv[iLoop], argv[iLoop+1]);
	}
	printf(".\n");

	return dwRet;
}

DWORD CALL_DisplayCounter(BYTE argc, DWORD argv[])
{
	DWORD dwRet = 1;
	int iLoop;

	printf("DisplayCounter : %d", argv[0]);
	printf(".\n");

	return dwRet;
}

DWORD CALL_GetTemperature(BYTE argc, DWORD argv[])
{
	DWORD dwRet = 37;
	int iLoop;

	printf("GetTemperature");
	printf(".\n");

	return dwRet;
}

DWORD CALL_GetGaugePower(BYTE argc, DWORD argv[])
{
	DWORD dwRet = 1;
	int iLoop;

	printf("GetGaugePower : %d", argv[0]);
	printf(".\n");

	return dwRet;
}

