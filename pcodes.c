char* pcodetxt[256];

void initpcodetxt(){
// --- operazioni ---
pcodetxt[1]="PLUS"; //         1
pcodetxt[2]="MINUS"; //        2
pcodetxt[3]="MULT"; //         3
pcodetxt[4]="DIV"; //          4
pcodetxt[5]="MOD"; //          5
pcodetxt[6]="UMINUS"; //       6
pcodetxt[7]="IDIV"; //         7

pcodetxt[8]="LT"; //           8
pcodetxt[9]="LE"; //           9
pcodetxt[10]="EQ"; //          10
pcodetxt[11]="GE"; //          11
pcodetxt[12]="GT"; //          12
pcodetxt[13]="NE"; //          13
pcodetxt[14]="LT_LT"; //       14
pcodetxt[15]="LE_LT"; //       15
pcodetxt[16]="LT_LE"; //       16
pcodetxt[17]="LE_LE"; //       17

pcodetxt[18]="NOT"; //         18
pcodetxt[19]="AND"; //         19
pcodetxt[20]="OR"; //          20
pcodetxt[71]="IS"; //          71

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

// --- last 82
}
