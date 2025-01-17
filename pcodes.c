char* pcodetxt[256];

void initpcodetxt(){
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
pcodetxt[21]="NIL"; //         21
pcodetxt[22]="TRUE"; //        22
pcodetxt[23]="FALSE"; //       23
pcodetxt[24]="INT_CONST"; //      24
pcodetxt[25]="STR_CONST"; //      25
pcodetxt[26]="FLOAT_CONST"; //    26
pcodetxt[27]="DATE_CONST"; //     27
pcodetxt[28]="DATETIME_CONST"; // 28
pcodetxt[29]="TIME_CONST"; //     29
pcodetxt[30]="ARRAY"; //          30
pcodetxt[31]="DICT"; //           31
pcodetxt[32]="TUPLE"; //          32

// -- programma --
pcodetxt[33]="LINE"; //        33
pcodetxt[34]="LABEL"; //       34
pcodetxt[35]="GOTO"; //        35
pcodetxt[36]="IF_FALSE"; //    36
pcodetxt[37]="IF_TRUE"; //     37
pcodetxt[38]="IF_OR"; //       38
pcodetxt[39]="IF_AND"; //      39
pcodetxt[40]="CALL"; //        40
pcodetxt[41]="FUNC"; //        41
pcodetxt[42]="PROC"; //        42
pcodetxt[43]="CLASS"; //       43
pcodetxt[44]="POP"; //         44

// --- tipi di base ---
pcodetxt[45]="INT_TYPE"; //    45
pcodetxt[46]="STR_TYPE"; //    46
pcodetxt[47]="BOOL_TYPE"; //   47
pcodetxt[48]="FLOAT_TYPE"; //  48
pcodetxt[49]="DATE_TYPE"; //   49
pcodetxt[50]="DATETIME_TYPE"; // 50
pcodetxt[51]="TIME_TYPE"; //   51
pcodetxt[52]="ANY_TYPE"; //    52
pcodetxt[53]="ARRAY_TYPE"; //  53
pcodetxt[54]="DICT_TYPE"; //   54
pcodetxt[55]="TUPLE_TYPE"; //  55
pcodetxt[56]="FUNC_TYPE"; //   56
pcodetxt[57]="PROC_TYPE"; //   57
pcodetxt[58]="OR_TYPE"; //     58

// --- variabili ---
pcodetxt[59]="VAR"; //         59
pcodetxt[60]="VAR_STORE"; //   60

pcodetxt[61]="LOAD"; //        61
pcodetxt[62]="LOAD_RESULT"; // 62
pcodetxt[63]="LOAD_INHERITED"; // 63
pcodetxt[83]="LOADREF";       // 83
pcodetxt[64]="LOADX"; //       64
pcodetxt[65]="SLICE"; //       65

pcodetxt[66]="STORE"; //       66
pcodetxt[67]="STORE_RESULT"; // 67
pcodetxt[68]="STOREX"; //      68
pcodetxt[69]="STORESLICE"; //  69
pcodetxt[70]="NAMEDPARM"; //   70
pcodetxt[82]="PARMNAME"; //    82

pcodetxt[72]="TRY"; //         72
pcodetxt[73]="CATCH"; //       73
pcodetxt[74]="RAISE"; //       74
pcodetxt[75]="OLD"; //         75

pcodetxt[76]="OK"; //          76
pcodetxt[77]="ERROR"; //       77
pcodetxt[78]="VERSION"; //     78

pcodetxt[79]="LOCAL"; //       79
pcodetxt[80]="CONST"; //       80
pcodetxt[81]="OBSERVER"; //    81

pcodetxt[84]="PRINT"; //    84

// --- last 84
}
