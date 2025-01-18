char* pcodetxt[256];

void initpcodetxt(){
	int i;
	for(i=0;i<256;i++) pcodetxt[i]="";
    pcodetxt[0]="--";	
    
	// --- operazioni ---
	pcodetxt[P_PLUS]="PLUS";       // 1
	pcodetxt[P_MINUS]="MINUS";     // 2
	pcodetxt[P_MULT]="MULT";       // 3
	pcodetxt[P_DIV]="DIV";         // 4
	pcodetxt[P_MOD]="MOD";         // 5
	pcodetxt[P_UMINUS]="UMINUS";   // 6
	pcodetxt[P_IDIV]="IDIV";       // 7

	pcodetxt[P_LT]="LT";           //  8
	pcodetxt[P_LE]="LE";           //  9
	pcodetxt[P_EQ]="EQ";           // 10
	pcodetxt[P_GE]="GE";           // 11
	pcodetxt[P_GT]="GT";           // 12
	pcodetxt[P_NE]="NE";           // 13
	pcodetxt[P_LT_LT]="LT_LT";     // 14
	pcodetxt[P_LE_LT]="LE_LT";     // 15
	pcodetxt[P_LT_LE]="LT_LE";     // 16
	pcodetxt[P_LE_LE]="LE_LE";     // 17

	pcodetxt[P_NOT]="NOT";         // 18
	pcodetxt[P_AND]="AND";         // 19
	pcodetxt[P_OR]="OR";           // 20
	pcodetxt[P_IS]="IS";           // 71

	// --- costanti ---
	pcodetxt[P_NIL]="NIL";         // 21
	pcodetxt[P_TRUE]="TRUE";       // 22
	pcodetxt[P_FALSE]="FALSE";     // 23
	pcodetxt[P_INT_CONST]="INT_CONST";     // 24
	pcodetxt[P_STR_CONST]="STR_CONST";     // 25
	pcodetxt[P_FLOAT_CONST]="FLOAT_CONST"; // 26
	pcodetxt[P_DATE_CONST]="DATE_CONST";   // 27
	pcodetxt[P_DATETIME_CONST]="DATETIME_CONST"; // 28
	pcodetxt[P_TIME_CONST]="TIME_CONST";   // 29
	pcodetxt[P_ARRAY]="ARRAY";     // 30
	pcodetxt[P_DICT]="DICT";       // 31
	pcodetxt[P_TUPLE]="TUPLE";     // 32

	// -- programma --
	pcodetxt[P_LINE]="LINE";       // 33
	pcodetxt[P_LABEL]="LABEL";     // 34
	pcodetxt[P_GOTO]="GOTO";       // 35
	pcodetxt[P_IF_FALSE]="IF_FALSE"; // 36
	pcodetxt[P_IF_TRUE]="IF_TRUE"; // 37
	pcodetxt[P_IF_OR]="IF_OR";     // 38
	pcodetxt[P_IF_AND]="IF_AND";   // 39
	pcodetxt[P_CALL]="CALL";       // 40
	pcodetxt[P_FUNC]="FUNC";       // 41
	pcodetxt[P_PROC]="PROC";       // 42
	pcodetxt[P_CLASS]="CLASS";     // 43
	pcodetxt[P_POP]="POP";         // 44
	pcodetxt[P_ABSTRACT]="ABSTRACT"; // 78
	pcodetxt[P_EVENT]="EVENT";       // 85 
	pcodetxt[P_ENDPROC]="ENDPROC";   // 86 
	pcodetxt[P_ENDFUNC]="ENDFUNC";   // 87
	pcodetxt[P_EMITEVENT]="EMITEVENT"; // 88

	// --- tipi di base ---
	pcodetxt[P_INT_TYPE]="INT_TYPE";     // 45
	pcodetxt[P_STR_TYPE]="STR_TYPE";     // 46
	pcodetxt[P_BOOL_TYPE]="BOOL_TYPE";   // 47
	pcodetxt[P_FLOAT_TYPE]="FLOAT_TYPE"; // 48
	pcodetxt[P_DATE_TYPE]="DATE_TYPE";   // 49
	pcodetxt[P_DATETIME_TYPE]="DATETIME_TYPE"; // 50
	pcodetxt[P_TIME_TYPE]="TIME_TYPE";   // 51
	pcodetxt[P_ANY_TYPE]="ANY_TYPE";     // 52
	pcodetxt[P_ARRAY_TYPE]="ARRAY_TYPE"; // 53
	pcodetxt[P_DICT_TYPE]="DICT_TYPE";   // 54
	pcodetxt[P_TUPLE_TYPE]="TUPLE_TYPE"; // 55
	pcodetxt[P_FUNC_TYPE]="FUNC_TYPE";   // 56
	pcodetxt[P_PROC_TYPE]="PROC_TYPE";   // 57
	pcodetxt[P_OR_TYPE]="OR_TYPE";       // 58

	// --- variabili ---
	pcodetxt[P_VAR]="VAR";               // 59
	pcodetxt[P_VAR_STORE]="VAR_STORE";   // 60

	pcodetxt[P_LOAD]="LOAD";                     // 61
	pcodetxt[P_LOAD_RESULT]="LOAD_RESULT";       // 62
	pcodetxt[P_LOAD_INHERITED]="LOAD_INHERITED"; // 63
	pcodetxt[P_LOADREF]="LOADREF";               // 83
	pcodetxt[P_LOADX]="LOADX";                   // 64
	pcodetxt[P_SLICE]="SLICE";                   // 65

	pcodetxt[P_STORE]="STORE";               // 66
	pcodetxt[P_STORE_RESULT]="STORE_RESULT"; // 67
	pcodetxt[P_STOREX]="STOREX";             // 68
	pcodetxt[P_STORESLICE]="STORESLICE";     // 69
	pcodetxt[P_NAMEDPARM]="NAMEDPARM";       // 70
	pcodetxt[P_PARMNAME]="PARMNAME";         // 82
    pcodetxt[P_PARM]="PARM";                 // 89
    pcodetxt[P_PARM_REF]="PARM_REF";         // 90
    pcodetxt[P_PARM_VA]="PARM_VA";           // 91
    pcodetxt[P_PARM_VD]="PARM_VD";           // 92
    pcodetxt[P_PARM_STORE]="PARM_STORE";     // 93
    pcodetxt[P_ENDPARM]="ENDPARM";           // 94
    
	pcodetxt[P_TRY]="TRY";         // 72
	pcodetxt[P_CATCH]="CATCH";     // 73
	pcodetxt[P_RAISE]="RAISE";     // 74
	pcodetxt[P_OLD]="OLD";         // 75

	pcodetxt[P_PCODEEND]="PCODEEND"; // 76

	pcodetxt[P_LOCAL]="LOCAL";     // 79
	pcodetxt[P_CONST]="CONST";     // 80
	pcodetxt[P_OBSERVER]="OBSERVER"; // 81

	pcodetxt[P_PRINT]="PRINT";     // 84
	
	pcodetxt[P_COMMENT]="#";       // 77     

}
