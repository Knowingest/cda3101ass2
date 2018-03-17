#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

const unsigned int sixbits = 63;
const unsigned int opmask = 4227858432;
const unsigned int rsmask = 65011712;
const unsigned int rtmask = 2031616;
const unsigned int rdmask = 63488;
const unsigned int immask = 65535;
const unsigned int shiftmask = 1984;
const unsigned int fnmask = 63;

struct instruction_data
{
    int instruction;
    unsigned int opcode;
    unsigned int rs;
    unsigned int rt;
    unsigned int rd;
    unsigned int immediate;
    unsigned int shamt;
    unsigned int func;
};

struct ifid
{
    int instruction;
    unsigned int opcode;
    unsigned int rs;
    unsigned int rt;
    unsigned int rd;
    unsigned int immediate;
    unsigned int shamt;
    unsigned int func;
    int pcp4;
};

struct idex
{
    int instruction;
    unsigned int opcode;
    unsigned int rs;
    unsigned int rt;
    unsigned int rd;
    unsigned int immediate;
    unsigned int shamt;
    unsigned int func;
    int pcp4;
    int branchtarget;
    int readdata1;
    int readdata2;
};

struct exmem
{
    int instruction;
    unsigned int opcode;
    unsigned int rs;
    unsigned int rt;
    unsigned int rd;
    unsigned int immediate;
    unsigned int shamt;
    unsigned int func;
    int aluresult;
    int writedata;
    int writeregister;
};

struct memwb
{
    unsigned int opcode;
    unsigned int rs;
    unsigned int rt;
    unsigned int rd;
    unsigned int immediate;
    unsigned int shamt;
    unsigned int func;
    int datafrommemory;
    int datafromalu;
};

struct dataMem
{
    int address;
    int value;
};

struct branchpredictor
{
    int pc;
    int target;
    int state;
};

void split_data(struct instruction_data* data);
void print_data();

int main(void)
{
    int i;
    //create data arrays
    struct instruction_data data_table[100];
    struct branchpredictor branchpredict[100];
   
    unsigned int regFile[32];
        for (i = 0; i < 32; ++i)
            regFile[i] = 0;
    struct dataMem memory[32];

    //read data
    for (i = 0; i < 100; ++i)
        scanf("%d", &data_table[i].instruction);

    //split data into pieces
    split_data(data_table);
    print_data();
    return 0;
}

//cuts out pieces of 32 bit instructions for future use
void split_data(struct instruction_data* data)
{
    int i;
    for (i = 0; i < 100; ++i)
    {
        data[i].opcode = (data[i].instruction & opmask) >> 26;
        data[i].rs = (data[i].instruction & rsmask) >> 21;
        data[i].rt = (data[i].instruction & rtmask) >> 16;
        data[i].rd = (data[i].instruction & rdmask) >> 11;
        data[i].immediate = (data[i].instruction & immask);
        data[i].shamt = (data[i].instruction & shiftmask) >> 6;
        data[i].func = (data[i].instruction & fnmask);
    }
}

void print_data()
{
    int i;
    printf("********************\nState at the beginning of cycle X:\n\tPC = X\n\tData Memory:\n");

    for (i = 0; i < 16; ++i)
    {
        printf("\t\tdataMem[%d] = X\t\tdataMem[%d] = X\n", i, i + 16);
    }

    printf("\tRegisters:\n");

    for (i = 0; i < 16; ++i)
    {
        printf("\t\tregFile[%d] = X\t\tregFile[%d] = X\n", i, i + 16);
    }

    printf("\tIF/ID:\n\t\tInstruction: X\n\t\tPCPlus4: X\n");
}