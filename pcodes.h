// --- operazioni ---
#define P_PLUS         1
#define P_MINUS        2
#define P_MULT         3
#define P_DIV          4
#define P_MOD          5
#define P_UMINUS       6
#define P_IDIV         7

#define P_LT           8
#define P_LE           9
#define P_EQ          10
#define P_GE          11
#define P_GT          12
#define P_NE          13
#define P_LT_LT       14
#define P_LE_LT       15
#define P_LT_LE       16
#define P_LE_LE       17

#define P_NOT         18
#define P_AND         19
#define P_OR          20
#define P_IS          71

// --- costanti ---
#define P_NIL         21
#define P_TRUE        22
#define P_FALSE       23
#define P_INT_CONST      24
#define P_STR_CONST      25
#define P_FLOAT_CONST    26
#define P_DATE_CONST     27
#define P_DATETIME_CONST 28
#define P_TIME_CONST     29
#define P_ARRAY          30
#define P_DICT           31
#define P_TUPLE          32

// --- programma ---
#define P_LINE        33
#define P_LABEL       34
#define P_GOTO        35
#define P_IF_FALSE    36
#define P_IF_TRUE     37
#define P_IF_OR       38
#define P_IF_AND      39
#define P_CALL        40
#define P_POP         44

// -- dichiarazioni ---
#define P_FUNC        41
#define P_PROC        42
#define P_CLASS       43
#define P_ABSTRACT    85
#define P_EVENT       78
#define P_ENDPROC     86
#define P_ENDFUNC     87
#define P_EMITEVENT   88
#define P_ENDCLASS    96
#define P_INHERIT     97

// --- tipi di base ---
#define P_INT_TYPE    45
#define P_STR_TYPE    46
#define P_BOOL_TYPE   47
#define P_FLOAT_TYPE  48
#define P_DATE_TYPE   49
#define P_DATETIME_TYPE 50
#define P_TIME_TYPE   51
#define P_ANY_TYPE    52
#define P_ARRAY_TYPE  53
#define P_DICT_TYPE   54
#define P_TUPLE_TYPE  55
#define P_FUNC_TYPE   56
#define P_PROC_TYPE   57
#define P_OR_TYPE     58

// --- variabili ---
#define P_VAR         59
#define P_VAR_STORE   60

#define P_LOAD        61
#define P_LOAD_RESULT 62
#define P_LOAD_INHERITED 63
#define P_LOADX       64
#define P_LOADREF     83
#define P_SLICE       65

#define P_STORE       66
#define P_STORE_RESULT 67
#define P_STOREX      68
#define P_STORESLICE  69
#define P_NAMEDPARM   70
#define P_PARMNAME    82
#define P_PARM        89
#define P_PARM_REF    90
#define P_PARM_VA     91
#define P_PARM_VD     92
#define P_PARM_DEFAULT  93
#define P_ENDPARM_DEFAULT  95
#define P_ENDPARM     94

#define P_TRY         72
#define P_CATCH       73
#define P_RAISE       74
#define P_OLD         75

#define P_PCODEEND    76

#define P_LOCAL       79
#define P_CONST       80
#define P_OBSERVER    81

#define P_PRINT       84
#define P_COMMENT     77

#define MAX_PCODE     97  // ultimo codice usato da un pcode

// --- unused: 
