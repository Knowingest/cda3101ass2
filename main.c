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
    int address;
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
    int PCPlus4;
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
    int PCPlus4;
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
void print_data(int* regFile, struct dataMem* memory);
int load_memory(struct instruction_data* data, struct dataMem* memory);
void populate_branch_predictor(struct instruction_data* data);

int main(void)
{
    int i;
    int address = 0;
    //create data arrays
    struct instruction_data data_table[100];
    struct branchpredictor branchpredict[100];
   
    int regFile[32];
        for (i = 0; i < 32; ++i)
            regFile[i] = 0;
    struct dataMem memory[32];

    //read data
    for (i = 0; i < 100; ++i)
    {
        if ((scanf("%d", &data_table[i].instruction)) != 1) break;  //break the scanf loop if we reach end of stream
        data_table[i].address = address;
        address += 4;
    }
    
    for (; i < 100; ++i)
        data_table[i].instruction = 0;      //zero out the unused portions of the array

    //split data into pieces
    split_data(data_table);
    int dataStart = load_memory(data_table, memory);
    print_data(regFile, memory);

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

//print data for each cycle in its entirety
void print_data(int* regFile, struct dataMem* memory)
{
    int i;
    printf("********************\nState at the beginning of cycle X:\n\tPC = X\n\tData Memory:\n");

    for (i = 0; i < 16; ++i)
    {
        printf("\t\tdataMem[%d] = %d\t\tdataMem[%d] = %d\n", i, memory[i].value, i + 16, memory[i + 16].value);
    }

    printf("\tRegisters:\n");

    for (i = 0; i < 16; ++i)
    {
        printf("\t\tregFile[%d] = %d\t\tregFile[%d] = %d\n", i, regFile[i], i + 16, regFile[i + 16]);
    }

    printf("\tIF/ID:\n\t\tInstruction: X\n\t\tPCPlus4: X\n");
}

//return address of data section and populate data file
int load_memory(struct instruction_data* data, struct dataMem* memory)
{
    int i, j;

    for (i = 0; i < 100; ++i)
        if (data[i].instruction == 1)       //if we find the instruction 1, we know the data segment will follow
        {
            i += 1;
            for (j = 0; j < 32 && i < 100; ++j)     //so we copy down the remaining "instructions" as data
            {
                memory[j].address = data[i].address;
                memory[j].value = data[i++].instruction;    //we stop the loop after 32 iterations
                                                           //(default to zero if nothing is there to read)
            }
            break;
        }          
    
    return memory[0].address;   //return starting address of data section
}

void populate_branch_predictor(struct instruction_data* data)
{
    
}