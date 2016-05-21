#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define MAXLINE 32
#define HASHSIZE 101


//hashtable
struct entry {
    struct entry *next;
    char *name;
    char *defn;
};

static struct entry *hashTable[HASHSIZE];

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

//instruction
struct dpi {
    int cond    : 4;
    int iden    : 2;
    int i       : 1;
    int opcode  : 4;
    int s       : 1;
    int rn      : 4;
    int rd      : 4;
    int operand : 12;
};


//tokenizer


void endianConvert(char *);
void swap(char *, char *);
int  getOpVal(char *);
struct dpi *convert(char *);
void setIflagAndOper(struct dpi *, char *);
void setRd(struct dpi *, char *);
void setRn(struct dpi *, char *);



int main(int argc, char **argv) {
    /* create a symbol table */
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




    /* reading source file */
    char *inname  = argv[1];
    char *outname = argv[2];
    FILE *inFile;
    FILE *outFile;

    char lineBuffer[MAXLINE];

    inFile    = fopen(inname, "r");
    outFile   = fopen(outname, "wb");

    if (inFile == NULL) {
        exit(EXIT_FAILURE);
    }

    while(fgets(lineBuffer, MAXLINE, inFile)) {



    }

    fclose(outFile);
    fclose(inFile);
    return 0;
}

//dataProcessingConverter
void endianConvert(char *ip) {
    swap(ip, ip + 3);
    swap(ip + 1, ip + 2);
}

void swap(char *first, char *end) {
    char temp = *first;
    *first = *end;
    *end   = temp;
}


struct dpi *convert(char *str) {
    char *test = str;
    char *opcode, *rd, *rn, *op2;
    struct dpi *ins = (struct dpi*) malloc(sizeof(struct dpi));

    opcode = strtok_r(test, " ", &test);

    int opc     = getOpVal(opcode);
    ins->opcode = opc;
    ins->s = 0;
    ins->cond = 10;

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
