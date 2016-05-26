#include "assemble.h"

int main(int argc, char **argv) {
    //create a symbol table
    setup();

    FILE *inFile;
    inFile    = fopen(argv[1], "r");
    outFile   = fopen(argv[2], "wb+");
    char lineBuffer[MAX_LINE];

    if (inFile == NULL) {
        exit(EXIT_FAILURE);
    }

    while (fgets(lineBuffer, MAX_LINE, inFile)) {
        if(strlen(lineBuffer) != 1){
            writer(lineBuffer, outFile);
        }
    }

    free(hashMap);

    forwardRefrence();

    fclose(outFile);
    fclose(inFile);

    return 0;
}

void writer(char *str, FILE *outFile) {
    if( strchr(str, ':') == NULL) {
        ++PC;
        char *test = str;
        char *copy = strdup(test);
        Ins type = typeId(strtok_r(copy, " ", &copy));

        switch (type) {
            case 1:
                parseDpi(test, outFile);
                break;
            case 2:
                parseMi(test, outFile);
                break;
            case 3:
                parseSdti(test, outFile);
                break;
            case 4:
                parseBi(test, outFile);
                break;
            case 5:
                parseSi(test, outFile);
                break;
            default:
                //possible extension
                //auto correcting the command
                //if the user input the command that can be corrected
                //then suggest the possible command
                fprintf(stderr, "unidentify command\n");
                exit(1);
        }

    } else {
        char *test = str;
        removeChar(test, ':');
        char *defn = (char *) malloc(sizeof(char));
        sprintf(defn, "%d", PC+1);

        put(test, defn);
    }
}

int getBinaryVal(char *opcode) {
    char *str = lookup(opcode)->defn;
    int q = (int) strtol(str, (char **) NULL, 2);

    return q;
}

void biToBin(struct bi *ins, FILE *file) {
    unsigned char *start;
    start = (unsigned char *) calloc(4, sizeof(unsigned char));

    *start |= (ins->cond) << 4;
    *start |= (ins->id);

    *(start+1) |= (ins->offset) >> 16;

    *(start+2) |= (ins->offset) >> 8;

    *(start+3) |= (ins->offset);

    endianConvert(start);
    fwrite(start, sizeof(unsigned char), 4, file);
    free(start);
}

struct bi *biConvert(char *str) {
    char *test = str;
    struct bi *ins = (struct bi*) malloc (sizeof(struct bi));

    char *opcode, *express;

    opcode = strtok_r(test, " ", &test);

    int opc = getOpVal(opcode);
    ins->cond = opc;
    ins->id = 10;
    express = test;
    switch(isLabelOrAddress(express)) {
        case 1:
            calOffset(ins, express);
            break;
        case 0: //address
            printf("haven't implmented\n");
            //convert address to two's complementary
            break;
        default:
            fprintf(stderr, "not invalid address field\n");
            exit(1);
    }

    return ins;
}

int isLabelOrAddress(char *str) {
    return 1;
}

void calOffset(struct bi *ins, char *label) {
    struct entry *query = (struct entry *) malloc(sizeof(struct entry));
    if(lookup(label) != NULL) {
        //backward reference
        query = lookup(label);
        char *str = query->defn;
        int lineNumber = (int) strtol(str, (char **) NULL, 10);

        int negative = lineNumber - (PC + 2);

        if(negative >= 0) {
            fprintf(stderr, "not a backward reference");
        }
        //TODO the negative value is extremely big execeed the field of 24bits
        ins->offset = negative;
    } else {
        ins->offset = 0;
        struct Fr *fr = (struct Fr *) malloc(sizeof(struct Fr));
        //TODO garbage collection
        char *src = (char *) malloc(sizeof(char));
        strcpy(src, label); //really buggy god gives me intuition
        fr->location = PC;
        fr->label = src;

        lst[numfr] = fr;
        ++numfr;
    }

    free(query);

}

void parseBi(char *lineBuffer, FILE *outFile) {
    struct bi *ins = (struct bi*) malloc(sizeof(struct bi));
    isEnoughSpace(ins);

    ins = biConvert(lineBuffer);
    biToBin(ins, outFile);

    free(ins);
}

void parseSi(char *lineBuffer, FILE *outFile) {
    switch(*lineBuffer) {

        unsigned char *instr;
        case 'a':
            instr = (unsigned char*) calloc(4, sizeof(unsigned char));
            isEnoughSpace(instr);
            fwrite(instr, sizeof(unsigned char), 4, outFile);
            free(instr);
            break;
        case 'l':
            parselsli(lineBuffer, outFile);
            break;
        default:
            fprintf(stderr, "unidentified command\n");
            exit(1);
            break;
    }
}

void parselsli(char *lineBuffer, FILE *outFile) {
    struct lsli *ins = (struct lsli*) malloc(sizeof(struct lsli));
    isEnoughSpace(ins);

    ins = lsliConvert(lineBuffer);
    lsliToBin(ins, outFile);

    free(ins);
}

struct lsli *lsliConvert(char *str) {
    char *test = str;
    char *rn, *expr;
    struct lsli *ins = (struct lsli*) malloc(sizeof(struct lsli));
    isEnoughSpace(ins);

    strtok_r(test, " ", &test);
    rn = strtok_r(test, ",", &test);
    ins->cond = 14;
    ins->iden = 0;
    ins->i = 0;
    ins->opcode = 13;
    ins->s = 0;
    ins->rn = 0;
    int s = getVal(rn);
    ins->rd = s;
    expr = strtok_r(test, " ", &test);

    int exprNum = (*(expr+2) == 'x') ?
        (int) strtol(expr + 1, NULL, 16) : getVal(expr);

    exprNum <<= 7;
    exprNum  |= ins->rd;
    ins->operand = exprNum;

    return ins;
}

void lsliToBin(struct lsli *ins, FILE *file) {
    unsigned char *start;
    start = (unsigned char*) calloc(4, sizeof(unsigned char));
    isEnoughSpace(start);

    *start |= ins->cond << 4;
    *start |= ins->iden << 2;
    *start |= ins->i << 1;
    *start |= ins->opcode >> 3;

    *(start+1) |= (ins->opcode << 1) << 4;
    *(start+1) |= ins->s << 4;
    *(start+1) |= ins->rn;

    *(start+2) |= ins->rd << 4;
    *(start+2) |= ins->operand >> 8;

    *(start+3) |= ins->operand;

    endianConvert(start);
    fwrite(start, sizeof(unsigned char), 4, file);
    free(start);
}

void parseMi(char *lineBuffer, FILE *outFile) {
    struct mi *ins = (struct mi*) malloc(sizeof(struct mi));
    isEnoughSpace(ins);

    ins = miConvert(lineBuffer);
    miToBin(ins, outFile);

    free(ins);
}

struct mi *miConvert(char *str) {
    char *test = str;
    char *opcode, *rd, *rn, *rs, *rm;
    struct mi *ins = (struct mi*) malloc(sizeof(struct mi));
    isEnoughSpace(ins);

    opcode = strtok_r(test, " ", &test);
    ins->cond = 14;
    ins->s = 0;  //TODO such a big bug
    ins->id1 = 0;
    ins->id2 = 9;

    switch(*(opcode+1)) {
        case 'u':
            ins->a = 0;
            ins->rn = 0;
            rd = strtok_r(test, ",", &test);
            rm = strtok_r(test, ",", &test);
            rs = strtok_r(test, ",", &test);
            ins->rd = getVal(rd);
            ins->rm = getVal(rm);
            ins->rs = getVal(rs);

            break;
        case 'l':
            ins->a = 1;
            rd = strtok_r(test, ",", &test);
            rm = strtok_r(test, ",", &test);
            rs = strtok_r(test, ",", &test);
            rn = strtok_r(test, ",", &test);

            ins->rd = getVal(rd);
            ins->rm = getVal(rm);
            ins->rs = getVal(rs);
            ins->rn = getVal(rn);

            break;
        default:
            fprintf(stderr, "Something is wrong\n");
            exit(1);
    }

    return ins;

}

int getVal(char *str) {
    return (int) strtol(str+1, NULL, 10);
}

void miToBin(struct mi *ins, FILE *file) {
    unsigned char *start;
    start = (unsigned char *) calloc(4, sizeof(unsigned char));

    *start |= (ins->cond) << 4;

    *(start+1) |= (ins->a) << 5;
    *(start+1) |= (ins->s) << 4;
    *(start+1) |= ins->rd;

    *(start+2) |= (ins->rn) << 4;
    *(start+2) |= ins->rs;

    *(start+3) |= (ins->id2) << 4;
    *(start+3) |= ins->rm;


    endianConvert(start);
    fwrite(start, sizeof(unsigned char), 4, file);

    free(start);
}

void parseDpi(char *lineBuffer, FILE *outFile) {
    struct dpi *ins = (struct dpi*) malloc(sizeof(struct dpi));
    isEnoughSpace(ins);

    ins = dpiConvert(lineBuffer);
    dpiToBin(ins, outFile);

    free(ins);
}

struct dpi *dpiConvert(char *str) {
    char *test = str;
    char *opcode, *rd, *rn, *op2;
    struct dpi *ins = (struct dpi*) malloc(sizeof(struct dpi));
    isEnoughSpace(ins);

    opcode = strtok_r(test, " ", &test);

    int opc     = getOpVal(opcode);
    ins->opcode = opc;
    ins->s = 0;
    ins->cond = 14;

    removeChar(test, ' ');

    switch (opc) {
        case 13: //mov ->  mov Rd, <Operand2>
            ins->rn = 0;
            rd = strtok_r(test, ",", &test);
            op2 = strtok_r(test, " ", &test);
            setIflagAndOper(ins, op2);
            setRd(ins, rd);

            break;
        case 8: //tst
            ins->rd = 0;
            ins->s = 1;
            rn = strtok_r(test, ",", &test);
            op2 = strtok_r(test, ",", &test);
            setIflagAndOper(ins, op2);
            setRn(ins, rn);
            break;
        case 9: //teq
            ins->rd = 0;
            ins->s  = 1;
            rn = strtok_r(test, ",", &test);
            op2 = strtok_r(test, ",", &test);
            setIflagAndOper(ins, op2);
            setRn(ins, rn);
            break;
        case 10: //cmp
            ins->rd = 0;
            ins->s  = 1;
            rn = strtok_r(test, ",", &test);
            op2 = strtok_r(test, ",", &test);
            setIflagAndOper(ins, op2);
            setRn(ins, rn);
            break;
        default:
            rd = strtok_r(test, ",", &test);
            rn = strtok_r(test, ",", &test);
            setIflagAndOper(ins, test);
            setRn(ins, rn);
            setRd(ins, rd);
            break;
    }

    return ins;
}

int getOpVal(char *opcode) {
    char *str = lookup(opcode)->defn;
    int q = (int) strtol(str, (char **) NULL, 2);

    return q;
}

void setRd(struct dpi *ins, char *str) {
    int s = (int) strtol(str+1, NULL, 10);
    ins-> rd = s;
}

void setRn(struct dpi *ins, char *str) {
    int s = (int) strtol(str+1, NULL, 10);
    ins-> rn = s;
}

int getOffset(char *str) {
    char *reg;
    int ope2;

    reg = strtok_r(str, ",", &str);
    ope2 = getVal(reg);

    int shift;
    if (*str != '\0') {
        shift = evaluateShiftedReg(str);
        shift <<= 4;
        ope2 |= shift;
    }
    return ope2;
}

int getImmOp(int ope2) {
    unsigned int lsBitIndex = 0;
    unsigned int op2 = ope2;
    while ((op2 & 1) != 1) {
        op2 >>= 1;
        lsBitIndex++;
    }
    if (lsBitIndex % 2 != 0) {
        lsBitIndex--;
    }

    unsigned int rotationAmount = (32 - lsBitIndex) >> 1;
    rotationAmount <<= 8;
    unsigned int immValue 
                      = getBits(lsBitIndex + 7, lsBitIndex, ope2);
    ope2 = rotationAmount | immValue;
    return ope2;
}

void setIflagAndOper(struct dpi *ins, char *str) {
    int ope2;
    switch (str[0]) {
        case '#':
            ins->i = 1;

            if(*(str+2) == 'x') {
                ope2 = (int) strtol(str+1, (char **) NULL, 16);
            } else {
                ope2 = (int) strtol(str+1, (char **) NULL, 10);          
            }

            if (ope2 > 255) {
                ope2 = getImmOp(ope2);
            }
            ins->operand = ope2;
            break;
        case 'r':
            ins->i = 0;
            ope2 = getOffset(str);
            ins->operand = ope2;
            break;
        default:
            fprintf(stderr, "no support operand2\n");
            exit(1);
    }
}

int checkLShift(char *type) {
    switch (*(type + 2)) {
        case 'l':
            // lsl: Set bits 6,5 to 00 (0)
            return 0;
        case 'r':
            // lsr: Set bits 6,5 to 01(1)
            return 1;
        default:
            fprintf(stderr, "checkLShift function argument invalid\n");
            exit(1);
        }
}

int checkShiftKind(char *kind) {
    int num;
    switch (*kind) {
       case 'r':
            // It is a register
            // Set bit 4 = 1
            // Set bit 7 = 0
            // Set bits 11,8 to reg number
            num = getVal(kind);
            num <<= 4;
            num |= 1;
            return num;
        case '#':
            // It is an integer
            // Set bit 4 = 0
            // Set bits 11,7 to the value
            num  = (int) strtol(kind + 1, (char **) NULL, 10);
            num <<= 3;
            return num;
        default:
            fprintf(stderr, "checkShiftKind function argument invalid\n");
            exit(1);
        }
}

int checkShiftType(char *type) {
    switch (*type) {
        case 'l':
            // It is a logical shift
            return checkLShift(type);
        case 'a':
            // It is arithmetic right shift
            // set bits 6,5: 10(2)
            return 2;
        case 'r':
            // It is a right rotation
            // Set bits 6,5 to 11(3)
            return 3;
        default:
            fprintf(stderr, "checkShiftType function argument invalid\n");
            exit(1);
        }
}

int evaluateShiftedReg(char *string) {
    char shiftType[4];
    strncpy(shiftType, string, 3);
    
    int type = checkShiftType(shiftType);
    
    string += 3;
    int kind = checkShiftKind(string);
    
    // Shifting type to include it with the kind
    type <<= 1;
    int result;
    result = kind | type;

    return result;
}


void dpiToBin(struct dpi *ins, FILE *file) {
    unsigned char *start;
    start =(unsigned char *) calloc(4, sizeof(unsigned char));
    isEnoughSpace(start);

    *start |= ins -> cond << 4;
    *start |= ins -> iden << 2;
    *start |= ins -> i << 1;
    *start |= ins -> opcode >> 3;

    *(start+1) |= (ins->opcode << 1) << 4;
    *(start+1) |= ins->s << 4;
    *(start+1) |= ins->rn;

    *(start+2) |= ins->rd << 4;
    *(start+2) |= ins->operand >> 8;


    *(start+3) |= ins->operand;

    endianConvert(start);
    fwrite(start, sizeof(unsigned char), 4, file);

    free(start);
}

void removeChar(char *str, char garbage) {

    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) dst++;
    }
    *dst = '\0';
}

int isSquare(char *str) {
    char *src = str;
    for(;*src != '\0';src++) {
        if(*src == ']') {
            return 1;
        }
    }

    return 0;
}

void helpParseRnRmU(struct sdti *ins, char *express) {
    char *test = express;

    switch(*test) {
        case '#' :
            ins->i = 0;
            if(*(test+1) == '-') {
                ins->u = 0;
                ins->offs = getVal(test+1);
            } else {
                ins->u = 1;
                ins->offs = getVal(test);
            }

            break;
        case '-':
            ins->i = 1;
            ins->u = 0;
            //do later for shifting
            ins-> offs = getOffset(test+1);
            break;
        case 'r':
            ins->i = 1;
            //do later for shifting
            ins->u = 1;
            ins->offs = getOffset(test);

            break;
        default:
            fprintf(stderr, "not a shifted register\n");
            exit(1);
    }
}

void parseRnRmU(struct sdti *ins, char *rn, char *express) {

    ins->rn = getVal(rn+1);
    if (express == NULL) {
        ins->p = 1;
        ins->u = 1;
        ins->offs = 0;
        ins->i = 0;
        return;
    }
    
    switch(isSquare(rn)) {
        case 1:
            ins->p = 0;
            helpParseRnRmU(ins, express);
            break;
        case 0:
            ins->p = 1;
            helpParseRnRmU(ins, express);
            break;
        default:
            fprintf(stderr, "problem\n");
            exit(1);
    }
}

void sdtiToBin(struct sdti *ins, FILE *file) {
    unsigned char *start;
    start = (unsigned char *) calloc(4, sizeof(unsigned char));


    *start |= (ins->cond) << 4;
    *start |= (ins->id)   << 2;
    *start |= (ins->i)    << 1;
    *start |= (ins->p);

    *(start+1) |= (ins->u) << 7;
    *(start+1) |= (ins->id2) << 5;
    *(start+1) |= (ins->l) << 4;
    *(start+1) |= (ins->rn);

    *(start+2) |= (ins->rd) << 4;
    *(start+2) |= (ins->offs) >> 8;

    *(start+3) |= ins->offs;

    endianConvert(start);
    fwrite(start, sizeof(unsigned char), 4, file);
    free(start);
}

char *combine(char *ptr1, char *ptr2) {
    char *str;
    str = (char *) malloc(sizeof(char));
    str = strcat(str, "mov ");
    str = strcat(str, ptr1);
    str = strcat(str, ",");
    str = strcat(str, ptr2);
    return str;
}

void ldrExpress(struct sdti *ins, char *str, char *rn) {
    char *s;
    s = "0xFF";
    int compare = (int) strtol(s, NULL, 16);
    int value = (int) strtol(str+1, NULL, 16);

    if(value <= compare) {
        *str = '#';
        char *lineBuffer = combine(rn, str);
        parseDpi(lineBuffer, outFile);
        free(lineBuffer);
        IsError = 1;
    } else {
        ins->i = 0;
        ins->p = 1;
        ins->rn = 15;

    }
}

struct sdti *sdtiConvert(char *str) {
    char *test = str;
    struct sdti *ins = (struct sdti*) malloc(sizeof(struct sdti));

    char *opcode, *rd, *express;
    char *rnOrExp;

    opcode  = strtok_r(test, " ", &test); //str
    removeChar(test, ' ');
    rd      = strtok_r(test, ",", &test); //r0
    rnOrExp = strtok_r(test, ",", &test); //[r1, or [r2]
    express = strtok_r(test, "]", &test); //r4

    ins->cond = 14;
    ins->id = 1;
    ins->id2 = 0;
    ins->rd = getVal(rd);

    switch (*(opcode)) {
        case 'l':
            ins->l = 1;
            switch(*(rnOrExp)) {
                case '=':
                    ldrExpress(ins, rnOrExp, rd);
                    break;
                case '[':
                    parseRnRmU(ins, rnOrExp, express);
                    break;
                default:
                    fprintf(stderr, "not a valid rn");
                    exit(1);
            }
            break;
        case 's':
            ins->l = 0;
            parseRnRmU(ins, rnOrExp, express);
            break;
        default:
            fprintf(stderr, "it's not a sdti ins\n");
            exit(1);
    }

    return ins;
}

void parseSdti(char *lineBuffer, FILE *outFile) {
    struct sdti *ins = (struct sdti*) malloc(sizeof(struct sdti));

    ins = sdtiConvert(lineBuffer);
    if(IsError) {
        IsError = 0;
        return;
    }
    sdtiToBin(ins, outFile);
    free(ins);
}

unsigned int hash(char *s) {
    unsigned int hashval;

    for (hashval = 0; *s != '\0'; s++) {
        hashval = *s + 31 * hashval;
    }

    return hashval % HASH_SIZE;
}

unsigned int hashTwo(char *s) {
    unsigned int hashval;

    for (hashval = 0; *s != '\0'; s++) {
        hashval = *s + 5 * hashval;
    }

    return hashval % NUM_INS;
}

struct elem *find(char *s) {
    struct elem *np;
    for (np = hashMap[hashTwo(s)]; np != NULL; np = np->next) {
        if (strcmp(s, np->name) == 0) {
            return np;
        }
    }

    return NULL;
}

struct elem *install(char *name, Ins defn) {
    struct elem *np;
    unsigned int hashval;

    if ((np = find(name)) == NULL) {
        np = (struct elem *) malloc(sizeof(*np));
        if (np == NULL || (np->name = strdup(name)) == NULL) {
            return NULL;
        }

        hashval = hashTwo(name);
        np->next = hashMap[hashval];
        hashMap[hashval] = np;
    } else {
        free((void *) np->defn);
    }

    np->defn = defn;


    return np;
}

struct entry *lookup(char *s) {
    struct entry *np;
    for (np = hashTable[hash(s)]; np != NULL; np = np->next) {
        if (strcmp(s, np->name) == 0) {
            return np;
        }
    }

    return NULL;
}

struct entry *put(char *name, char *defn) {
    struct entry *np;
    unsigned int hashval;

    if ((np = lookup(name)) == NULL) {
        np = (struct entry *) malloc(sizeof(*np));
        if (np == NULL || (np->name = strdup(name)) == NULL) {
            return NULL;
        }

        hashval = hash(name);
        np->next = hashTable[hashval];
        hashTable[hashval] = np;
    } else {
        free((void *) np->defn);
    }

    if ((np->defn = strdup(defn)) == NULL) {
        return NULL;
    }

    return np;
}

void setup() {
    hashMap = (struct elem **) calloc(NUM_INS, sizeof(struct elem *));

    put("and", "0000");
    put("eor", "0001");
    put("sub", "0010");
    put("rsb", "0011");
    put("add", "0100");
    put("orr", "1100");
    put("mov", "1101");
    put("tst", "1000");
    put("teq", "1001");
    put("cmp", "1010");
    install("add", ADD);
    install("sub", SUB);
    install("rsb",RSB);
    install("and",AND);
    install("eor",EOR);
    install("orr",ORR);
    install("mov",MOV);
    install("tst",TST);
    install("teq",TEQ);
    install("cmp",CMP);
    install("mul",MUL);
    install("mla",MLA);
    install("ldr",LDR);
    install("str",STR);
    install("beq",BEQ);
    install("bne",BNE);
    install("bge",BGE);
    install("blt",BLT);
    install("ble",BLE);
    install("b",B);
    install("bgt",BGT);
    install("lsl",LSL);
    install("andeq",ANDEQ);

    //condition code

    put("beq", "0000");
    put("bne", "0001");
    put("bge", "1010");
    put("blt", "1011");
    put("bgt", "1100");
    put("ble", "1101");
    put("b", "1110");

}

void isEnoughSpace(void *ip) {
    if (ip == NULL) {
        fprintf(stderr, "Out of memory, exiting\n");
        exit(1);
    }
}

void endianConvert(unsigned char *ip) {
    swap(ip, ip + 3);
    swap(ip + 1, ip + 2);
}

void swap(unsigned char *first, unsigned char *end) {
    char temp = *first;
    *first = *end;
    *end   = temp;
}

Ins typeId(char *str) {
    return find(str)->defn;
}

void forwardRefrence(){
    if(numfr) {
        int i;
        for (i = 0; i < numfr; i++) {
            int pcofbranch = lst[i]->location;
            int addr = (int) strtol(lookup(lst[i]->label)->defn, NULL, 10);
            int offset = addr - (pcofbranch + 2);
            overWriteFile(outFile, pcofbranch, &offset);
        }
    }
}

void overWriteFile(FILE *file, int pos, int  *num) {
    fseek(file, (pos - 1) * 4, SEEK_SET);
    fwrite(num, 3, 1, file);
}

int getBits(int leftmost, int rightmost, int num) {
    if (leftmost < rightmost) {
        perror("The arguments to getBits function are invalid");
    }

    int mask = createMask(leftmost, rightmost);  
    num &= mask;
    num >>= rightmost;
    return num;   
}

int createMask(int top, int bot) {
    if (top < bot) {
        perror("The arguments to createMask function are invalid");
    }

    int difference = top - bot;
    int mask = (1 << (difference + 1)) - 1;
    mask <<= bot;
    return mask;
}
