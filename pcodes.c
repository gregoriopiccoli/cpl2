/* Elenco dei PCode con il loro testo, per stamparli o tradurli nell'assmbler 

APPUNTI PER LO SVILUPPO DEI PCODE

FUNC
Il pcode viene dopo la dichiarazione di tipo, quindi deve prendere dalla cima dello stack il suo tipo

CLASS
Segue sempre un pcode INHERTIT con il conteggio delle classi da cui deriva.
0 se non ci sono derivazioni.
Se ci sono derivazioni ci sono i pcode che mettono il riferimento alle classi nello stack e quindi il pcode INHERIT con il numero di classi da considerare.

VAR
trova i tipo nello stack, siccome possono essere dichiarate tante variabili segue la POP che elimina il tipo dallo stack

IF_AND
IF_OR
sono i pcode per la shortcut delle booleane, a differenza delle altre IF non devono fare la POP del valore

IF_TRUE
IF_FALSE
eseguono un salto condizionato, consumano la cima dello stack

ASSERT
ha il numero di cose nello stack (1 o 2), prima il valore di check e in cima l'eventuale messaggio

IMPORT
FROM
nello stack i moduli da importare, hanno il numero di cose lasciate sullo stack
COSI' NON FUNZIONA ...

CALL
il numero dei parametri è value%1000
se il numero è maggior di 1000 ci sono parametri nominati
*/

char* pcodetxt[256];

int initpcodetxt(){
	int i;
	for(i=0;i<256;i++) pcodetxt[i]=(char*)"";
    pcodetxt[0]=(char*)"--";	
    
	// --- operazioni ---
	pcodetxt[P_PLUS]=(char*)"PLUS";       // 1
	pcodetxt[P_MINUS]=(char*)"MINUS";     // 2
	pcodetxt[P_MULT]=(char*)"MULT";       // 3
	pcodetxt[P_DIV]=(char*)"DIV";         // 4
	pcodetxt[P_MOD]=(char*)"MOD";         // 5
	pcodetxt[P_UMINUS]=(char*)"UMINUS";   // 6
	pcodetxt[P_IDIV]=(char*)"IDIV";       // 7

	pcodetxt[P_LT]=(char*)"LT";           //  8
	pcodetxt[P_LE]=(char*)"LE";           //  9
	pcodetxt[P_EQ]=(char*)"EQ";           // 10
	pcodetxt[P_GE]=(char*)"GE";           // 11
	pcodetxt[P_GT]=(char*)"GT";           // 12
	pcodetxt[P_NE]=(char*)"NE";           // 13
	pcodetxt[P_LT_LT]=(char*)"LT_LT";     // 14
	pcodetxt[P_LE_LT]=(char*)"LE_LT";     // 15
	pcodetxt[P_LT_LE]=(char*)"LT_LE";     // 16
	pcodetxt[P_LE_LE]=(char*)"LE_LE";     // 17

	pcodetxt[P_NOT]=(char*)"NOT";         // 18
	pcodetxt[P_AND]=(char*)"AND";         // 19
	pcodetxt[P_OR]=(char*)"OR";           // 20
	pcodetxt[P_IS]=(char*)"IS";           // 71

	// --- costanti ---
	pcodetxt[P_NIL]=(char*)"NIL";         // 21
	pcodetxt[P_TRUE]=(char*)"TRUE";       // 22
	pcodetxt[P_FALSE]=(char*)"FALSE";     // 23
	pcodetxt[P_INT_CONST]=(char*)"INT_CONST";     // 24
	pcodetxt[P_STR_CONST]=(char*)"STR_CONST";     // 25
	pcodetxt[P_FLOAT_CONST]=(char*)"FLOAT_CONST"; // 26
	pcodetxt[P_DATE_CONST]=(char*)"DATE_CONST";   // 27
	pcodetxt[P_DATETIME_CONST]=(char*)"DATETIME_CONST"; // 28
	pcodetxt[P_TIME_CONST]=(char*)"TIME_CONST";   // 29
	pcodetxt[P_ARRAY]=(char*)"ARRAY";     // 30
	pcodetxt[P_DICT]=(char*)"DICT";       // 31
	pcodetxt[P_TUPLE]=(char*)"TUPLE";     // 32

	// --- programma ---
	pcodetxt[P_LINE]=(char*)"LINE";       // 33
	pcodetxt[P_LABEL]=(char*)"LABEL";     // 34
	pcodetxt[P_GOTO]=(char*)"GOTO";       // 35
	pcodetxt[P_IF_FALSE]=(char*)"IF_FALSE"; // 36
	pcodetxt[P_IF_TRUE]=(char*)"IF_TRUE"; // 37
	pcodetxt[P_IF_OR]=(char*)"IF_OR";     // 38
	pcodetxt[P_IF_AND]=(char*)"IF_AND";   // 39
	pcodetxt[P_CALL]=(char*)"CALL";       // 40
	pcodetxt[P_POP]=(char*)"POP";         // 44
	
    // -- dichiarazioni ---	
	pcodetxt[P_FUNC]=(char*)"FUNC";         // 41
	pcodetxt[P_PROC]=(char*)"PROC";         // 42
	pcodetxt[P_CLASS]=(char*)"CLASS";       // 43
	pcodetxt[P_ABSTRACT]=(char*)"ABSTRACT"; // 78
	pcodetxt[P_EVENT]=(char*)"EVENT";       // 85 
	pcodetxt[P_ENDPROC]=(char*)"ENDPROC";   // 86 
	pcodetxt[P_ENDFUNC]=(char*)"ENDFUNC";   // 87
	pcodetxt[P_EMITEVENT]=(char*)"EMITEVENT"; // 88
	pcodetxt[P_ENDCLASS]=(char*)"ENDCLASS"; // 96
    pcodetxt[P_INHERIT]=(char*)"INHERIT";   // 97
    
	// --- tipi di base ---
	pcodetxt[P_INT_TYPE]=(char*)"INT_TYPE";     // 45
	pcodetxt[P_STR_TYPE]=(char*)"STR_TYPE";     // 46
	pcodetxt[P_BOOL_TYPE]=(char*)"BOOL_TYPE";   // 47
	pcodetxt[P_FLOAT_TYPE]=(char*)"FLOAT_TYPE"; // 48
	pcodetxt[P_DATE_TYPE]=(char*)"DATE_TYPE";   // 49
	pcodetxt[P_DATETIME_TYPE]=(char*)"DATETIME_TYPE"; // 50
	pcodetxt[P_TIME_TYPE]=(char*)"TIME_TYPE";   // 51
	pcodetxt[P_ANY_TYPE]=(char*)"ANY_TYPE";     // 52
	pcodetxt[P_ARRAY_TYPE]=(char*)"ARRAY_TYPE"; // 53
	pcodetxt[P_DICT_TYPE]=(char*)"DICT_TYPE";   // 54
	pcodetxt[P_TUPLE_TYPE]=(char*)"TUPLE_TYPE"; // 55
	pcodetxt[P_FUNC_TYPE]=(char*)"FUNC_TYPE";   // 56
	pcodetxt[P_PROC_TYPE]=(char*)"PROC_TYPE";   // 57
	pcodetxt[P_OR_TYPE]=(char*)"OR_TYPE";       // 58

	// --- variabili ---
	pcodetxt[P_VAR]=(char*)"VAR";               // 59
	pcodetxt[P_VAR_STORE]=(char*)"VAR_STORE";   // 60

	pcodetxt[P_LOAD]=(char*)"LOAD";                     // 61
	pcodetxt[P_LOAD_RESULT]=(char*)"LOAD_RESULT";       // 62
	pcodetxt[P_LOAD_INHERITED]=(char*)"LOAD_INHERITED"; // 63
	pcodetxt[P_LOADREF]=(char*)"LOADREF";               // 83
	pcodetxt[P_LOADX]=(char*)"LOADX";                   // 64
	pcodetxt[P_SLICE]=(char*)"SLICE";                   // 65

	pcodetxt[P_STORE]=(char*)"STORE";               // 66
	pcodetxt[P_STORE_RESULT]=(char*)"STORE_RESULT"; // 67
	pcodetxt[P_STOREX]=(char*)"STOREX";             // 68
	pcodetxt[P_STORESLICE]=(char*)"STORESLICE";     // 69
	pcodetxt[P_NAMEDPARM]=(char*)"NAMEDPARM";       // 70
	pcodetxt[P_PARMNAME]=(char*)"PARMNAME";         // 82
    pcodetxt[P_PARM]=(char*)"PARM";                 // 89
    pcodetxt[P_PARM_REF]=(char*)"PARM_REF";         // 90
    pcodetxt[P_PARM_VA]=(char*)"PARM_VA";           // 91
    pcodetxt[P_PARM_VD]=(char*)"PARM_VD";           // 92
    pcodetxt[P_PARM_DEFAULT]=(char*)"PARM_DEFAULT";       // 93
    pcodetxt[P_ENDPARM_DEFAULT]=(char*)"ENDPARM_DEFAULT"; // 95
    pcodetxt[P_ENDPARM]=(char*)"ENDPARM";           // 94
    
	pcodetxt[P_TRY]=(char*)"TRY";         // 72
	pcodetxt[P_CATCH]=(char*)"CATCH";     // 73
	pcodetxt[P_RAISE]=(char*)"RAISE";     // 74
	pcodetxt[P_OLD]=(char*)"OLD";         // 75
	pcodetxt[P_ASSERT]=(char*)"ASSERT";  // 98
	pcodetxt[P_PRE]=(char*)"PRE";        // 99
	pcodetxt[P_POST]=(char*)"POST";      // 100

	pcodetxt[P_PCODEEND]=(char*)"PCODEEND"; // 76

	pcodetxt[P_LOCAL]=(char*)"LOCAL";     // 79
	pcodetxt[P_CONST]=(char*)"CONST";     // 80
	pcodetxt[P_OBSERVER]=(char*)"OBSERVER"; // 81

	pcodetxt[P_PRINT]=(char*)"PRINT";     // 84
	
	pcodetxt[P_COMMENT]=(char*)"#";       // 77     
	
	pcodetxt[P_IMPORT]=(char*)"IMPORT";   // 101
	pcodetxt[P_FROM]=(char*)"FROM";       // 102
	pcodetxt[P_IMPORTID]=(char*)"IMPORTID";   // 103
	pcodetxt[P_IMPORTIDX]=(char*)"IMPORTIDX"; // 104
	
	pcodetxt[P_IMPLEMENTS]=(char*)"IMPLEMENTS"; // 105
    
    return 1;
}

#ifdef __cplusplus
// in c++ inizializza il resro dei pcodes
int __pcoeds_inited=initpcodetxt();
#endif
