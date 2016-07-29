#include <math.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

//
#include "CPES_Lang_Func.h"
#include "CPES_Lang_Exec.h"


// PCOde Execution
DWORD pCodeExecute(const BYTE *ptrPCode, int PCodeLen, BYTE *ptrStackIdentifier, int StackIdentifierLen, BYTE *ptrStack, int StackLen)
{
	DWORD dwRet;
	bool TSTInstructionResult;
	ePCodeSymbols pCodeByte;
	DWORD *ptrDWORDSrc, *ptrDWORDDst, *ptrOP1, *ptrOP2;
	int posPCode;
	int posStack, maxStack = 0;

	TSTInstructionResult = false;
	posPCode = 0;
	posStack = 0;

	pCodeByte = pcsNOP;
	memset(ptrStackIdentifier, 0, StackIdentifierLen);
	memset(ptrStack, 0, StackLen);

	while (pCodeByte != pcsEXIT && posPCode < PCodeLen) {
		// control and stat
		if (posStack > StackLen) {
			printf("Stack overflow (%d // %d)\n", posStack, StackLen);
			break;
		}
		maxStack = max(maxStack, posStack);
		//
		pCodeByte = (ePCodeSymbols)ptrPCode[posPCode];
		switch (pCodeByte) {
		case pcsNOP :
			posPCode++;
			break;
		case pcsPUSHNumber :
			// Get next 4 bytes and push them on the stack
			posPCode++;
			ptrDWORDSrc = (DWORD *)&ptrPCode[posPCode];
			ptrDWORDDst = (DWORD *)&ptrStack[posStack];
			*ptrDWORDDst = *ptrDWORDSrc;
			posPCode += 4;
			posStack += 4;
			break;
		case pcsPUSHIdentifier :
			// Get next byte that reference the 4 bytes in the identifierstack and push them on the stack
			posPCode++;
			ptrDWORDSrc = (DWORD *)&ptrStackIdentifier[ptrPCode[posPCode]*4];
			ptrDWORDDst = (DWORD *)&ptrStack[posStack];
			*ptrDWORDDst = *ptrDWORDSrc;
			posPCode++;
			posStack += 4;
			break;
		case pcsPOP :
			posStack -= 4;
			posPCode++;
			break;
		case pcsPOPV :
			posPCode++;
			posStack -= (ptrPCode[posPCode] * 4);
			posPCode ++;
			break;
		case pcsSETIdentifier :
			posPCode++;
			ptrDWORDDst = (DWORD *)&ptrStackIdentifier[ptrPCode[posPCode]*4];
			ptrDWORDSrc = (DWORD *)&ptrStack[posStack-4];
			*ptrDWORDDst = *ptrDWORDSrc;
			posPCode++;
			break;
		case pcsADD :
			ptrOP1 = (DWORD *)&ptrStack[posStack-8];
			ptrOP2 = (DWORD *)&ptrStack[posStack-4];
			ptrDWORDDst = (DWORD *)&ptrStack[posStack];
			posStack += 4;
			*ptrDWORDDst = (*ptrOP1) + (*ptrOP2);
			posPCode++;
			break;
		case pcsSUB :
			ptrOP1 = (DWORD *)&ptrStack[posStack-8];
			ptrOP2 = (DWORD *)&ptrStack[posStack-4];
			ptrDWORDDst = (DWORD *)&ptrStack[posStack];
			posStack += 4;
			*ptrDWORDDst = (*ptrOP1) - (*ptrOP2);
			posPCode++;
			break;
		case pcsMUL :
			ptrOP1 = (DWORD *)&ptrStack[posStack-8];
			ptrOP2 = (DWORD *)&ptrStack[posStack-4];
			ptrDWORDDst = (DWORD *)&ptrStack[posStack];
			posStack += 4;
			*ptrDWORDDst = (*ptrOP1) * (*ptrOP2);
			posPCode++;
			break;
		case pcsDIV :
			ptrOP1 = (DWORD *)&ptrStack[posStack-8];
			ptrOP2 = (DWORD *)&ptrStack[posStack-4];
			ptrDWORDDst = (DWORD *)&ptrStack[posStack];
			posStack += 4;
			*ptrDWORDDst = (*ptrOP1) / (*ptrOP2);
			posPCode++;
			break;
		case pcsBOR :
			ptrOP1 = (DWORD *)&ptrStack[posStack-8];
			ptrOP2 = (DWORD *)&ptrStack[posStack-4];
			ptrDWORDDst = (DWORD *)&ptrStack[posStack];
			posStack += 4;
			*ptrDWORDDst = (*ptrOP1) | (*ptrOP2);
			posPCode++;
			break;
		case pcsBAND :
			ptrOP1 = (DWORD *)&ptrStack[posStack-8];
			ptrOP2 = (DWORD *)&ptrStack[posStack-4];
			ptrDWORDDst = (DWORD *)&ptrStack[posStack];
			posStack += 4;
			*ptrDWORDDst = (*ptrOP1) & (*ptrOP2);
			posPCode++;
			break;
		case pcsBXOR :
			ptrOP1 = (DWORD *)&ptrStack[posStack-8];
			ptrOP2 = (DWORD *)&ptrStack[posStack-4];
			ptrDWORDDst = (DWORD *)&ptrStack[posStack];
			posStack += 4;
			*ptrDWORDDst = (*ptrOP1) ^ (*ptrOP2);
			posPCode++;
			break;
		case pcsBNOT :
			ptrOP1 = (DWORD *)&ptrStack[posStack-8];
			ptrOP2 = (DWORD *)&ptrStack[posStack-4];
			ptrDWORDDst = (DWORD *)&ptrStack[posStack];
			posStack += 4;
			*ptrDWORDDst = ~(*ptrOP2);
			posPCode++;
			break;
		case pcsLSHFT :
			ptrOP1 = (DWORD *)&ptrStack[posStack-8];
			ptrOP2 = (DWORD *)&ptrStack[posStack-4];
			ptrDWORDDst = (DWORD *)&ptrStack[posStack];
			posStack += 4;
			*ptrDWORDDst = (*ptrOP1) << (*ptrOP2);
			posPCode++;
			break;
		case pcsRSHFT :
			ptrOP1 = (DWORD *)&ptrStack[posStack-8];
			ptrOP2 = (DWORD *)&ptrStack[posStack-4];
			ptrDWORDDst = (DWORD *)&ptrStack[posStack];
			posStack += 4;
			*ptrDWORDDst = (*ptrOP1) >> (*ptrOP2);
			posPCode++;
			break;
		case pcsJMP :
			posPCode++;
			ptrDWORDSrc = (DWORD *)&ptrPCode[posPCode];
			posPCode = *ptrDWORDSrc;
			break;
		case pcsJMPCOND :
			posPCode++;
			ptrDWORDSrc = (DWORD *)&ptrPCode[posPCode];
			if (TSTInstructionResult == false) {
				posPCode = *ptrDWORDSrc;
			}
			else {
				posPCode += 4;
			}
			break;
		case pcsTSTEQ :
			ptrOP1 = (DWORD *)&ptrStack[posStack-8];
			ptrOP2 = (DWORD *)&ptrStack[posStack-4];
			TSTInstructionResult = (((*ptrOP1) == (*ptrOP2)) ? true : false);
			posPCode++;
			break;
		case pcsTSTNEQ :
			ptrOP1 = (DWORD *)&ptrStack[posStack-8];
			ptrOP2 = (DWORD *)&ptrStack[posStack-4];
			TSTInstructionResult = (((*ptrOP1) != (*ptrOP2)) ? true : false);
			posPCode++;
			break;
		case pcsTSTGT :
			ptrOP1 = (DWORD *)&ptrStack[posStack-8];
			ptrOP2 = (DWORD *)&ptrStack[posStack-4];
			TSTInstructionResult = (((*ptrOP1) > (*ptrOP2)) ? true : false);
			posPCode++;
			break;
		case pcsTSTLT :
			ptrOP1 = (DWORD *)&ptrStack[posStack-8];
			ptrOP2 = (DWORD *)&ptrStack[posStack-4];
			TSTInstructionResult = (((*ptrOP1) < (*ptrOP2)) ? true : false);
			posPCode++;
			break;
		case pcsTSTGTOREQ :
			ptrOP1 = (DWORD *)&ptrStack[posStack-8];
			ptrOP2 = (DWORD *)&ptrStack[posStack-4];
			TSTInstructionResult = (((*ptrOP1) >= (*ptrOP2)) ? true : false);
			posPCode++;
			break;
		case pcsTSTLTOREQ :
			ptrOP1 = (DWORD *)&ptrStack[posStack-8];
			ptrOP2 = (DWORD *)&ptrStack[posStack-4];
			TSTInstructionResult = (((*ptrOP1) <= (*ptrOP2)) ? true : false);
			posPCode++;
			break;
		case pcsTSTBOR :
			ptrOP1 = (DWORD *)&ptrStack[posStack-8];
			ptrOP2 = (DWORD *)&ptrStack[posStack-4];
			TSTInstructionResult = ((((*ptrOP1) | (*ptrOP2)) != 0) ? true : false);
			posPCode++;
			break;
		case pcsTSTBAND :
			ptrOP1 = (DWORD *)&ptrStack[posStack-8];
			ptrOP2 = (DWORD *)&ptrStack[posStack-4];
			TSTInstructionResult = ((((*ptrOP1) & (*ptrOP2)) != 0) ? true : false);
			posPCode++;
			break;
		case pcsCALL :
			posPCode++;

			// This work with microsoft compiler but not arm compiler
			//*((DWORD *)&ptrStack[posStack-4]) = arFunctionsCALL[ptrPCode[posPCode++]](ptrPCode[posPCode], ((DWORD *)&ptrStack[posStack-4-(ptrPCode[posPCode]*4)]));

			// This works in both compiler :)
			*((DWORD *)&ptrStack[posStack-4]) = arFunctionsCALL[ptrPCode[posPCode]](ptrPCode[posPCode+1], ((DWORD *)&ptrStack[posStack-4-(ptrPCode[posPCode+1]*4)]));
			posPCode++;

			posPCode++;
			break;
		case pcsCALLVOID :
			posPCode++;

			// This work with microsoft compiler but not arm compiler
			//*((DWORD *)&ptrStack[posStack-4]) = arFunctionsCALL[ptrPCode[posPCode++]](ptrPCode[posPCode], ((DWORD *)&ptrStack[posStack-4-(ptrPCode[posPCode]*4)]));

			// This works in both compiler :)
			arFunctionsCALL[ptrPCode[posPCode]](ptrPCode[posPCode+1], ((DWORD *)&ptrStack[posStack-(ptrPCode[posPCode+1]*4)]));
			posPCode++;

			posPCode++;
			break;
		case pcsEXIT :
			posPCode++;
			if (ptrPCode[posPCode] == pcsPUSHNumber) {
				posPCode++;
				ptrDWORDSrc = (DWORD *)&ptrPCode[posPCode];
				posPCode += 4;
			}
			else {
				posPCode++;
				ptrDWORDSrc = (DWORD *)&ptrStackIdentifier[ptrPCode[posPCode]*4];
				posPCode++;
			}
			dwRet = *ptrDWORDSrc;
			break;
		}	
	}

	if (posStack != 0) {
		printf("ERROR in Stack position : %d\n", posStack);
	}

	printf("Max Stack Size : %d\n", maxStack);
	printf("EXIT : %d\n", dwRet);

	return dwRet;
}
