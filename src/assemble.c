#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define MAX_LINE  32
#define HASH_SIZE 101
#define NUM_INS   101

typedef enum {
    ADD=1,SUB=1,RSB=1,AND=1,EOR=1,ORR=1,MOV=1,TST=1,TEQ=1,CMP=1,
    MUL=2,MLA=2,LDR=3,STR=3,BEQ=4,BNE=4,BGE=4,BLT=4,BGT=4,
    BLE=4,B  =4,LSL=5,ANDEQ=5
} Ins;

struct elem {
    struct elem *next;
    char *name;
    Ins defn;
};

static struct elem *hashMap[NUM_INS];
//create hash table and entry for hashtable
static struct entry *hashTable[HASH_SIZE];

struct entry {
    struct entry *next;
    char *name;
    char *defn;
};

//data processing instruction format
struct dpi {
    unsigned int cond    : 4;
    unsigned int iden    : 2;
    unsigned int i       : 1;
    unsigned int opcode  : 4;
    unsigned int s       : 1;
    unsigned int rn      : 4;
    unsigned int rd      : 4;
    unsigned int operand : 12;
};

//multiply instruction format
struct mi {
    unsigned int cond : 4;
    unsigned int id1  : 6;
    unsigned int a    : 1;
    unsigned int s    : 1;
    unsigned int rd   : 4;
    unsigned int rn   : 4;
    unsigned int rs   : 4;
    unsigned int id2  : 4;
    unsigned int rm   : 4;
};

struct ani {
    unsigned int instr : 32;
};

struct lsli {
    unsigned int cond    : 4;
    unsigned int iden    : 2;
    unsigned int i       : 1;
    unsigned int opcode  : 4;
    unsigned int s       : 1;
    unsigned int rn      : 4;
    unsigned int rd      : 4;
    unsigned int operand : 12;
};

//grand design
void writer(char *, FILE *);
//parsing data processing instructions
void parseDpi(char *, FILE *);

void dpiToBin(struct dpi *, FILE *);
struct dpi *dpiConvert(char *);
int  getOpVal(char *);
void setIflagAndOper(struct dpi *, char *);
void setRd(struct dpi *, char *);
void setRn(struct dpi *, char *);
//end

//parsing mulitple instructions
void parseMi(char *, FILE *);

void miToBin(struct mi *, FILE *);
struct mi *miConvert(char *);
int getVal(char *);
//end

//parsing special instructions
void parseSi(char *, FILE *);
void parselslI(char *, FILE *); 
struct lsli *lsliConvert(char *);
void lsliToBin(struct lsli *, FILE *);
//end

//API for hashtable
unsigned int hash(char *);
unsigned int hashTwo(char *);
struct entry *lookup(char *);
struct entry *put(char *, char *);
struct elem *find(char *);
struct elem *install(char *, Ins);
void setup();
Ins typeId(char *);

//utility functions
void isEnoughSpace(void *);
void endianConvert(unsigned char *);
void swap(unsigned char *, unsigned char *);

int main(int argc, char **argv) {
    //create a symbol table
    setup();

    FILE *inFile;
    FILE *outFile;
    inFile    = fopen(argv[1], "r");
    outFile   = fopen(argv[2], "wb");
    char lineBuffer[MAX_LINE];

    if (inFile == NULL) {
        exit(EXIT_FAILURE);
    }

    while (fgets(lineBuffer, MAX_LINE, inFile)) {
        writer(lineBuffer, outFile);
    }

    fclose(outFile);
    fclose(inFile);

    return 0;
}

void writer(char *str, FILE *outFile) {
    if( strchr(str, ':') == NULL) {
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
                //TODO single data transfer
                break;
            case 4:
                //TODO branch instruction
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
                break;
        }

    } else {
        //TODO deal with label
    }
}

void parseSi(char *lineBuffer, FILE *outFile) {
    switch(*lineBuffer) {
        unsigned char *zeroes;
        case 'a': 
            zeroes = calloc(8, sizeof(unsigned char));
            fwrite(zeroes, sizeof(unsigned char), sizeof(int), outFile);
            free(zeroes);  
            break;
        case 'l': 
            parselslI(lineBuffer, outFile);
            break;
        default:
            fprintf(stderr, "unidentified command\n");
            exit(1);
            break;   
    }
}

void parselslI(char *lineBuffer, FILE *outFile) { 
    struct lsli *ins = (struct lsli*) malloc(sizeof(struct lsli));
    isEnoughSpace(ins);

    ins = lsliConvert(lineBuffer);
    lsliToBin(ins, outFile);

    free(ins);  
}

struct lsli *lsliConvert(char *str) {
    char *test = str; 
    char *opcode, *rd, *rn, *expr;
    struct lsli *ins = (struct lsli*) malloc(sizeof(struct lsli));
    isEnoughSpace(ins);

    opcode = "mov";
    strtok_r(test, " ", &test);
    rd = strtok_r(test, ",", &test); 
    ins->cond = (int) strtol("1101", NULL, 10); 
    ins->iden = (int) strtol("00", NULL, 10);
    ins->i = (int) strtol("0", NULL, 10);
    ins->opcode = (int) strtol("1101", NULL, 10); 
    ins->s = (int) strtol("0", NULL, 10);
    ins->rn = (int) strtol("0", NULL, 10);
    int s = (int) strtol(rd+1, NULL, 10);
    ins-> rd = s;   
  
    expr = strtok_r(test, " ", &test);
    
    switch(*(expr + 2)) {
        int exprNum; 
        case 'x' : 
            exprNum   = (int) strtol(expr + 1, NULL, 16);   
            exprNum <<= 7;
            exprNum  |= ins->rn;
            ins->operand = exprNum; 
            break;  
        default  :
            exprNum   = (int) strtol(expr + 1, NULL, 10);
            exprNum <<= 7;
            exprNum  |= ins->rn;
            ins->operand = exprNum;
            break;
    }    
    return ins;
}

void lsliToBin(struct lsli *ins, FILE *file) {
    unsigned char *start;
    start = (unsigned char*) calloc(4, sizeof(unsigned char));
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
            break;
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
            rn = strtok_r(test, ",", &test);
            op2 = strtok_r(test, " ", &test);
            setIflagAndOper(ins, op2);
            setRn(ins, rn);
            break;
        case 9: //teq
            ins->rd = 0;
            ins->s  = 1;
            rn = strtok_r(test, ",", &test);
            op2 = strtok_r(test, " ", &test);
            setIflagAndOper(ins, op2);
            setRn(ins, rn);
            break;
        case 10: //cmp
            ins->rd = 0;
            ins->s  = 1;
            rn = strtok_r(test, ",", &test);
            op2 = strtok_r(test, " ", &test);
            setIflagAndOper(ins, op2);
            setRn(ins, rn);
            break;
        default:
            rd = strtok_r(test, ",", &test);
            rn = strtok_r(test, ",", &test);
            op2 = strtok_r(test, ",", &test);
            setIflagAndOper(ins, op2);
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

void setIflagAndOper(struct dpi *ins, char *str) {
    int ope2;
    switch (str[0]) {
        case '#':
            ins->i = 1;
            if((str+2) != NULL) {
                if(*(str+2) == 'x') {
                    ope2 = (int) strtol(str+1, (char **) NULL, 16);
                    ins->operand  = ope2;
                    break;
                    printf("you will never see this printing\n");
                }
            }

            ope2 = (int) strtol(str+1, (char **) NULL, 10);
            ins->operand = ope2;
            break;
        case 'r':
            ins->i = 0;
            ope2 = (int) strtol(str+1, (char **) NULL, 10);
            ins->operand = ope2;
            break;
        default:
            fprintf(stderr, "no support operand2\n");
            exit(1);
    }
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
