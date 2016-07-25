// Code Bytes
typedef enum _ePCodeSymbols {
 pcsNOP,
 pcsPUSHNumber,
 pcsPUSHIdentifier,
 pcsPOP,
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
 pcsEXIT
} ePCodeSymbols;

DWORD pCodeExecute(BYTE *ptrPCode, int PCodeLen, BYTE *ptrStackIdentifier, int StackIdentifierLen, BYTE *ptrStack, int StackLen);
