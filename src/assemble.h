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

struct sdti {
    unsigned int cond : 4;
    unsigned int id   : 2;
    unsigned int i    : 1;
    unsigned int p    : 1;
    unsigned int u    : 1;
    unsigned int id2  : 2;
    unsigned int l    : 1;
    unsigned int rn   : 4;
    unsigned int rd   : 4;
    unsigned int offs : 12;
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

//parsing single data transfer
void parseSdti(char *, FILE *);

struct sdti *sdtiConvert(char *);
void parseSdti(char *, FILE *);

void removeChar(char *, char);
int isSquare(char *);
void helpParseRnRmU(struct sdti *, char *);
void parseRnRmU(struct sdti *, char *, char *);
void sdtiToBin(struct sdti *, FILE *);
void ldrExpress(struct sdti *, char *);


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
