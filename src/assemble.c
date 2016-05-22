#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define MAXLINE 32
#define HASHSIZE 101

//create hash table and entry for hashtable
static struct entry *hashTable[HASHSIZE];

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

//API for convert data processing instruction to binary file

//write the output binary file using the struct dpi returned
//by the struct dpi *convert(char *)

void dpiToBin(struct dpi *, FILE *);

//conver the data processing ins
struct dpi *convert(char *);

//followings are helper function of struct dpi *conver(char *)

//change the big endian to small endian memory
void endianConvert(unsigned char *);
void swap(unsigned char *, unsigned char *);
//get the decimal number of opcode
int  getOpVal(char *);
//set up I field and Operand field
void setIflagAndOper(struct dpi *, char *);
//set up desitination regesiter field
void setRd(struct dpi *, char *);
//set up first operand register
void setRn(struct dpi *, char *);


//API for hashtable
unsigned int has(char *);
struct entry *lookup(char *);
struct entry *put(char *, char *);

//error handling check if there is enough space for malloc
void isEnoughSpace(void *);


//main function
int main(int argc, char **argv) {
    //create a symbol table
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

    //reading soruce file and creating output file
    char *inname  = argv[1];
    char *outname = argv[2];
    FILE *inFile;
    FILE *outFile;
    char lineBuffer[MAXLINE];
    //initilize the files
    inFile    = fopen(inname, "r");
    outFile   = fopen(outname, "wb");
    //error handling for inFile
    if (inFile == NULL) {
        exit(EXIT_FAILURE);
    }
    //IO
    while (fgets(lineBuffer, MAXLINE, inFile)) {
        struct dpi *ins = (struct dpi*) malloc(sizeof(struct dpi));
        isEnoughSpace(ins);

        ins = convert(lineBuffer);
        dpiToBin(ins, outFile);

        free(ins);
    }

    fclose(outFile);
    fclose(inFile);

    return 0;
}

//dataProcessingConverter
void endianConvert(unsigned char *ip) {
    swap(ip, ip + 3);
    swap(ip + 1, ip + 2);
}

void swap(unsigned char *first, unsigned char *end) {
    char temp = *first;
    *first = *end;
    *end   = temp;
}

struct dpi *convert(char *str) {
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
            op2 = strtok_r(test, ",", &test);
            setIflagAndOper(ins, op2);
            setRd(ins, rd);

            break;
        case 8: //tst
            ins->rd = 0;
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
            op2 = strtok_r(test, ",", &test);
            setIflagAndOper(ins, op2);
            setRn(ins, rn);
            setRd(ins, rd);


            break;
    }

    return ins;
}

//get the int value of entry
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
    char *test = str;
    int iflag;
    int ope2;
    if (test[0] == '#') {
        iflag = 1;
        if(*(test+2) == 'x') {
            ope2 = (int) strtol(test+1, (char **) NULL, 16);
        } else {
            ope2 = (int) strtol(test+1, (char **) NULL, 10);
        }
    } else {
        //assert(&(test+1) == 'r');
        iflag = 0;
        ope2 = (int) strtol(test+1, (char **) NULL, 10);
    }

    ins->i = iflag;
    ins->operand = ope2;
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

    return hashval % HASHSIZE;
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

void isEnoughSpace(void *ip) {
    if (ip == NULL) {
        fprintf(stderr, "Out of memory, exiting\n");
        exit(1);
    }
}
