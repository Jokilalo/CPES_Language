// CPES_Language.cpp : définit le point d'entrée pour l'application console.
//

#include "stdafx.h"
#include "CPES_Language.h"
#include <vector>
#include <map>
#include "CPES_Lang_Func.h"
#include "CPES_Lang_Exec.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// Seul et unique objet application

CWinApp theApp;

using namespace std;

typedef enum _eTokenType {
	ettUnknow = 0,
	ettREM,
	ettREMSLASH,
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
	ettMax
} eTokenType;

char *arTokenCPES[ettMax] = {
	"",
	"rem",
	"//",
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
	"exit"
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

fnctTokenParse *arTokenParsers[ettMax] = {
	NULL,
	parseTTREM,
	parseTTREM,
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
	parseTTEXIT
};

std::vector<char *> gIdentifier;
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

// Functions arrays
int checkFunctionName(char *tokenString);

bool parseFunctionCall(int iFunctionCall, char *tokenBuff, char *CPESTextIn, char **CPESTextOut);

BYTE stack[4*1024];
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
						pcode[posPCode++] = pcsEXIT;
						pcode[posPCode++] = pcsPUSHNumber; // Wont be pushed, just for exit value
						*((DWORD *)&pcode[posPCode]) = 0;
						posPCode += 4;
						printf("PCODE size : %d\n", posPCode);
						printf("Identifier Stack size : %d\n", posIdentifierStack);
						printf("----------\n");
						pCodeExecute(pcode, sizeof(pcode), identifierStack, sizeof(identifierStack), stack, sizeof(stack));
					}
				}
			}

			for (it = gIdentifier.begin(); it != gIdentifier.end(); it++) {
				delete *it;
			}
		}
	}

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

bool ParseCPESTexte(char *CPESText)
{
	bool bRet = true;
	char *parsePos = CPESText;
	char tokenBuff[64], *ptrToken = tokenBuff, *ptrTokenBegin = NULL;
	eTokenType ettRunning = ettUnknow;
	bool bEnteredQuote = false;
	std::vector<char *> vecParameters;

	memset(tokenBuff, 0, sizeof(tokenBuff));

	while (bRet == true && *parsePos != 0x00) {
		if (ettRunning == ettUnknow) {
			if (isspace(*parsePos) == 0) {
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
						}
					}

					bRet = false;
					if (iFunctionCall == -1) {
						if (arTokenParsers[ettRunning] != NULL) {
							// call specialized parser
							bRet = arTokenParsers[ettRunning](parsePos, &parsePos);
						}
					}
					else {
						bRet = parseFunctionCall(iFunctionCall, tokenBuff, parsePos, &parsePos);
					}

					if (bRet == false) {
						printf("Error near : %s position %d\n", tokenBuff, parsePos-CPESText);
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
				*Number = strtol(&IndetiferTmp[2], &endPtr, 16);
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
				*Number = strtol(&IndetiferTmp[2], &endPtr, 2);
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
		checkIdentifierExists(Identifier) == false &&
		**CPESTextOut == ';') {
		(*CPESTextOut)++;
		gIdentifier.push_back(Identifier);
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
						delete IdentifierValue;
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
		pcode[posPCode++] = pcsPOP;
		if (bAssign == false) {
			// POP + POP
			pcode[posPCode++] = pcsPOP;
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
				delete IdentifierValue;
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
		pcode[posPCode++] = pcsPOP;
		pcode[posPCode++] = pcsPOP;

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
				delete IdentifierValue;
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
		pcode[posPCode++] = pcsPOP;
		pcode[posPCode++] = pcsPOP;

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
					delete IdentifierValue;
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
		pcode[posPCode++] = pcsPOP;
		pcode[posPCode++] = pcsPOP;

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
		pcode[posPCode++] = pcsPOP;
		pcode[posPCode++] = pcsPOP;
		pcode[posPCode++] = pcsPOP;

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
				delete Identifier;
				Identifier = NULL;
				bRet = false;
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
		// Push number on stack for function result
		pcode[posPCode++] = pcsPUSHNumber;
		ptrDWORD = (DWORD *)&pcode[posPCode];
		*ptrDWORD = 0;
		posPCode += 4;
		// Make function call
		pcode[posPCode++] = pcsCALL;
		pcode[posPCode++] = iFunctionCall; // No more than 255 functions
		pcode[posPCode++] = vectParameters.size(); // No more than 255 arguments
		// test void call
		if (*Identifier != 0x0) {
			// pcsSETIdent
			pcode[posPCode++] = pcsSETIdentifier;
			pcode[posPCode++] = getIdentifierStackIndice(Identifier);
		}
		// POP (result)
		pcode[posPCode++] = pcsPOP;
		// POP argumets
		for (itVP = vectParameters.begin(); itVP != vectParameters.end(); itVP++) {
			pcode[posPCode++] = pcsPOP;
		}
	}

	// Clean up
	for (itVP = vectParameters.begin(); itVP != vectParameters.end(); itVP++) {
		if ((*itVP).bIdent == true) {
			delete (*itVP).Identifier;
		}
	}

	return bRet;
}

