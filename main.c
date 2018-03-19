#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>


//these are the bit masks that will be used to isolate sections of an instruction
const unsigned int sixbits = 63;                //six 1s, can be shifted as needed
const unsigned int opmask = 4227858432;        //opcode mask
const unsigned int rsmask = 65011712;         //rs register mask
const unsigned int rtmask = 2031616;         //rt register mask
const unsigned int rdmask = 63488;          //rd register mask
const unsigned int immask = 65535;         //immediate value mask
const unsigned int shiftmask = 1984;      //shift amount mask
const unsigned int fnmask = 63;          //function mask

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
    for (i = 0; i < 100; ++i)       //scanf loop for reading data.
    {                              //record entire instructions along with their address
        if ((scanf("%d", &data_table[i].instruction)) != 1) break;  //break the scanf loop if we reach end of stream
        data_table[i].address = address;
        address += 4;
    }
    
    for (; i < 100; ++i)                     //zero out the remaining array indices
        data_table[i].instruction = 0;

    split_data(data_table);    //split data into pieces
    int dataStart = load_memory(data_table, memory);//load memory and save the starting address of the data section
    //printf("start of data section: %d\n", dataStart);
    print_data(regFile, memory);    //print out data

    return 0;
}

//cuts out pieces of 32 bit instructions for future use
void split_data(struct instruction_data* data)
{
    int i;      //essentially we cut out individual portions of the instruction
    for (i = 0; i < 100; ++i)
    {   //we apply bit masks and shift the result forward so it can be easily read
        data[i].opcode = (data[i].instruction & opmask) >> 26;
        data[i].rs = (data[i].instruction & rsmask) >> 21;
        data[i].rt = (data[i].instruction & rtmask) >> 16;
        data[i].rd = (data[i].instruction & rdmask) >> 11;
        data[i].immediate = (data[i].instruction & immask);
        data[i].shamt = (data[i].instruction & shiftmask) >> 6;
        data[i].func = (data[i].instruction & fnmask);
    }   //we can now reference each individual component of the instruction
}     //(we store all of the components for all instructions, even though they won't all be used)

//print data for the current cycle
void print_data(int* regFile, struct dataMem* memory)
{
    int i;
    printf("********************\nState at the beginning of cycle X:\n\tPC = X\n\tData Memory:\n");

    for (i = 0; i < 16; ++i)
        printf("\t\tdataMem[%d] = %d\t\tdataMem[%d] = %d\n", i, memory[i].value, i + 16, memory[i + 16].value);

    printf("\tRegisters:\n");

    for (i = 0; i < 16; ++i)
        printf("\t\tregFile[%d] = %d\t\tregFile[%d] = %d\n", i, regFile[i], i + 16, regFile[i + 16]);

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
            }                                              //(default to zero if nothing is there to read)
            break;
        }          
    return memory[0].address;   //return starting address of data section
}

void populate_branch_predictor(struct instruction_data* data)
{
    int i;
    for (i = 0; i < 100; ++i)
    {
        //put code here kappa
    }
}