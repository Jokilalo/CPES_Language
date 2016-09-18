// CPES_Language.cpp : définit le point d'entrée pour l'application console.
//

#include "stdafx.h"
#include "CPES_Language.h"
#include "glob-md5.h"
#include "md5.h"
#include <map>
#include <set>
#include <list>
#include <vector>
#include <string>
extern "C" {
#include "CPES_Lang_Func.h"
#include "CPES_Lang_Exec.h"
};
#include <stdio.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Seul et unique objet application

CWinApp theApp;

using namespace std;


extern "C" {

extern int maxStack;

}

CByteArray	barMD5Digest;

typedef enum _eTokenType {
	ettUnknow = 0,
	ettREM,
	ettREMSLASH,
	ettBEGINREM,
	ettENDREM,
	ettVAR,
	ettLET,
	ettLABEL,
	ettGOTO,
	ettIF,
	ettELSE,
	ettENDIF,
	ettWHILE,
	ettBREAK,
	ettCONTINUE,
	ettDONE,
	ettFOR,
	ettNEXT,
	ettEXIT,
	ettDEFINE,
	ettPRAGMA,
	ettMax
} eTokenType;

char *arTokenCPES[ettMax] = {
	"",
	"rem",
	"//",
	"/*",
	"*/",
	"var",
	"let",
	"label",
	"goto",
	"if",
	"else",
	"endif;",
	"while",
	"break;",
	"continue;",
	"done;",
	"for",
	"next;",
	"exit",
	"#define",
	"#pragma",
};

typedef enum _eTokenSign {
	etsUnknow = 0,
	etsEqual,
	etsLeftParenthesis,
	etsRightParenthesis,
	etsAdd,
	etsSubstract,
	etsMultiply,
	etsDivide,
	etsBitwiseOR,
	etsBitwiseAND,
	etsBitwiseXOR,
	etsBitwiseNOT,
	etsGreaterThan,
	etsLessThan,
	etsGreaterThanOrEqual,
	etsLessThanOrEqual,
	etsNotEqual,
	etsLeftShift,
	etsRightShift,
	etsTestOR,
	etsTestAND,
	etsMax
} eTokenSign;

char *arTokenSigns[etsMax] = {
	"",
	"=",
	"(",
	")",
	"+",
	"-",
	"*",
	"/",
	"|",
	"&",
	"^",
	"~",
	">",
	"<",
	">=",
	"<=",
	"!=",
	"<<",
	">>",
	"||",
	"&&"
};

bool ParseCPESFile(char *fileName);
bool WriteCPESPCode(char *fileName, char cOutputType, BYTE *pcode, int pcodeSize, int stackIdentifierSize, int stackSize);
bool ParseCPESTexte(char *CPESText);
eTokenType getTokenType(char *tokenString);

typedef bool(fnctTokenParse)(char *, char**);

bool parseTTREM(char *CPESTextIn, char **CPESTextOut);
bool parseTTVAR(char *CPESTextIn, char **CPESTextOut);
bool parseTTLET(char *CPESTextIn, char **CPESTextOut);
bool parseTTLABEL(char *CPESTextIn, char **CPESTextOut);
bool parseTTGOTO(char *CPESTextIn, char **CPESTextOut);
bool parseTTIF(char *CPESTextIn, char **CPESTextOut);
bool parseTTELSE(char *CPESTextIn, char **CPESTextOut);
bool parseTTENDIF(char *CPESTextIn, char **CPESTextOut);
bool parseTTWHILE(char *CPESTextIn, char **CPESTextOut);
bool parseTTBREAK(char *CPESTextIn, char **CPESTextOut);
bool parseTTCONTINUE(char *CPESTextIn, char **CPESTextOut);
bool parseTTDONE(char *CPESTextIn, char **CPESTextOut);
bool parseTTFOR(char *CPESTextIn, char **CPESTextOut);
bool parseTTNEXT(char *CPESTextIn, char **CPESTextOut);
bool parseTTEXIT(char *CPESTextIn, char **CPESTextOut);
bool parseTTDEFINE(char *CPESTextIn, char **CPESTextOut);
bool parseTTPRAGMA(char *CPESTextIn, char **CPESTextOut);

fnctTokenParse *arTokenParsers[ettMax] = {
	NULL,
	parseTTREM,
	parseTTREM,
	NULL,
	NULL,
	parseTTVAR,
	parseTTLET,
	parseTTLABEL,
	parseTTGOTO,
	parseTTIF,
	parseTTELSE,
	parseTTENDIF,
	parseTTWHILE,
	parseTTBREAK,
	parseTTCONTINUE,
	parseTTDONE,
	parseTTFOR,
	parseTTNEXT,
	parseTTEXIT,
	parseTTDEFINE,
	parseTTPRAGMA
};

std::vector<char *> gIdentifier;
std::map<DWORD, std::string> gmapIdentifier;
std::vector<char *> gLabels;
std::vector<int> gLabelsRefPCode;
std::vector<std::vector<int>> gLabelsRefInPCode;
std::vector<int> gIfLabelsPosTST;
std::vector<int> gIfLabelsPosELSE;
std::vector<int> gIfLabelsPosWHILETST;
std::vector<int> gIfLabelsPosWHILE;
std::vector<std::vector<int>> gIfLabelsPosWHILEBREAK;
std::vector<int> gIfLabelsPosFOR;
std::vector<int> gIfLabelsPosSTEPVALUE;
std::vector<std::vector<int>> gIfLabelsPosNEXT;

std::map<std::string, DWORD> gmapDefineValues;
std::map<std::string, std::string> gmapPragmaValues;

std::set<std::string> gmapPragmaAllowed;

// Functions arrays
int checkFunctionName(char *tokenString);

bool parseFunctionCall(int iFunctionCall, char *tokenBuff, char *CPESTextIn, char **CPESTextOut);

BYTE stack[4*32];
int posStack;
BYTE identifierStack[4*1024];
int posIdentifierStack;
BYTE pcode[32*1024];
int posPCode;

bool checkIdentifierExists(char *Identifier);
bool parseValueSign(char *CPESTextIn, char **CPESTextOut, eTokenSign *etsType);
bool parseIdentifier(char *CPESTextIn, char **CPESTextOut, char **Identifier);


int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// Initialise MFC et affiche un message d'erreur en cas d'échec
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO : modifiez le code d'erreur selon les besoins
		_tprintf(_T("Erreur irrécupérable : l'initialisation MFC a échoué\n"));
		nRetCode = 1;
	}
	else
	{
		printf("CPES Compiler\n");
		gmapPragmaAllowed.insert(std::string("outFileName"));
		gmapPragmaAllowed.insert(std::string("maxStackSize"));
		gmapPragmaAllowed.insert(std::string("progDescription"));

		if (argc > 1) {
			std::vector<char *>::iterator it;
			memset(stack, 0, sizeof(stack));
			posStack = 0;
			memset(pcode, 0, sizeof(pcode));
			posPCode = 0;
			memset(identifierStack, 0, sizeof(identifierStack));
			posIdentifierStack = 0;

			if (ParseCPESFile(argv[1]) == true) {
				// Jump resolution
				std::vector<char *>::iterator			itLabel;
				std::vector<int>::iterator				itRefLabelPCode;
				std::vector<std::vector<int>>::iterator	itRefLabelInPCode;
				std::vector<int>::iterator				itRefInPCode;

				for (itLabel = gLabels.begin(), itRefLabelPCode = gLabelsRefPCode.begin(), itRefLabelInPCode = gLabelsRefInPCode.begin();
					 itLabel != gLabels.end();
					 itLabel++, itRefLabelPCode++, itRefLabelInPCode++) {
					if (*itRefLabelPCode == 0xFFFFFFFF) {
						break;
					}
					for (itRefInPCode = (*itRefLabelInPCode).begin(); itRefInPCode != (*itRefLabelInPCode).end(); itRefInPCode++) {
						DWORD *ptrDWORD;
						ptrDWORD = (DWORD *)&pcode[*itRefInPCode];
						*ptrDWORD = *itRefLabelPCode;
					}

					delete (*itLabel);
				}
				
				if (itLabel == gLabels.end()) {
					// Jump Resolution fully finished
					if (gIfLabelsPosTST.empty() == true) {
						// No IF/ENDIF mismatch, Execute PCODE !!!!
						char *PCodeOutBinaryFileName = "pcode.out";
						char *PCodeOutCFileName = "pcode.out.c";
						char *PCodeOutASMFileName = "pcode.out.a";
						int iStackSize = sizeof(stack);

						if (argc > 2) {
							PCodeOutBinaryFileName = new char[strlen(argv[2])+1];
							strcpy(PCodeOutBinaryFileName, argv[2]);
						}
						else if (gmapPragmaValues.find(std::string("outFileName")) !=  gmapPragmaValues.end()) {
							std::string strVal = gmapPragmaValues.find(std::string("outFileName"))->second;
							PCodeOutBinaryFileName = new char[strVal.length()+1];
							strcpy(PCodeOutBinaryFileName, strVal.c_str());
						}
						else {
							PCodeOutBinaryFileName = new char[strlen(PCodeOutBinaryFileName)+1];
							strcpy(PCodeOutBinaryFileName, "pcode.out");
						}
						if (argc > 3) {
							PCodeOutCFileName = new char[strlen(argv[3])+1];
							strcpy(PCodeOutCFileName, argv[3]);
						}
						else {
							PCodeOutCFileName = new char[strlen(PCodeOutCFileName)+1];
							strcpy(PCodeOutCFileName, "pcode.out.c");
						}
						if (argc > 4) {
							PCodeOutASMFileName = new char[strlen(argv[3])+1];
							strcpy(PCodeOutASMFileName, argv[3]);
						}
						else {
							PCodeOutASMFileName = new char[strlen(PCodeOutASMFileName)+1];
							strcpy(PCodeOutASMFileName, "pcode.out.a");
						}

						pcode[posPCode++] = pcsEXIT;
						pcode[posPCode++] = pcsPUSHNumber; // Wont be pushed, just for exit value
						*((DWORD *)&pcode[posPCode]) = 0;
						posPCode += 4;

						if (gmapPragmaValues.find(std::string("maxStackSize")) !=  gmapPragmaValues.end()) {
							iStackSize = atoi(gmapPragmaValues.find(std::string("maxStackSize"))->second.c_str());
						}

						printf("PCODE size : %d\n", posPCode);
						printf("Identifier Stack size : %d\n", posIdentifierStack);
						printf("Stack size : %d\n", iStackSize);
						printf("----------\n");
						if (gmapPragmaValues.find(std::string("progDescription")) !=  gmapPragmaValues.end()) {
							std::string strProgDesc = gmapPragmaValues.find(std::string("progDescription"))->second;
							printf("Prog Description : %s\n", strProgDesc.c_str());
							printf("----------\n");
						}

						barMD5Digest.RemoveAll();
						printf("Writing pcode binary file to compute MD5 : %s\n", "tmp.out");
						if (WriteCPESPCode("tmp.out", 'b', pcode, posPCode, posIdentifierStack, iStackSize) == true) {
							printf("success write %s\n", PCodeOutBinaryFileName);
							md5file("tmp.out", barMD5Digest);		
						}

						printf("Writing pcode binary file : %s\n", PCodeOutBinaryFileName);
						if (WriteCPESPCode(PCodeOutBinaryFileName, 'b', pcode, posPCode, posIdentifierStack, iStackSize) == true) {
							printf("success write %s\n", PCodeOutBinaryFileName);
						}

						printf("Writing pcode c file : %s\n", PCodeOutCFileName);
						if (WriteCPESPCode(PCodeOutCFileName, 'c', pcode, posPCode, posIdentifierStack, iStackSize) == true) {
							printf("success write %s\n", PCodeOutCFileName);
						}

						printf("Writing pcode ASM file : %s\n", PCodeOutASMFileName);
						if (WriteCPESPCode(PCodeOutASMFileName, 'a', pcode, posPCode, posIdentifierStack, iStackSize) == true) {
							printf("success write %s\n", PCodeOutASMFileName);
						}

						delete PCodeOutBinaryFileName;
						delete PCodeOutCFileName;
						delete PCodeOutASMFileName;

						pCodeExecute(pcode, posPCode, identifierStack, sizeof(identifierStack), stack, sizeof(stack));

						if (iStackSize < maxStack) {
							printf("Stack size defined is too small (%d needed %d)\n", iStackSize, maxStack);
						}
					}
				}
			}

			for (it = gIdentifier.begin(); it != gIdentifier.end(); it++) {
				delete *it;
			}
			// -----------------------
			// Clear all memory stacks
			// -----------------------
			gIdentifier.clear();
			gmapIdentifier.clear();
			gLabels.clear();
			gLabelsRefPCode.clear();
			gLabelsRefInPCode.clear();
			gIfLabelsPosTST.clear();
			gIfLabelsPosELSE.clear();
			gIfLabelsPosWHILETST.clear();
			gIfLabelsPosWHILE.clear();
			gIfLabelsPosWHILEBREAK.clear();
			gIfLabelsPosFOR.clear();
			gIfLabelsPosSTEPVALUE.clear();
			gIfLabelsPosNEXT.clear();
			gmapDefineValues.clear();
			gmapPragmaValues.clear();
		}
	}

	fgetc(stdin);

	return nRetCode;
}

bool ParseCPESFile(char *fileName)
{
	bool bRet = true;
	FILE *fCPES = fopen(fileName, "r");
	if (fCPES != NULL) {
		fpos_t posCPES;
		fseek(fCPES, 0, SEEK_END);
		if (fgetpos(fCPES, &posCPES) == 0) {
			char *CPESText, *CPESPtr;
			CPESPtr = CPESText = new char[posCPES+1];
			if (CPESText != NULL) {
				int c;
				fseek(fCPES, 0, SEEK_SET);
				memset(CPESText, 0, posCPES+1);
				do {
					c = fgetc(fCPES);
					if (c != EOF) {
						*CPESPtr = c;
						CPESPtr++;
					}
				} while (c != EOF);
			}
			bRet = ParseCPESTexte(CPESText);
			delete CPESText;
		}
		fclose(fCPES);
	}

	return bRet;
}

bool WriteCPESPCode(char *fileName, char cOutputType, BYTE *pcode, int pcodeSize, int stackIdentifierSize, int stackSize)
{
	bool bRet = true;
	FILE *fCPES = NULL;
	if (cOutputType == 'b') {
		fCPES = fopen(fileName, "wb");
	}
	else if (cOutputType == 'c') {
		fCPES = fopen(fileName, "wt");
	}
	else if (cOutputType == 'a') {
		fCPES = fopen(fileName, "wt");
	}
	if (fCPES != NULL) {
		DWORD dwVal;
		if (cOutputType == 'b') {
			fwrite("CPES", 4, 1, fCPES);
			dwVal = _CPES_PCOD_VERSION;
			fwrite(&dwVal, 4, 1, fCPES);
			dwVal = _CPES_FUNC_VERSION;
			fwrite(&dwVal, 4, 1, fCPES);
			if (gmapPragmaValues.find(std::string("progDescription")) !=  gmapPragmaValues.end()) {
				std::string strProgDesc = gmapPragmaValues.find(std::string("progDescription"))->second;
				dwVal = strProgDesc.length();
				fwrite(&dwVal, 4, 1, fCPES);
				fwrite(strProgDesc.c_str(), dwVal, 1, fCPES);
			}
			else {
				dwVal = 0;
				fwrite(&dwVal, 4, 1, fCPES);
			}

			if (barMD5Digest.IsEmpty() == FALSE) {
				// Ecriture du MD5
				dwVal = barMD5Digest.GetSize();
				fwrite(&dwVal, 4, 1, fCPES); // longueur MD5
				fwrite(barMD5Digest.GetData(), dwVal, 1, fCPES);	// MD5
			}

			dwVal = pcodeSize;
			fwrite(&dwVal, 4, 1, fCPES);
			dwVal = stackIdentifierSize;
			fwrite(&dwVal, 4, 1, fCPES);
			dwVal = stackSize;
			fwrite(&dwVal, 4, 1, fCPES);

			fwrite(pcode, 1, pcodeSize, fCPES);
		}
		else if (cOutputType == 'a') {
			BYTE *ptrPCode = pcode;
			DWORD dwOffset;
			ePCodeSymbols pCodeByte;
			int posPCode;

			std::map<DWORD, std::string>::iterator it;

			fprintf(fCPES, "; --------------------------------------\n");
			fprintf(fCPES, "; PCODE desassembled File\n");
			fprintf(fCPES, "; --------------------------------------\n");
			if (gmapPragmaValues.find(std::string("progDescription")) !=  gmapPragmaValues.end()) {
				std::string strProgDesc = gmapPragmaValues.find(std::string("progDescription"))->second;
				fprintf(fCPES, "; %s\n", strProgDesc.c_str());
				fprintf(fCPES, "; --------------------------------------\n");
			}
			if (barMD5Digest.IsEmpty() == FALSE) {
				// Ecriture du MD5
				int iLoop;
				fprintf(fCPES, "; MD5 : ");
				for (iLoop = 0; iLoop < barMD5Digest.GetSize(); iLoop++) {
					fprintf(fCPES, "%02X", barMD5Digest[iLoop]);
				}
				fprintf(fCPES, "\n");
			}
			else {
				fprintf(fCPES, "; NO MD5\n");
			}
			fprintf(fCPES, "; --------------------------------------\n");
			fprintf(fCPES, "; CPES_PCOD_VERSION_GEN %08X\n", _CPES_PCOD_VERSION);
			fprintf(fCPES, "; CPES_FUNC_VERSION_GEN %08X\n", _CPES_FUNC_VERSION);
			fprintf(fCPES, "; Variable stack \n");
			fprintf(fCPES, "VARSTCK:%08X\n", stackIdentifierSize);
			for (dwOffset = 0, it = gmapIdentifier.begin(); it !=  gmapIdentifier.end(); it++, dwOffset += 4) {
				fprintf(fCPES, "	%08X\t%d\t; %s\n", dwOffset, it->first, it->second.c_str());
			}
			fprintf(fCPES, "; PCode \n");

			pCodeByte = pcsNOP;
			posPCode = 0;

			while (posPCode < pcodeSize) {
				pCodeByte = (ePCodeSymbols)ptrPCode[posPCode];
				fprintf(fCPES, "\t%08X\t", posPCode);
				switch (pCodeByte) {
				case pcsNOP : fprintf(fCPES, "NOP\n"); posPCode++; break;
				case pcsPUSHNumber : posPCode++; fprintf(fCPES, "PSHN\t%08X\n", *((DWORD *)&ptrPCode[posPCode])); posPCode += 4; break;
				case pcsPUSHIdentifier : posPCode++; fprintf(fCPES, "PSHI\t%d\t; %s\n", ptrPCode[posPCode], gmapIdentifier.find(ptrPCode[posPCode])->second.c_str()); posPCode++; break;
				case pcsPOP : posPCode++; fprintf(fCPES, "POP\n"); break;
				case pcsPOPV : posPCode++; fprintf(fCPES, "POPV\t%d\n", ptrPCode[posPCode]); posPCode++; break;
				case pcsSETIdentifier : posPCode++; fprintf(fCPES, "SETI\t%d\t; %s\n", ptrPCode[posPCode], gmapIdentifier.find(ptrPCode[posPCode])->second.c_str()); posPCode++; break;
				case pcsADD : posPCode++; fprintf(fCPES, "ADD\n"); break;
				case pcsSUB : posPCode++; fprintf(fCPES, "SUB\n"); break;
				case pcsMUL : posPCode++; fprintf(fCPES, "MUL\n"); break;
				case pcsDIV : posPCode++; fprintf(fCPES, "DIV\n"); break;
				case pcsBOR : posPCode++; fprintf(fCPES, "BOR\n"); break;
				case pcsBAND : posPCode++; fprintf(fCPES, "BAND\n"); break;
				case pcsBXOR : posPCode++; fprintf(fCPES, "BXOR\n"); break;
				case pcsBNOT : posPCode++; fprintf(fCPES, "NOT\n"); break;
				case pcsLSHFT : posPCode++; fprintf(fCPES, "LSHFT\n"); break;
				case pcsRSHFT : posPCode++; fprintf(fCPES, "RSHFT\n"); break;
				case pcsJMP : posPCode++; fprintf(fCPES, "JMP\t\t%08X\n", *((DWORD *)&ptrPCode[posPCode])); posPCode += 4; break;
				case pcsJMPCOND : posPCode++; fprintf(fCPES, "JMPCOND\t%08X\n", *((DWORD *)&ptrPCode[posPCode])); posPCode += 4; break;
				case pcsTSTEQ : posPCode++; fprintf(fCPES, "TSTEQ\n"); break;
				case pcsTSTNEQ : posPCode++; fprintf(fCPES, "TSTNEQ\n"); break;
				case pcsTSTGT : posPCode++; fprintf(fCPES, "TSTGT\n"); break;
				case pcsTSTLT : posPCode++; fprintf(fCPES, "TSTLT\n"); break;
				case pcsTSTBOR : posPCode++; fprintf(fCPES, "TSTBOR\n"); break;
				case pcsTSTBAND : posPCode++; fprintf(fCPES, "TSTBAND\n"); break;
				case pcsTSTGTOREQ : posPCode++; fprintf(fCPES, "TSTGTOREQ\n"); break;
				case pcsTSTLTOREQ : posPCode++; fprintf(fCPES, "TSTLTOREQ\n"); break;
				case pcsCALL :
						posPCode++;
						fprintf(fCPES, "CALL\t%d\t[%d]\t; %s\n", ptrPCode[posPCode], ptrPCode[posPCode+1], arFunctions[ptrPCode[posPCode]].funcName);
						posPCode++;
						posPCode++;
						break;
				case pcsCALLVOID :
						posPCode++;
						fprintf(fCPES, "CALLVOID\t%d\t[%d]\t; %s\n", ptrPCode[posPCode], ptrPCode[posPCode+1], arFunctions[ptrPCode[posPCode]].funcName);
						posPCode++;
						posPCode++;
						break;
				case pcsEXIT : 
						posPCode++;
						if (ptrPCode[posPCode] == pcsPUSHNumber) {
							posPCode++;
							fprintf(fCPES, "EXTN\t%08X\n", *((DWORD *)&ptrPCode[posPCode]));
							posPCode += 4;

						}
						else {
							posPCode++;
							fprintf(fCPES, "EXTI\t%d\t; %s\n", ptrPCode[posPCode], gmapIdentifier.find(ptrPCode[posPCode])->second.c_str());
							posPCode++;
						}
						break;
				};
			}
		}
		else if (cOutputType == 'c') {
			fprintf(fCPES, "// --------------------------------------\n");
			fprintf(fCPES, "// PCODE File\n");
			fprintf(fCPES, "// --------------------------------------\n");
			if (gmapPragmaValues.find(std::string("progDescription")) !=  gmapPragmaValues.end()) {
				std::string strProgDesc = gmapPragmaValues.find(std::string("progDescription"))->second;
				fprintf(fCPES, "// %s\n", strProgDesc.c_str());
				fprintf(fCPES, "// --------------------------------------\n");
			}
			if (barMD5Digest.IsEmpty() == FALSE) {
				// Ecriture du MD5
				int iLoop;
				fprintf(fCPES, "// MD5 : ");
				for (iLoop = 0; iLoop < barMD5Digest.GetSize(); iLoop++) {
					fprintf(fCPES, "%02X", barMD5Digest[iLoop]);
				}
				fprintf(fCPES, "\n");
			}
			else {
				fprintf(fCPES, "// NO MD5\n");
			}
			fprintf(fCPES, "// --------------------------------------\n");
			fprintf(fCPES, "#include \n");
			fprintf(fCPES, "#include <math.h>\n");
			fprintf(fCPES, "#include <stdio.h>\n");
			fprintf(fCPES, "#include <string.h>\n");
			fprintf(fCPES, "#include <ctype.h>\n");
			fprintf(fCPES, "#include <stdlib.h>\n");
			fprintf(fCPES, "#include <string.h>\n");
			fprintf(fCPES, "// --------------------------------------\n");
			fprintf(fCPES, "#include \"CPES_Lang_Func.h\"\n");
			fprintf(fCPES, "#include \"CPES_Lang_Exec.h\"\n");
			fprintf(fCPES, "// --------------------------------------\n");
			fprintf(fCPES, "// Defines\n");
			fprintf(fCPES, "// --------------------------------------\n");
			fprintf(fCPES, "#define CPES_PCOD_VERSION_GEN %08X\n", _CPES_PCOD_VERSION);
			fprintf(fCPES, "#define CPES_FUNC_VERSION_GEN %08X\n", _CPES_FUNC_VERSION);
			fprintf(fCPES, "#define PCode_Size %d\n", pcodeSize);
			fprintf(fCPES, "#define PCode_StackIdentifierSize %d\n", stackIdentifierSize);
			fprintf(fCPES, "#define PCode_StackSize %d\n", stackSize);
			fprintf(fCPES, "// --------------------------------------\n");
			fprintf(fCPES, "// Stacks\n");
			fprintf(fCPES, "// --------------------------------------\n");
			fprintf(fCPES, "BYTE	barStackIdentifier[PCode_StackIdentifierSize];\n");
			fprintf(fCPES, "BYTE	barStack[PCode_StackSize];\n");
			fprintf(fCPES, "// --------------------------------------\n");
			fprintf(fCPES, "// Pcode\n");
			fprintf(fCPES, "// --------------------------------------\n");
			fprintf(fCPES, "const BYTE	barPCode[PCode_Size] = {\n");

			for (dwVal = 0; dwVal < pcodeSize; dwVal++) {
				if (dwVal > 0) {
					fprintf(fCPES, ",");
				}
				if (dwVal && (dwVal % 32) == 0) {
					fprintf(fCPES, "\n");
				}
				fprintf(fCPES, "0x%02X", pcode[dwVal]);
			}

			fprintf(fCPES, "\n};\n");

			fprintf(fCPES, "// --------------------------------------\n");
			fprintf(fCPES, "// Execute PCode \n");
			fprintf(fCPES, "// --------------------------------------\n");
			fprintf(fCPES, "/*\n");
			fprintf(fCPES, "DWORD dwRet;\ndwRet = pCodeExecute(barPCode, PCode_Size, barStackIdentifier, PCode_StackIdentifierSize, barStack, PCode_StackSize);\n");
			fprintf(fCPES, "*/\n");
		}
		fclose(fCPES);
	}

	return bRet;
}

bool ParseCPESTexte(char *CPESText)
{
	bool bRet = true;
	char *parsePos = CPESText;
	char *CPESTextInclude = NULL;
	char *parsePosBeforeInclude = NULL;
	char tokenBuff[64], *ptrToken = tokenBuff, *ptrTokenBegin = NULL;
	eTokenType ettRunning = ettUnknow;
	bool bEnteredQuote = false;
	std::vector<char *> vecParameters;
	bool bParse = true;

	memset(tokenBuff, 0, sizeof(tokenBuff));
	
	do {
		if (parsePosBeforeInclude != NULL) {
			parsePos = parsePosBeforeInclude;
			parsePosBeforeInclude = NULL;
			delete CPESTextInclude;
			CPESTextInclude = NULL;
		}
		while (bRet == true && *parsePos != 0x00) {
			if (ettRunning == ettUnknow) {
				if (*parsePos > 64 || *parsePos < 0 || isspace(*parsePos) == 0) {
					// Not space
					if (ptrTokenBegin == NULL)
						ptrTokenBegin = parsePos;
					*ptrToken++ = *parsePos;
				}
				else {
					if (ptrToken != &tokenBuff[0]) {
						int iFunctionCall = -1;
						char *functionIdentifier = NULL;
						// token to analyse
						ettRunning = getTokenType(tokenBuff);
						// jump to next token
						while (*parsePos != 0x00 && isspace(*parsePos) != 0) {
							parsePos++;
						}
						if (ettRunning == ettUnknow) {
							if (checkIdentifierExists(tokenBuff) == true) {
								// Try to see if it's an identifier assignation
								char *CPESTextIn = parsePos;
								char *CPESTextOut;
								eTokenSign signEqual;
								if (parseValueSign(CPESTextIn, &CPESTextOut, &signEqual) == true && signEqual == etsEqual) {
									// it is, now try to see if it's a function call
									CPESTextIn = CPESTextOut;
									if (parseIdentifier(CPESTextIn, &CPESTextOut, &functionIdentifier) == true) {
										// check identifier vs available functions
										iFunctionCall = checkFunctionName(functionIdentifier);
										if (iFunctionCall == -1) {
											// LET
											ettRunning = ettLET;
											parsePos = ptrTokenBegin;
										}
										else {
											parsePos = CPESTextOut;
										}
									}
								}
							}
							else {
								// Is it a "void" call ?
								iFunctionCall = checkFunctionName(tokenBuff);
								if (iFunctionCall != -1) {
									// it is
									tokenBuff[0] = 0x0; // No return value, void call
								}
								else {
									if (bParse == true && strcmp(tokenBuff, "#include") == 0) {
										if (parsePosBeforeInclude == NULL) {
											FILE *fCPES;
											bool bIncludeOk = false;
											char memC;
											char *posNewLine;
											char *posFileName;
											for (posFileName = NULL, posNewLine = parsePos; *posNewLine != 0x0 && *posNewLine != '\r' && *posNewLine != '\n'; posNewLine++) {
												if (isspace(*posNewLine) == 0 && posFileName == NULL) {
													posFileName = posNewLine;
												}
											}

											memC = *posNewLine;
											*posNewLine = 0x0;

											fCPES = fopen(posFileName, "r");
											if (fCPES != NULL) {
												fpos_t posCPES;
												fseek(fCPES, 0, SEEK_END);
												if (fgetpos(fCPES, &posCPES) == 0) {
													char *CPESPtr;
													CPESPtr = CPESTextInclude = new char[posCPES+1];
													if (CPESTextInclude != NULL) {
														int c;
														fseek(fCPES, 0, SEEK_SET);
														memset(CPESTextInclude, 0, posCPES+1);
														do {
															c = fgetc(fCPES);
															if (c != EOF) {
																*CPESPtr = c;
																CPESPtr++;
															}
														} while (c != EOF);
														bIncludeOk = true;
													}
												}
												fclose(fCPES);
											}
											if (bIncludeOk == true) {
												printf("Including file : %s\n", posFileName);
												*posNewLine = memC;
												parsePosBeforeInclude = posNewLine;
												parsePos = CPESTextInclude;
												bRet = true;
											}
										}
									}
								}
							}
						}

						if (parsePos != CPESTextInclude) {
							bRet = true;
							if (iFunctionCall == -1) {
								if (ettRunning == ettBEGINREM) {
									bParse = false;
									bRet = true;
									parsePos--;
								}
								else if (ettRunning == ettENDREM) {
									bParse = true;
									bRet = true;
									parsePos--;
								} else if (bParse == true) {
									if (arTokenParsers[ettRunning] != NULL) {
										// call specialized parser
										bRet = arTokenParsers[ettRunning](parsePos, &parsePos);
									}
									else {
										bRet = false;
									}
								}
								else {
									parsePos--;
								}
							}
							else {
								bRet = parseFunctionCall(iFunctionCall, tokenBuff, parsePos, &parsePos);
							}

							if (bRet == false) {
								char *posNewLine;
								if (ptrTokenBegin == NULL) {
									ptrTokenBegin = parsePos;
								}
								for (posNewLine = ptrTokenBegin; *posNewLine != 0x0 && *posNewLine != '\r' && *posNewLine != '\n'; posNewLine++);
								*posNewLine = 0x0;
								if (iFunctionCall == -1)
									printf("Error near : %s position %d :\n%s\n", tokenBuff, parsePos-CPESText, ptrTokenBegin);
								else
									printf("Error in function call near : %s position %d :\n%s\n", arFunctions[iFunctionCall].funcName, parsePos-CPESText, ptrTokenBegin);
							}
						}
						else {
								parsePos--; // Because of ++ in the end of the while
						}

						// empty token buffer
						ptrToken = &tokenBuff[0];
						ptrTokenBegin = NULL;
						memset(tokenBuff, 0, sizeof(tokenBuff));

						if (functionIdentifier != NULL) {
							delete functionIdentifier;
						}

						ettRunning = ettUnknow;
					}
					else {
						// token still empty
					}
				}
			}
			else {
				// Token is running
				if (isspace(*parsePos) == 0) {
					// Not space
					if (*ptrToken == '"') {
						// If quote string mark it or leave it
						bEnteredQuote = !bEnteredQuote;
					}
					if (ptrTokenBegin == NULL)
						ptrTokenBegin = parsePos;
					*ptrToken++ = *parsePos;
				}
				else {
					// Space
					if (bEnteredQuote == true) {
						if (ptrTokenBegin == NULL)
							ptrTokenBegin = parsePos;
						*ptrToken++ = *parsePos;
					}
					else {
						// Add argument for the function call
						if (ptrToken != &tokenBuff[0]) {
							char *tokenParam = new char[(&tokenBuff[0] - ptrToken)+1];
							
							memset(tokenParam, 0, (&tokenBuff[0] - ptrToken)+1);
							memcpy(tokenParam, &tokenBuff[0], (&tokenBuff[0] - ptrToken)+1);

							vecParameters.push_back(tokenParam);
						}
					}
				}
			}
			parsePos++;
		}
	} while (parsePosBeforeInclude != NULL);

	if (CPESTextInclude != NULL) {
		delete CPESTextInclude;
		CPESTextInclude = NULL;
	}

	return bRet;
}


bool parseTTREM(char *CPESTextIn, char **CPESTextOut)
{
	bool bRet = true;
	char *Identifier = NULL;

	// Because of microsoft ASSERT (non blocking error but anoying dialog box in DEBUG mode) when char is greter than 128 (that is < 0) just test it
	while (*CPESTextIn > 32 || *CPESTextIn < 0 || iscntrl(*CPESTextIn) == 0) {
		CPESTextIn++;
	}
	*CPESTextOut = CPESTextIn;

	return bRet;
}


eTokenType getTokenType(char *tokenString)
{
	int ettRet;

	for (ettRet = ettUnknow+1; ettRet != ettMax; ettRet++) {
		if (memcmp(arTokenCPES[ettRet], tokenString, strlen(arTokenCPES[ettRet])) == 0) {
		//if (stricmp(arTokenCPES[ettRet], tokenString) == 0) {
			// Found token
			break;
		}
	}

	if (ettRet == ettMax) {
		ettRet = ettUnknow;
	}

	return (eTokenType)ettRet;
}

int checkFunctionName(char *tokenString)
{
	int iRet = -1;
	int iLoop;

	for (iLoop = 0; iRet == -1 && arFunctions[iLoop].funcName != NULL; iLoop++) {
		if (strcmp(arFunctions[iLoop].funcName, tokenString) == 0) {
			iRet = iLoop;
		}
	}

	return iRet;
}

bool checkIdentifierExists(char *Identifier)
{
	bool bRet = false;
	std::vector<char *>::iterator it;
	int lenIdentifier = strlen(Identifier);

	for (it = gIdentifier.begin(); bRet == false && it != gIdentifier.end(); it++) {
		if (strlen(*it) == lenIdentifier && memcmp(*it, Identifier, lenIdentifier) == 0) {
			bRet = true;
		}
	}

	return bRet;
}

int getIdentifierStackIndice(char *Identifier)
{
	int iRet;
	std::vector<char *>::iterator it;
	int lenIdentifier = strlen(Identifier);

	for (it = gIdentifier.begin(), iRet = 0; it != gIdentifier.end(); it++, iRet++) {
		if (strlen(*it) == lenIdentifier && memcmp(*it, Identifier, lenIdentifier) == 0) {
			break;
		}
	}

	return iRet;
}

bool parseIdentifier(char *CPESTextIn, char **CPESTextOut, char **Identifier)
{
	bool bRet = false;

	char IndetiferTmp[64], *ptrIndetiferTmp = &IndetiferTmp[0];

	memset(IndetiferTmp, 0, sizeof(IndetiferTmp));
	// remove any space
	while (*CPESTextIn != 0x00 && isspace(*CPESTextIn) != 0) {
		CPESTextIn++;
	}
	while (*CPESTextIn != 0x00 && isspace(*CPESTextIn) == 0 && *CPESTextIn != ';') {
		*ptrIndetiferTmp++ = *CPESTextIn++;
	}
	while (*CPESTextIn != 0x00 && (isspace(*CPESTextIn) != 0)) {
		CPESTextIn++;
	}

	if (&IndetiferTmp[0] != ptrIndetiferTmp) {
		ptrIndetiferTmp = &IndetiferTmp[0];

		while (*ptrIndetiferTmp != 0x00 && (isalpha(*ptrIndetiferTmp) != 0 || isdigit(*ptrIndetiferTmp) != 0 ||  *ptrIndetiferTmp == '_')) {
			ptrIndetiferTmp++;
		}

		if (*ptrIndetiferTmp == 0x00) {
			*Identifier = new char[(ptrIndetiferTmp - &IndetiferTmp[0]) + 1];
			memset(*Identifier, 0, (ptrIndetiferTmp - &IndetiferTmp[0]) + 1);
			memcpy(*Identifier, &IndetiferTmp[0], (ptrIndetiferTmp - &IndetiferTmp[0]));
			*CPESTextOut = CPESTextIn;
			bRet = true;
		}
		else {
			// Something is wrong in identifier name
		}
	}

	return bRet;
}

bool parseValueNumber(char *CPESTextIn, char **CPESTextOut, DWORD *Number)
{
	bool bRet = false;

	char IndetiferTmp[64], *ptrIndetiferTmp = &IndetiferTmp[0];

	memset(IndetiferTmp, 0, sizeof(IndetiferTmp));
	// remove any space
	while (*CPESTextIn != 0x00 && isspace(*CPESTextIn) != 0) {
		CPESTextIn++;
	}
	while (*CPESTextIn != 0x00 && isspace(*CPESTextIn) == 0 && *CPESTextIn != ';') {
		*ptrIndetiferTmp++ = *CPESTextIn++;
	}

	if (&IndetiferTmp[0] != ptrIndetiferTmp) {
		char *endPtr;
		ptrIndetiferTmp = &IndetiferTmp[0];

		if (IndetiferTmp[0] == '0' && (IndetiferTmp[1] == 'x' || IndetiferTmp[1] == 'X')) {
			// Hexadecimal expression
			ptrIndetiferTmp = &IndetiferTmp[2];
			while (*ptrIndetiferTmp != 0x00 && (isxdigit(*ptrIndetiferTmp) != 0)) {
				ptrIndetiferTmp++;
			}

			if (*ptrIndetiferTmp == 0x00) {
				*Number = strtoul(&IndetiferTmp[2], &endPtr, 16);
				bRet = true;
			}
		}
		else if (IndetiferTmp[0] == '0' && (IndetiferTmp[1] == 'b' || IndetiferTmp[1] == 'B')) {
			// Hexadecimal expression
			ptrIndetiferTmp = &IndetiferTmp[2];
			while (*ptrIndetiferTmp != 0x00 && (*ptrIndetiferTmp == '0' || *ptrIndetiferTmp == '1')) {
				ptrIndetiferTmp++;
			}

			if (*ptrIndetiferTmp == 0x00) {
				*Number = strtoul(&IndetiferTmp[2], &endPtr, 2);
				bRet = true;
			}
		}
		else {
			if (*ptrIndetiferTmp == '-') {
				// Negative sign at first place
				ptrIndetiferTmp++;
			}
			while (*ptrIndetiferTmp != 0x00 && (isdigit(*ptrIndetiferTmp) != 0)) {
				ptrIndetiferTmp++;
			}

			if (*ptrIndetiferTmp == 0x00) {
				*Number = strtol(&IndetiferTmp[0], &endPtr, 10);
				bRet = true;
			}
		}

		if (bRet == true) {
			*CPESTextOut = CPESTextIn;
		}
		else {
			// Something is wrong in identifier name
		}
	}

	return bRet;
}

bool parseValueString(char *CPESTextIn, char **CPESTextOut, std::string &strValue)
{
	bool bRet = false;
	bool bEscapeSequence = false;
	char byChar;
	int iEscapeCharCount;
	char *endPtr;
	DWORD dwConvVal;
	
	strValue.clear();
	// remove any space
	while (*CPESTextIn != 0x00 && isspace(*CPESTextIn) != 0) {
		CPESTextIn++;
	}
	
	if (*CPESTextIn != 0x00 && *CPESTextIn == '"') {
		CPESTextIn++;

		while (*CPESTextIn != 0x00 && (bEscapeSequence == true || *CPESTextIn != '"')) {
			byChar = *CPESTextIn++;
			if (bEscapeSequence == true) {
				iEscapeCharCount++;
				if (iEscapeCharCount == 1) {
					switch (byChar) {
					case 'a' : byChar =  7; bEscapeSequence = false; break;
					case 'b' : byChar =  8; bEscapeSequence = false; break;
					case 'f' : byChar = 12; bEscapeSequence = false; break;
					case 'n' : byChar = 10; bEscapeSequence = false; break;
					case 'r' : byChar = 13; bEscapeSequence = false; break;
					case 't' : byChar =  9; bEscapeSequence = false; break;
					case 'v' : byChar = 11; bEscapeSequence = false; break;
					case '\'' : byChar = '\''; bEscapeSequence = false; break;
					case '"' : byChar = '"'; bEscapeSequence = false; break;
					case '0' : case '1' : case '2' : case '3' : case '4' : case '5' : case '6' : case '7' : break;
					case 'x' : break;
					}
				}
				else {
					switch (*(CPESTextIn - iEscapeCharCount)) {
					case '0' : case '1' : case '2' : case '3' : case '4' : case '5' : case '6' : case '7' :
						if (iEscapeCharCount == 4) {
							dwConvVal = strtoul((CPESTextIn - (iEscapeCharCount-1)), &endPtr, 8);
							byChar = (dwConvVal & 0x000000FF);
							bEscapeSequence = false;
						}
						break;
					case 'x' :
						if (iEscapeCharCount == 3) {
							dwConvVal = strtoul((CPESTextIn - (iEscapeCharCount-1)), &endPtr, 16);
							byChar = (dwConvVal & 0x000000FF);
							bEscapeSequence = false;
						}
						break;
					}
				}
				if (bEscapeSequence == false) {
					strValue += byChar;
				}
			}
			else {
				if (byChar == '\\') {
					bEscapeSequence = true;
					iEscapeCharCount = 0;
				}
				else {
					strValue += byChar;
				}
			}
		}

		if (strValue.empty() == false) {
			bRet = true;
		}
	}

	if (bRet == true) {
		*CPESTextOut = CPESTextIn;
	}
	else {
		// Something is wrong in string
	}

	return bRet;
}

bool parseValueSign(char *CPESTextIn, char **CPESTextOut, eTokenSign *etsType)
{
	bool bRet = false;

	char IndetiferTmp[64], *ptrIndetiferTmp = &IndetiferTmp[0];

	memset(IndetiferTmp, 0, sizeof(IndetiferTmp));
	// remove any space
	while (*CPESTextIn != 0x00 && isspace(*CPESTextIn) != 0) {
		CPESTextIn++;
	}
	while (*CPESTextIn != 0x00 && isspace(*CPESTextIn) == 0 && *CPESTextIn != ';') {
		*ptrIndetiferTmp++ = *CPESTextIn++;
	}

	if (&IndetiferTmp[0] != ptrIndetiferTmp) {
		int iLoop;

		for (iLoop = etsUnknow+1; iLoop < etsMax; iLoop++) {
			if (strcmp(arTokenSigns[iLoop], IndetiferTmp) == 0) {
				break;
			}
		}
		*etsType = (eTokenSign)iLoop;
		if (*etsType == etsMax) {
			*etsType = etsUnknow;
		}

		if (*etsType != etsUnknow) {
			*CPESTextOut = CPESTextIn;
			bRet = true;
		}
		else {
			// Something is wrong in identifier name
		}
	}

	return bRet;
}

bool parseTTVAR(char *CPESTextIn, char **CPESTextOut)
{
	bool bRet = false;
	char *Identifier = NULL;

	if (parseIdentifier(CPESTextIn, CPESTextOut, &Identifier) == true &&
		checkIdentifierExists(Identifier) == false && gmapDefineValues.find(std::string(Identifier)) == gmapDefineValues.end() &&
		**CPESTextOut == ';') {
		DWORD dwStize = gIdentifier.size();
		(*CPESTextOut)++;
		gIdentifier.push_back(Identifier);
		gmapIdentifier.insert(std::map<DWORD, std::string>::value_type(dwStize, std::string(Identifier)));
		bRet = true;
	}

	// PCODE/STACK
	if (bRet == true) {
		if (posPCode == 0) {
			// No pcode, every identifier is a DWORD value on stack
			posIdentifierStack += 4;
		}
		else {
			// No variable declaration once code has begun
			bRet = false;
		}
	}

	return bRet;
}

bool parseTTLET(char *CPESTextIn, char **CPESTextOut)
{
	bool bRet = false;

	char *Identifier = NULL;
	char *IdentifierValue = NULL;
	char *IdentifierValueR = NULL;
	DWORD Number;
	DWORD NumberR;
	eTokenSign etsType;
	bool bAssign;

	bAssign = false;
	bRet = parseIdentifier(CPESTextIn, CPESTextOut, &Identifier);
	if (bRet == true && checkIdentifierExists(Identifier) == true) {
		// Found the identifier
		CPESTextIn = *CPESTextOut;
		if (parseValueSign(CPESTextIn, CPESTextOut, &etsType) == true && etsType == etsEqual) {
			CPESTextIn = *CPESTextOut;
			if (parseValueNumber(CPESTextIn, CPESTextOut, &Number) == true ||
				parseIdentifier(CPESTextIn, CPESTextOut, &IdentifierValue) == true) {
				if (IdentifierValue != NULL) {
					// it's an identifier
					if (checkIdentifierExists(IdentifierValue) == true) {
						bRet = true;
					}
					else {
						if (gmapDefineValues.find(std::string(IdentifierValue)) != gmapDefineValues.end()) {
							Number = gmapDefineValues.find(std::string(IdentifierValue))->second;
							delete IdentifierValue;
							IdentifierValue = NULL;
							bRet = true;
						}
						// IdentifierValue delete when exit;
					}
				}
				else {
					// It's a number
					bRet = true;
				}
				if (bRet == true) {
					// next should be an operator or assignation
					if (**CPESTextOut == ';') {
						// just assignation
						(*CPESTextOut )++;
						bAssign = true;
					}
					else {
						bRet = false;
						CPESTextIn = *CPESTextOut;
						if (parseValueSign(CPESTextIn, CPESTextOut, &etsType) == true &&
							(etsType == etsAdd || etsType == etsSubstract || etsType == etsMultiply || etsType == etsDivide || etsType == etsBitwiseOR || etsType == etsBitwiseAND || etsType == etsBitwiseXOR || etsType == etsBitwiseNOT|| etsType == etsLeftShift || etsType == etsRightShift)) {
							CPESTextIn = *CPESTextOut;
							if (parseValueNumber(CPESTextIn, CPESTextOut, &NumberR) == true ||
								parseIdentifier(CPESTextIn, CPESTextOut, &IdentifierValueR) == true) {
								if (IdentifierValueR != NULL) {
									// it's an identifier
									if (checkIdentifierExists(IdentifierValueR) == true) {
										bRet = true;
									}
									else {
										if (gmapDefineValues.find(std::string(IdentifierValueR)) != gmapDefineValues.end()) {
											NumberR = gmapDefineValues.find(std::string(IdentifierValueR))->second;
											delete IdentifierValueR;
											IdentifierValueR = NULL;
											bRet = true;
										}
										// IdentifierValueR delete when exit;
									}
								}
								else {
									// It's a number
									bRet = true;
								}
								if (bRet == true) {
									if (**CPESTextOut == ';') {
										// Only one operation allowed
										(*CPESTextOut )++;
									}
									else {
										bRet = false;
									}
								}
							}
						}
					}
				}
			}
			else {
				// Delete allocated identifier when exit
			}
		}
	}
	else {
		bRet = false;
	}

	if (bRet == false) {
	}
	else {
		// PCODE/STACK
		// 
		if (IdentifierValue != NULL) {
			// pcsPUSHIdentifier
			pcode[posPCode++] = pcsPUSHIdentifier;
			pcode[posPCode++] = getIdentifierStackIndice(IdentifierValue);
		} else {
			// PUSHNumber
			DWORD *ptrDWORD;
			pcode[posPCode++] = pcsPUSHNumber;
			ptrDWORD = (DWORD *)&pcode[posPCode];
			*ptrDWORD = Number;
			posPCode += 4;
		}
		if (bAssign == false) {
			if (IdentifierValueR != NULL) {
				// pcsPUSHIdentifier
				pcode[posPCode++] = pcsPUSHIdentifier;
				pcode[posPCode++] = getIdentifierStackIndice(IdentifierValueR);
			} else {
				// PUSHNumber
				DWORD *ptrDWORD;
				pcode[posPCode++] = pcsPUSHNumber;
				ptrDWORD = (DWORD *)&pcode[posPCode];
				*ptrDWORD = NumberR;
				posPCode += 4;
			}
			switch (etsType) {
			case etsAdd :
				pcode[posPCode++] = pcsADD;
				break;
			case etsSubstract :
				pcode[posPCode++] = pcsSUB;
				break;
			case etsMultiply :
				pcode[posPCode++] = pcsMUL;
				break;
			case etsDivide :
				pcode[posPCode++] = pcsDIV;
				break;
			case etsBitwiseOR :
				pcode[posPCode++] = pcsBOR;
				break;
			case etsBitwiseAND :
				pcode[posPCode++] = pcsBAND;
				break;
			case etsBitwiseXOR :
				pcode[posPCode++] = pcsBXOR;
				break;
			case etsBitwiseNOT :
				pcode[posPCode++] = pcsBNOT;
				break;
			case etsLeftShift :
				pcode[posPCode++] = pcsLSHFT;
				break;
			case etsRightShift :
				pcode[posPCode++] = pcsRSHFT;
				break;
			}
		}
		// Set the final value to the Identifier
		// pcsSETIdent
		pcode[posPCode++] = pcsSETIdentifier;
		pcode[posPCode++] = getIdentifierStackIndice(Identifier);
		// POP
		if (bAssign == false) {
			// POP + POP
			pcode[posPCode++] = pcsPOPV;
			pcode[posPCode++] = 3;
		}
		else {
			pcode[posPCode++] = pcsPOP;
		}
	}

	if (Identifier != NULL) {
		delete Identifier;
	}
	if (IdentifierValue != NULL) {
		delete IdentifierValue;
	}
	if (IdentifierValueR != NULL) {
		delete IdentifierValueR;
	}

	return bRet;
}

bool checkLabelExists(char *LabelIdentifier)
{
	bool bRet = false;
	std::vector<char *>::iterator it;
	int lenLabelIdentifier = strlen(LabelIdentifier);

	for (it = gLabels.begin(); bRet == false && it != gLabels.end(); it++) {
		if (strlen(*it) == lenLabelIdentifier && memcmp(*it, LabelIdentifier, lenLabelIdentifier) == 0) {
			bRet = true;
		}
	}

	return bRet;
}

bool setLabelRefPCode(char *LabelIdentifier, int posInPCode)
{
	bool bRet = false;
	std::vector<char *>::iterator it;
	int lenLabelIdentifier = strlen(LabelIdentifier);
	std::vector<int>::iterator itR;
	
	for (it = gLabels.begin(), itR = gLabelsRefPCode.begin(); bRet == false && it != gLabels.end(); it++, itR++) {
		if (strlen(*it) == lenLabelIdentifier && memcmp(*it, LabelIdentifier, lenLabelIdentifier) == 0) {
			*itR = posInPCode;
			bRet = true;
		}
	}

	return bRet;
}

bool addRefInPCode(char *LabelIdentifier, int posInPCode)
{
	bool bRet = false;
	std::vector<char *>::iterator it;
	int lenLabelIdentifier = strlen(LabelIdentifier);
	std::vector<std::vector<int>>::iterator itR;
	
	for (it = gLabels.begin(), itR = gLabelsRefInPCode.begin(); bRet == false && it != gLabels.end(); it++, itR++) {
		if (strlen(*it) == lenLabelIdentifier && memcmp(*it, LabelIdentifier, lenLabelIdentifier) == 0) {
			(*itR).push_back(posInPCode);
			bRet = true;
		}
	}

	return bRet;
}

bool parseTTLABEL(char *CPESTextIn, char **CPESTextOut)
{
	bool bRet = false;
	char *Identifier = NULL;

	if (parseIdentifier(CPESTextIn, CPESTextOut, &Identifier) == true && **CPESTextOut == ';') {
		(*CPESTextOut)++;
		if (checkLabelExists(Identifier) == false) {
			std::vector<int> vectTmp;
			gLabels.push_back(Identifier);
			gLabelsRefPCode.push_back(posPCode);
			gLabelsRefInPCode.push_back(vectTmp);
		}
		else {
			bRet = setLabelRefPCode(Identifier, posPCode);
			delete Identifier;
		}
		bRet = true;
	}

	return bRet;
}

bool parseTTGOTO(char *CPESTextIn, char **CPESTextOut)
{
	bool bRet = false;
	char *Identifier = NULL;

	if (parseIdentifier(CPESTextIn, CPESTextOut, &Identifier) == true && **CPESTextOut == ';') {
		DWORD *ptrDWORD;
		(*CPESTextOut)++;

		ptrDWORD = NULL;
		if (checkLabelExists(Identifier) == false) {
			std::vector<int> vectTmp;
			gLabels.push_back(Identifier);
			gLabelsRefPCode.push_back(0xFFFFFFFF);
			gLabelsRefInPCode.push_back(vectTmp);
			ptrDWORD = (DWORD *)0xFFFFFFFF;
		}

		pcode[posPCode++] = pcsJMP;

		bRet = addRefInPCode(Identifier, posPCode);
		if (ptrDWORD == NULL) {
			delete Identifier;
		}
		ptrDWORD = (DWORD *)&pcode[posPCode];
		*ptrDWORD = 0xFFFFFFFF;
		posPCode += 4;
	}

	return bRet;
}

bool parseTTIF(char *CPESTextIn, char **CPESTextOut)
{
	bool bRet = false;

	char *Identifier = NULL;
	char *IdentifierValue = NULL;
	char *IdentifierValueR = NULL;
	DWORD Number;
	DWORD NumberR;
	eTokenSign etsType;

	if (parseValueNumber(CPESTextIn, CPESTextOut, &Number) == true ||
		parseIdentifier(CPESTextIn, CPESTextOut, &IdentifierValue) == true) {
		if (IdentifierValue != NULL) {
			// it's an identifier
			if (checkIdentifierExists(IdentifierValue) == true) {
				bRet = true;
			}
			else {
				if (gmapDefineValues.find(std::string(IdentifierValue)) != gmapDefineValues.end()) {
					Number = gmapDefineValues.find(std::string(IdentifierValue))->second;
					delete IdentifierValue;
					IdentifierValue = NULL;
					bRet = true;
				}
				// IdentifierValue delete when exit
			}
		}
		else {
			// It's a number
			bRet = true;
		}
		if (bRet == true) {
			// next should be an operator
			bRet = false;
			CPESTextIn = *CPESTextOut;
			if (parseValueSign(CPESTextIn, CPESTextOut, &etsType) == true &&
				(etsType == etsEqual || etsType == etsTestOR || etsType == etsTestAND || etsType == etsGreaterThan || etsType == etsLessThan || etsType == etsNotEqual || etsType == etsGreaterThanOrEqual || etsType == etsLessThanOrEqual)) {
				CPESTextIn = *CPESTextOut;
				if (parseValueNumber(CPESTextIn, CPESTextOut, &NumberR) == true ||
					parseIdentifier(CPESTextIn, CPESTextOut, &IdentifierValueR) == true) {
					if (IdentifierValueR != NULL) {
						// it's an identifier
						if (checkIdentifierExists(IdentifierValueR) == true) {
							bRet = true;
						}
						else {
							if (gmapDefineValues.find(std::string(IdentifierValueR)) != gmapDefineValues.end()) {
								NumberR = gmapDefineValues.find(std::string(IdentifierValueR))->second;
								delete IdentifierValueR;
								IdentifierValueR = NULL;
								bRet = true;
							}
							// IdentifierValueR delete when exit;
						}
					}
					else {
						// It's a number
						bRet = true;
					}
					if (bRet == true) {
						CPESTextIn = *CPESTextOut;
						bRet = false;
						if (parseIdentifier(CPESTextIn, CPESTextOut, &Identifier) == true &&
							stricmp(Identifier, "then") == 0) {
							// Only one comparator allowed
							CPESTextIn = *CPESTextOut;
							CPESTextIn--;
							*CPESTextOut = CPESTextIn;
							bRet = true;
						}
						else {
							// Not THEN keyword !
							bRet = false;
						}
					}
				}
			}
		}
	}

	if (bRet == false) {
	}
	else {
		// PCODE/STACK
		// 
		DWORD *ptrDWORD;

		if (IdentifierValue != NULL) {
			// pcsPUSHIdentifier
			pcode[posPCode++] = pcsPUSHIdentifier;
			pcode[posPCode++] = getIdentifierStackIndice(IdentifierValue);
		} else {
			// PUSHNumber
			pcode[posPCode++] = pcsPUSHNumber;
			ptrDWORD = (DWORD *)&pcode[posPCode];
			*ptrDWORD = Number;
			posPCode += 4;
		}
		if (IdentifierValueR != NULL) {
			// pcsPUSHIdentifier
			pcode[posPCode++] = pcsPUSHIdentifier;
			pcode[posPCode++] = getIdentifierStackIndice(IdentifierValueR);
		} else {
			// PUSHNumber
			pcode[posPCode++] = pcsPUSHNumber;
			ptrDWORD = (DWORD *)&pcode[posPCode];
			*ptrDWORD = NumberR;
			posPCode += 4;
		}
		switch (etsType) {
		case etsEqual :
			pcode[posPCode++] = pcsTSTEQ;
			break;
		case etsGreaterThan :
			pcode[posPCode++] = pcsTSTGT;
			break;
		case etsLessThan :
			pcode[posPCode++] = pcsTSTLT;
			break;
		case etsNotEqual :
			pcode[posPCode++] = pcsTSTNEQ;
			break;
		case etsGreaterThanOrEqual :
			pcode[posPCode++] = pcsTSTGTOREQ;
			break;
		case etsLessThanOrEqual :
			pcode[posPCode++] = pcsTSTLTOREQ;
			break;
		case etsTestOR :
			pcode[posPCode++] = pcsTSTBOR;
			break;
		case etsTestAND :
			pcode[posPCode++] = pcsTSTBAND;
			break;
		}

		// POP + POP 
		pcode[posPCode++] = pcsPOPV;
		pcode[posPCode++] = 2;

		// JUMP if TST fail to the ELSE or ENDIF
		pcode[posPCode++] = pcsJMPCOND;

		gIfLabelsPosTST.push_back(posPCode);		// Here
		gIfLabelsPosELSE.push_back(0xFFFFFFFF);		// When else appears 

		ptrDWORD = (DWORD *)&pcode[posPCode];
		*ptrDWORD = 0xFFFFFFFF;	// Will be replaced by the ELSE pos or the ENDIF pos (if ELSE doesn't exists)
		posPCode += 4;

	}

	if (Identifier != NULL) {
		delete Identifier;
	}
	if (IdentifierValue != NULL) {
		delete IdentifierValue;
	}
	if (IdentifierValueR != NULL) {
		delete IdentifierValueR;
	}

	return bRet;
}

bool parseTTELSE(char *CPESTextIn, char **CPESTextOut)
{
	bool bRet = false;

	if (gIfLabelsPosTST.empty() == false) {
		DWORD *ptrDWORD;

		pcode[posPCode++] = pcsJMP;

		gIfLabelsPosELSE.back() = posPCode;		// Here

		ptrDWORD = (DWORD *)&pcode[posPCode];
		*ptrDWORD = 0xFFFFFFFF;	// Will be replaced by the ENDIF pos
		posPCode += 4;

		ptrDWORD = (DWORD *)&pcode[gIfLabelsPosTST.back()];
		*ptrDWORD = posPCode;	// If test fail then jump here(the else statment)

		CPESTextIn--;
		*CPESTextOut = CPESTextIn;

		bRet = true;
	}

	return bRet;
}

bool parseTTENDIF(char *CPESTextIn, char **CPESTextOut)
{
	bool bRet = false;

	if (gIfLabelsPosTST.empty() == false) {
		DWORD *ptrDWORD;

		if (gIfLabelsPosELSE.back() != 0xFFFFFFFF) {
			// ELSE statment exists, replace the jump localisation
			ptrDWORD = (DWORD *)&pcode[gIfLabelsPosELSE.back()];
			*ptrDWORD = posPCode;	// If test fail then jump here(the else statment)
		}
		else {
			// ELSE statment doesn't exists, replace the TST jump localisation
			ptrDWORD = (DWORD *)&pcode[gIfLabelsPosTST.back()];
			*ptrDWORD = posPCode;	// If test fail then jump here(the endif statment)
		}
		gIfLabelsPosTST.pop_back();
		gIfLabelsPosELSE.pop_back();

		CPESTextIn--;
		*CPESTextOut = CPESTextIn;

		bRet = true;
	}

	return bRet;
}

bool parseTTWHILE(char *CPESTextIn, char **CPESTextOut)
{
	bool bRet = false;

	char *Identifier = NULL;
	char *IdentifierValue = NULL;
	char *IdentifierValueR = NULL;
	DWORD Number;
	DWORD NumberR;
	eTokenSign etsType;

	if (parseValueNumber(CPESTextIn, CPESTextOut, &Number) == true ||
		parseIdentifier(CPESTextIn, CPESTextOut, &IdentifierValue) == true) {
		if (IdentifierValue != NULL) {
			// it's an identifier
			if (checkIdentifierExists(IdentifierValue) == true) {
				bRet = true;
			}
			else {
				if (gmapDefineValues.find(std::string(IdentifierValue)) != gmapDefineValues.end()) {
					Number = gmapDefineValues.find(std::string(IdentifierValue))->second;
					delete IdentifierValue;
					IdentifierValue = NULL;
					bRet = true;
				}
				// delete IdentifierValue when exit
			}
		}
		else {
			// It's a number
			bRet = true;
		}
		if (bRet == true) {
			// next should be an operator
			bRet = false;
			CPESTextIn = *CPESTextOut;
			if (parseValueSign(CPESTextIn, CPESTextOut, &etsType) == true &&
				(etsType == etsEqual || etsType == etsTestOR || etsType == etsTestAND || etsType == etsGreaterThan || etsType == etsLessThan || etsType == etsNotEqual || etsType == etsGreaterThanOrEqual || etsType == etsLessThanOrEqual)) {
				CPESTextIn = *CPESTextOut;
				if (parseValueNumber(CPESTextIn, CPESTextOut, &NumberR) == true ||
					parseIdentifier(CPESTextIn, CPESTextOut, &IdentifierValueR) == true) {
					if (IdentifierValueR != NULL) {
						// it's an identifier
						if (checkIdentifierExists(IdentifierValueR) == true) {
							bRet = true;
						}
						else {
							if (gmapDefineValues.find(std::string(IdentifierValueR)) != gmapDefineValues.end()) {
								NumberR = gmapDefineValues.find(std::string(IdentifierValueR))->second;
								delete IdentifierValueR;
								IdentifierValueR = NULL;
								bRet = true;
							}
							// IdentifierValueR delete when exit;
						}
					}
					else {
						// It's a number
						bRet = true;
					}
					if (bRet == true) {
						CPESTextIn = *CPESTextOut;
						bRet = false;
						if (parseIdentifier(CPESTextIn, CPESTextOut, &Identifier) == true &&
							stricmp(Identifier, "do") == 0) {
							// Only one comparator allowed
							CPESTextIn = *CPESTextOut;
							CPESTextIn--;
							*CPESTextOut = CPESTextIn;
							bRet = true;
						}
						else {
							// Not THEN keyword !
							bRet = false;
						}
					}
				}
			}
		}
	}

	if (bRet == false) {
	}
	else {
		// PCODE/STACK
		// 
		DWORD *ptrDWORD;

		gIfLabelsPosWHILETST.push_back(posPCode);		// Here

		if (IdentifierValue != NULL) {
			// pcsPUSHIdentifier
			pcode[posPCode++] = pcsPUSHIdentifier;
			pcode[posPCode++] = getIdentifierStackIndice(IdentifierValue);
		} else {
			// PUSHNumber
			pcode[posPCode++] = pcsPUSHNumber;
			ptrDWORD = (DWORD *)&pcode[posPCode];
			*ptrDWORD = Number;
			posPCode += 4;
		}
		if (IdentifierValueR != NULL) {
			// pcsPUSHIdentifier
			pcode[posPCode++] = pcsPUSHIdentifier;
			pcode[posPCode++] = getIdentifierStackIndice(IdentifierValueR);
		} else {
			// PUSHNumber
			pcode[posPCode++] = pcsPUSHNumber;
			ptrDWORD = (DWORD *)&pcode[posPCode];
			*ptrDWORD = NumberR;
			posPCode += 4;
		}
		switch (etsType) {
		case etsEqual :
			pcode[posPCode++] = pcsTSTEQ;
			break;
		case etsGreaterThan :
			pcode[posPCode++] = pcsTSTGT;
			break;
		case etsLessThan :
			pcode[posPCode++] = pcsTSTLT;
			break;
		case etsNotEqual :
			pcode[posPCode++] = pcsTSTNEQ;
			break;
		case etsGreaterThanOrEqual :
			pcode[posPCode++] = pcsTSTGTOREQ;
			break;
		case etsLessThanOrEqual :
			pcode[posPCode++] = pcsTSTLTOREQ;
			break;
		case etsTestOR :
			pcode[posPCode++] = pcsTSTBOR;
			break;
		case etsTestAND :
			pcode[posPCode++] = pcsTSTBAND;
			break;
		}

		// POP + POP 
		pcode[posPCode++] = pcsPOPV;
		pcode[posPCode++] = 2;

		// JUMP if TST fail to the ELSE or ENDIF
		pcode[posPCode++] = pcsJMPCOND;

		gIfLabelsPosWHILE.push_back(posPCode);		// Here
		ptrDWORD = (DWORD *)&pcode[posPCode];
		*ptrDWORD = 0xFFFFFFFF;	// Will be replaced by the ELSE pos or the ENDIF pos (if ELSE doesn't exists)
		posPCode += 4;

		{
			std::vector<int> vecTmp;
			gIfLabelsPosWHILEBREAK.push_back(vecTmp);
		}
	}

	if (Identifier != NULL) {
		delete Identifier;
	}
	if (IdentifierValue != NULL) {
		delete IdentifierValue;
	}
	if (IdentifierValueR != NULL) {
		delete IdentifierValueR;
	}

	return bRet;
}

bool IsWhileLoopNearestOne()
{
	bool bRet = false;
	int posWhileLoop, posForLoop;

	// CONTINUE can be both encountered in "while" and "for" loop
	// test if we are in a "while" or "for" loop based on the vector stack greter posPCode
	if (gIfLabelsPosWHILETST.empty() == true) {
		posWhileLoop = -1;
	}
	else {
		posWhileLoop = gIfLabelsPosWHILETST.back();
	}
	if (gIfLabelsPosFOR.empty() == true) {
		posForLoop = -1;
	}
	else {
		posForLoop = gIfLabelsPosFOR.back();
	}

	if (posWhileLoop > posForLoop) {
		bRet = true;
	}

	return bRet;
}

bool parseTTCONTINUE(char *CPESTextIn, char **CPESTextOut)
{
	bool bRet = false;

	if (IsWhileLoopNearestOne() == true) {
		// nearest loop is "while" loop
		if (gIfLabelsPosWHILETST.empty() == false) {
			DWORD *ptrDWORD;

			pcode[posPCode++] = pcsJMP;
			ptrDWORD = (DWORD *)&pcode[posPCode];
			*ptrDWORD = gIfLabelsPosWHILETST.back();	// Jump to the while test loop
			posPCode += 4;

			CPESTextIn--;
			*CPESTextOut = CPESTextIn;

			bRet = true;
		}
	}
	else {
		// nearest loop is "for" loop
		if (gIfLabelsPosFOR.empty() == false) {
			DWORD *ptrDWORD;

			pcode[posPCode++] = pcsJMP;
			gIfLabelsPosNEXT.back().push_back(posPCode);
			ptrDWORD = (DWORD *)&pcode[posPCode];
			*ptrDWORD = 0xFFFFFFFF;	// Jump to the next increment loop
			posPCode += 4;

			CPESTextIn--;
			*CPESTextOut = CPESTextIn;

			bRet = true;
		}
	}

	return bRet;
}

bool parseTTBREAK(char *CPESTextIn, char **CPESTextOut)
{
	bool bRet = false;

	if (IsWhileLoopNearestOne() == true) {
		if (gIfLabelsPosWHILETST.empty() == false) {
			DWORD *ptrDWORD;

			pcode[posPCode++] = pcsJMP;

			gIfLabelsPosWHILEBREAK.back().push_back(posPCode);

			ptrDWORD = (DWORD *)&pcode[posPCode];
			*ptrDWORD = 0xFFFFFFFF;	// Jump to the while end loop
			posPCode += 4;

			CPESTextIn--;
			*CPESTextOut = CPESTextIn;

			bRet = true;
		}
	}
	else {
		if (gIfLabelsPosFOR.empty() == false) {
			DWORD *ptrDWORD;

			pcode[posPCode++] = pcsJMP;

			gIfLabelsPosNEXT.back().push_back(posPCode);

			ptrDWORD = (DWORD *)&pcode[posPCode];
			*ptrDWORD = 0xEEEEEEEE;	// Jump to the end of for loop
			posPCode += 4;

			CPESTextIn--;
			*CPESTextOut = CPESTextIn;

			bRet = true;
		}
	}

	return bRet;
}

bool parseTTDONE(char *CPESTextIn, char **CPESTextOut)
{
	bool bRet = false;

	if (gIfLabelsPosWHILETST.empty() == false) {
		DWORD *ptrDWORD;
		std::vector<int>::iterator itBreak;

		pcode[posPCode++] = pcsJMP;
		ptrDWORD = (DWORD *)&pcode[posPCode];
		*ptrDWORD = gIfLabelsPosWHILETST.back();	// Jump to the while test
		posPCode += 4;

		ptrDWORD = (DWORD *)&pcode[gIfLabelsPosWHILE.back()];
		*ptrDWORD = posPCode;	// If while test fail then jump here(the endif statment)

		for (itBreak = gIfLabelsPosWHILEBREAK.back().begin(); itBreak != gIfLabelsPosWHILEBREAK.back().end(); itBreak++) {
			ptrDWORD = (DWORD *)&pcode[*itBreak];
			*ptrDWORD = posPCode;	// break then jump here(the endif statment)
		}

		gIfLabelsPosWHILETST.pop_back();
		gIfLabelsPosWHILE.pop_back();
		gIfLabelsPosWHILEBREAK.pop_back();

		CPESTextIn--;
		*CPESTextOut = CPESTextIn;

		bRet = true;
	}

	return bRet;
}

bool parseTTFOR(char *CPESTextIn, char **CPESTextOut)
{
	bool bRet = false;

	char *Identifier = NULL;
	char *IdentifierTO = NULL;
	char *IdentifierValue = NULL;
	char *IdentifierValueR = NULL;
	char *IdentifierValueS = NULL;
	DWORD Number;
	DWORD NumberR;
	DWORD NumberS = 1;
	eTokenSign etsType;

	if (parseIdentifier(CPESTextIn, CPESTextOut, &Identifier) == true &&
		parseValueSign(*CPESTextOut, CPESTextOut, &etsType) == true &&
		etsType == etsEqual) {

		CPESTextIn = *CPESTextOut;

		if (parseValueNumber(CPESTextIn, CPESTextOut, &Number) == true ||
			parseIdentifier(CPESTextIn, CPESTextOut, &IdentifierValue) == true) {
			if (IdentifierValue != NULL) {
				// it's an identifier
				if (checkIdentifierExists(IdentifierValue) == true) {
					bRet = true;
				}
				else {
					if (gmapDefineValues.find(std::string(IdentifierValue)) != gmapDefineValues.end()) {
						Number = gmapDefineValues.find(std::string(IdentifierValue))->second;
						delete IdentifierValue;
						IdentifierValue = NULL;
						bRet = true;
					}
					// delete IdentifierValue when exit
				}
			}
			else {
				// It's a number
				bRet = true;
			}
			if (bRet == true) {
				CPESTextIn = *CPESTextOut;
				if (parseIdentifier(CPESTextIn, CPESTextOut, &IdentifierTO) == true &&
					stricmp(IdentifierTO, "to") == 0) {
					
					delete IdentifierTO;
					IdentifierTO = NULL;

					CPESTextIn = *CPESTextOut;
					bRet = false;

					if (parseValueNumber(CPESTextIn, CPESTextOut, &NumberR) == true ||
						parseIdentifier(CPESTextIn, CPESTextOut, &IdentifierValueR) == true) {
						if (IdentifierValueR != NULL) {
							// it's an identifier
							if (checkIdentifierExists(IdentifierValueR) == true) {
								bRet = true;
							}
							else {
								if (gmapDefineValues.find(std::string(IdentifierValueR)) != gmapDefineValues.end()) {
									NumberR = gmapDefineValues.find(std::string(IdentifierValueR))->second;
									delete IdentifierValueR;
									IdentifierValueR = NULL;
									bRet = true;
								}
								// IdentifierValueR delete when exit;
							}
						}
						else {
							// It's a number
							bRet = true;
						}

						if (bRet == true) {
							CPESTextIn = *CPESTextOut;
							bRet = false;
							if (parseIdentifier(CPESTextIn, CPESTextOut, &IdentifierTO) == true) {
								if (stricmp(IdentifierTO, "do") == 0) {
									// Only one comparator allowed
									CPESTextIn = *CPESTextOut;
									CPESTextIn--;
									*CPESTextOut = CPESTextIn;
									bRet = true;
								}
								else if (stricmp(IdentifierTO, "step") == 0) {
									
									CPESTextIn = *CPESTextOut;
									bRet = false;
									delete IdentifierTO;
									IdentifierTO = NULL;

									if (parseValueNumber(CPESTextIn, CPESTextOut, &NumberS) == true ||
										parseIdentifier(CPESTextIn, CPESTextOut, &IdentifierValueS) == true) {
										if (IdentifierValueS != NULL) {
											// it's an identifier
											if (checkIdentifierExists(IdentifierValueS) == true) {
												bRet = true;
											}
											else {
												if (gmapDefineValues.find(std::string(IdentifierValueS)) != gmapDefineValues.end()) {
													NumberR = gmapDefineValues.find(std::string(IdentifierValueS))->second;
													delete IdentifierValueS;
													IdentifierValueS = NULL;
													bRet = true;
												}
												// IdentifierValueS delete when exit;
											}
										}
										else {
											// It's a number
											if ((NumberS & 0xF0000000) == 0) {
												bRet = true;
											}
										}
										if (bRet == true) {
											CPESTextIn = *CPESTextOut;
											bRet = false;
											if (parseIdentifier(CPESTextIn, CPESTextOut, &IdentifierTO) == true &&
												stricmp(IdentifierTO, "do") == 0) {
												// Only one comparator allowed
												CPESTextIn = *CPESTextOut;
												CPESTextIn--;
												*CPESTextOut = CPESTextIn;
												bRet = true;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if (bRet == false) {
	}
	else {
		// PCODE/STACK
		// 
		DWORD				*ptrDWORD;
		std::vector<int>	vecTmp;

		// Set the final value to the Identifier
		if (IdentifierValue != NULL) {
			// pcsPUSHIdentifier
			pcode[posPCode++] = pcsPUSHIdentifier;
			pcode[posPCode++] = getIdentifierStackIndice(IdentifierValue);
		} else {
			// PUSHNumber
			pcode[posPCode++] = pcsPUSHNumber;
			ptrDWORD = (DWORD *)&pcode[posPCode];
			*ptrDWORD = Number;
			posPCode += 4;
		}
		// pcsSETIdent
		pcode[posPCode++] = pcsSETIdentifier;
		pcode[posPCode++] = getIdentifierStackIndice(Identifier);
		// POP
		pcode[posPCode++] = pcsPOP;

		gIfLabelsPosFOR.push_back(posPCode);		// Here the test of for loop
		gIfLabelsPosNEXT.push_back(vecTmp);
		if (IdentifierValueS != NULL) {
			// pcsPUSHIdentifier
			gIfLabelsPosSTEPVALUE.push_back(getIdentifierStackIndice(IdentifierValueS) | 0xF0000000);
		} else {
			// PUSHNumber
			gIfLabelsPosSTEPVALUE.push_back(NumberS);
		}


		pcode[posPCode++] = pcsPUSHIdentifier;
		pcode[posPCode++] = getIdentifierStackIndice(Identifier);

		if (IdentifierValueR != NULL) {
			// pcsPUSHIdentifier
			pcode[posPCode++] = pcsPUSHIdentifier;
			pcode[posPCode++] = getIdentifierStackIndice(IdentifierValueR);
		} else {
			// PUSHNumber
			pcode[posPCode++] = pcsPUSHNumber;
			ptrDWORD = (DWORD *)&pcode[posPCode];
			*ptrDWORD = NumberR;
			posPCode += 4;
		}
		pcode[posPCode++] = pcsTSTLTOREQ;

		// POP + POP 
		pcode[posPCode++] = pcsPOPV;
		pcode[posPCode++] = 2;

		// JUMP if TST fail to the end of for loop
		pcode[posPCode++] = pcsJMPCOND;

		gIfLabelsPosNEXT.back().push_back(posPCode);		// Here
		ptrDWORD = (DWORD *)&pcode[posPCode];
		*ptrDWORD = 0xEEEEEEEE;	// jump to the end of for loop
		posPCode += 4;
	}

	if (Identifier != NULL) {
		delete Identifier;
	}
	if (IdentifierTO != NULL) {
		delete IdentifierTO;
	}
	if (IdentifierValue != NULL) {
		delete IdentifierValue;
	}
	if (IdentifierValueR != NULL) {
		delete IdentifierValueR;
	}
	if (IdentifierValueS != NULL) {
		delete IdentifierValueS;
	}

	return bRet;
}

bool parseTTNEXT(char *CPESTextIn, char **CPESTextOut)
{
	bool bRet = false;

	if (gIfLabelsPosFOR.empty() == false) {
		DWORD *ptrDWORD;
		std::vector<int>::iterator itBreak;
		int posPCodeIncrement = posPCode;

		pcode[posPCode++] = pcode[gIfLabelsPosFOR.back()]; // Push identifier
		pcode[posPCode++] = pcode[gIfLabelsPosFOR.back()+1]; // identifier id
		// push increment
		if (gIfLabelsPosSTEPVALUE.back() & 0xF0000000) {
			// pcsPUSHIdentifier
			pcode[posPCode++] = pcsPUSHIdentifier;
			pcode[posPCode++] = (gIfLabelsPosSTEPVALUE.back() & 0x0FFFFFFF);
		} else {
			// PUSHNumber
			pcode[posPCode++] = pcsPUSHNumber;
			ptrDWORD = (DWORD *)&pcode[posPCode];
			*ptrDWORD = gIfLabelsPosSTEPVALUE.back();
			posPCode += 4;
		}
		pcode[posPCode++] = pcsADD;
		// Set the final value to the Identifier
		// pcsSETIdent
		pcode[posPCode++] = pcsSETIdentifier;
		pcode[posPCode++] = pcode[gIfLabelsPosFOR.back()+1]; // identifier id
		// POP
		pcode[posPCode++] = pcsPOPV;
		pcode[posPCode++] = 3;

		pcode[posPCode++] = pcsJMP;
		ptrDWORD = (DWORD *)&pcode[posPCode];
		*ptrDWORD = gIfLabelsPosFOR.back();	// Jump to the for test loop
		posPCode += 4;

		for (itBreak = gIfLabelsPosNEXT.back().begin(); itBreak != gIfLabelsPosNEXT.back().end(); itBreak++) {
			ptrDWORD = (DWORD *)&pcode[*itBreak];
			if (*ptrDWORD == 0xFFFFFFFF) {
				*ptrDWORD = posPCodeIncrement;	// continue
			}
			else if (*ptrDWORD == 0xEEEEEEEE) {
				*ptrDWORD = posPCode;	// break or end for vale then jump here
			}
		}

		gIfLabelsPosFOR.pop_back();
		gIfLabelsPosSTEPVALUE.pop_back();
		gIfLabelsPosNEXT.pop_back();

		CPESTextIn--;
		*CPESTextOut = CPESTextIn;

		bRet = true;
	}

	return bRet;
}

bool parseTTEXIT(char *CPESTextIn, char **CPESTextOut)
{
	bool bRet = true;

	char *Identifier = NULL;
	DWORD Number;

	if (parseValueNumber(CPESTextIn, CPESTextOut, &Number) == true ||
		parseIdentifier(CPESTextIn, CPESTextOut, &Identifier) == true) {
		if (Identifier != NULL) {
			// it's an identifier
			if (checkIdentifierExists(Identifier) == true) {
				bRet = true;
			}
			else {
				if (gmapDefineValues.find(std::string(Identifier)) != gmapDefineValues.end()) {
					Number = gmapDefineValues.find(std::string(Identifier))->second;
					delete Identifier;
					Identifier = NULL;
					bRet = true;
				}
				else {
					delete Identifier;
					Identifier = NULL;
					bRet = false;
				}
			}
		}
		else {
			// It's a number
			bRet = true;
		}

		if (bRet == true) {
			// next should be ';'
			if (**CPESTextOut == ';') {
				(*CPESTextOut )++;
			}
			else {
				bRet = false;
			}
		}
	}
	else {
		bRet = false;
	}

	if (bRet == false) {
	}
	else {
		// PCODE/STACK
		// 
		pcode[posPCode++] = pcsEXIT;
		if (Identifier != NULL) {
			// pcsPUSHIdentifier
			pcode[posPCode++] = pcsPUSHIdentifier;
			pcode[posPCode++] = getIdentifierStackIndice(Identifier);
		} else {
			// PUSHNumber
			DWORD *ptrDWORD;
			pcode[posPCode++] = pcsPUSHNumber;
			ptrDWORD = (DWORD *)&pcode[posPCode];
			*ptrDWORD = Number;
			posPCode += 4;
		}
	}

	if (Identifier != NULL) {
		delete Identifier;
	}

	return bRet;
}

bool parseTTDEFINE(char *CPESTextIn, char **CPESTextOut)
{
	bool bRet = true;

	char *Identifier = NULL;
	DWORD Number;

	if (parseIdentifier(CPESTextIn, CPESTextOut, &Identifier) == true) {
		CPESTextIn = *CPESTextOut;
		if (parseValueNumber(CPESTextIn, CPESTextOut, &Number) == true &&
				checkIdentifierExists(Identifier) == false &&
				gmapDefineValues.find(std::string(Identifier)) == gmapDefineValues.end()) {
			std::string strKey(Identifier);
			gmapDefineValues.insert(std::map<std::string, DWORD>::value_type(strKey, Number));
			bRet = true;
		}
	}
	else {
		bRet = false;
	}

	if (Identifier != NULL) {
		delete Identifier;
	}

	return bRet;
}

bool parseTTPRAGMA(char *CPESTextIn, char **CPESTextOut)
{
	bool bRet = true;

	char *Identifier = NULL;
	std::string strVal;

	if (parseIdentifier(CPESTextIn, CPESTextOut, &Identifier) == true) {
		CPESTextIn = *CPESTextOut;
		if (parseValueString(CPESTextIn, CPESTextOut, strVal) == true &&
				gmapPragmaValues.find(std::string(Identifier)) == gmapPragmaValues.end() &&
				gmapPragmaAllowed.find(std::string(Identifier)) != gmapPragmaAllowed.end()) {
			std::string strKey(Identifier);
			gmapPragmaValues.insert(std::map<std::string, std::string>::value_type(strKey, strVal));
			bRet = true;
		}
	}
	else {
		bRet = false;
	}

	if (Identifier != NULL) {
		delete Identifier;
	}

	return bRet;
}

bool parseFunctionCall(int iFunctionCall, char *Identifier, char *CPESTextIn, char **CPESTextOut)
{
	typedef struct _stIdentOrNumber {
		bool bIdent;
		union {
			char	*Identifier;
			DWORD	Number;
		};
	} stIdentOrNumber;
	bool bRet = false;
	stFunctionDef functionDef = arFunctions[iFunctionCall];
	std::vector<stIdentOrNumber> vectParameters;
	std::vector<stIdentOrNumber>::iterator itVP;
	stIdentOrNumber tmpIorN;
	char *ptrArgdef;

	// Fill the vector ...
	for (ptrArgdef = functionDef.argdef; *ptrArgdef != 0x0; ptrArgdef++) {
		
		tmpIorN.bIdent = false;
		tmpIorN.Identifier = NULL;
		tmpIorN.Number = 0;
		
		bRet = parseValueNumber(CPESTextIn, CPESTextOut, &tmpIorN.Number);
		if (bRet == false) {
			bRet = parseIdentifier(CPESTextIn, CPESTextOut, &tmpIorN.Identifier);
			tmpIorN.bIdent = bRet;
			if (bRet == true &&
					gmapDefineValues.find(std::string(tmpIorN.Identifier)) != gmapDefineValues.end()) {
				DWORD dwDefineValue = gmapDefineValues.find(std::string(tmpIorN.Identifier))->second;
				delete tmpIorN.Identifier;
				tmpIorN.Identifier = NULL; // stupid as it is a union
				tmpIorN.Number = dwDefineValue;
				tmpIorN.bIdent = false;
				bRet = true;
			}

		}
		if (bRet == true) {
			bRet = false;
			if (tmpIorN.bIdent == true) {
				// it's an identifier
				if ((*ptrArgdef == 'I' || *ptrArgdef == 'X' || *ptrArgdef == 'V') &&
					checkIdentifierExists(tmpIorN.Identifier) == true) {
					tmpIorN.bIdent = true;
					bRet = true;
				}
				else {
					delete tmpIorN.Identifier;
				}
			}
			else {
				// It's a number
				if (*ptrArgdef == 'N' || *ptrArgdef == 'X' || *ptrArgdef == 'V') {
					bRet = true;
				}
			}

			if (bRet == true) {
				CPESTextIn = *CPESTextOut;
				vectParameters.push_back(tmpIorN);
				bRet = false;
				if (*ptrArgdef == 'V') {
					if (**CPESTextOut == ';') {
						break;
					}
					else {
						// Variable is never ending :)
						ptrArgdef--;
					}
				}
			}
			else {
				break;
			}
		}
	}

	// Now check if we reach all requested parameters
	if (*ptrArgdef == 0x0 || *ptrArgdef == 'V') {
		// Nice ! now check if ; is next
		if (**CPESTextOut == ';') {
			// Only one operation allowed
			(*CPESTextOut )++;
			bRet = true;
		}
		else {
			bRet = false;
		}
	}

	if (vectParameters.size() == 0 && *functionDef.argdef != 0) {
		// Missing parameter
		bRet = false;
	}

	if (bRet == true) {
		// All is good, now construct pcode
		DWORD *ptrDWORD;
		for (itVP = vectParameters.begin(); itVP != vectParameters.end(); itVP++) {
			if ((*itVP).bIdent == true) {
				// pcsPUSHIdentifier
				pcode[posPCode++] = pcsPUSHIdentifier;
				pcode[posPCode++] = getIdentifierStackIndice((*itVP).Identifier);
			} else {
				// PUSHNumber
				pcode[posPCode++] = pcsPUSHNumber;
				ptrDWORD = (DWORD *)&pcode[posPCode];
				*ptrDWORD = (*itVP).Number;
				posPCode += 4;
			}
		}
		if (*Identifier != 0) {
			// Push number on stack for function result
			pcode[posPCode++] = pcsPUSHNumber;
			ptrDWORD = (DWORD *)&pcode[posPCode];
			*ptrDWORD = 0;
			posPCode += 4;
		}
		// Make function call
		if (*Identifier != 0x0) {
			pcode[posPCode++] = pcsCALL;
		}
		else {
			pcode[posPCode++] = pcsCALLVOID;
		}
		pcode[posPCode++] = iFunctionCall; // No more than 255 functions
		pcode[posPCode++] = vectParameters.size(); // No more than 255 arguments
		// test void call
		if (*Identifier != 0x0) {
			// pcsSETIdent
			pcode[posPCode++] = pcsSETIdentifier;
			pcode[posPCode++] = getIdentifierStackIndice(Identifier);
		}
		// POP (result)
		pcode[posPCode++] = pcsPOPV;
		// POP arguments
		if (*Identifier != 0x0) {
			pcode[posPCode++] = 1 + vectParameters.size();
		}
		else {
			pcode[posPCode++] = vectParameters.size();
		}
		/*
		for (itVP = vectParameters.begin(); itVP != vectParameters.end(); itVP++) {
			pcode[posPCode++] = pcsPOP;
		}
		*/
	}

	// Clean up
	for (itVP = vectParameters.begin(); itVP != vectParameters.end(); itVP++) {
		if ((*itVP).bIdent == true) {
			delete (*itVP).Identifier;
		}
	}

	return bRet;
}

