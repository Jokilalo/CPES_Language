#include "CPES_Type.h"

#ifndef _CPES_PCOD_VERSION
#define _CPES_PCOD_VERSION 0x00010000
#endif

// Code Bytes 
typedef enum _ePCodeSymbols {
 pcsNOP,
 pcsPUSHNumber,
 pcsPUSHIdentifier,
 pcsPOP,
 pcsPOPV,
 pcsSETIdentifier,
 pcsADD,
 pcsSUB,
 pcsMUL,
 pcsDIV,
 pcsBOR,
 pcsBAND,
 pcsBXOR,
 pcsBNOT,
 pcsLSHFT,
 pcsRSHFT,
 pcsJMP,
 pcsJMPCOND,
 pcsTSTEQ,
 pcsTSTNEQ,
 pcsTSTGT,
 pcsTSTLT,
 pcsTSTBOR,
 pcsTSTBAND,
 pcsTSTGTOREQ,
 pcsTSTLTOREQ,
 pcsCALL,
 pcsCALLVOID,
 pcsEXIT
} ePCodeSymbols;

DWORD pCodeExecute(const BYTE *ptrPCode, int PCodeLen, BYTE *ptrStackIdentifier, int StackIdentifierLen, BYTE *ptrStack, int StackLen);
