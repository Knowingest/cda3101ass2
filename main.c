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

struct raw_data
{
    char input[32];
};

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

struct memory_data
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

int read_data(struct raw_data* src);
void split_data(struct instruction_data* data, struct raw_data* src, int ending_index);
void print_data(int* regFile, struct memory_data* memory);
void load_memory(struct raw_data* src, struct memory_data* memory, int starting_index);
void populate_branch_predictor(struct instruction_data* data);

int main(void)
{
    int i;
    int address = 0;
    int data_index = 0;
    //create data arrays
    struct raw_data source_table[101];               //holds data in cstr form
    struct instruction_data instruction_table[100];        //table of instructions as ints
    struct branchpredictor branch_table[100];
   
    int regFile[32];                //32 registers
    for (i = 0; i < 32; ++i)
        regFile[i] = 0;
    struct memory_data memory[32];

    data_index = read_data(source_table);       //read data and record start of data section
    split_data(instruction_table, source_table, data_index - 1);    //split data into pieces
    load_memory(source_table, memory, data_index);
    print_data(regFile, memory);    //print out data

    return 0;
}

//reads data into array of cstr and returns start of data section
int read_data(struct raw_data* src)
{
    int data_index;
    int i;
    for (i = 0; i < 101; ++i)
    {
        if (gets(src[i].input) == NULL) break; //read in data as array of cstr
        //printf("%s\n", source_data[i].input);
    }

    for (; i < 101; ++i)                       //zero out unused section of array
        src[i].input[0] = (char) 0;

    for (i = 0; i < 101; ++i)                  //find where data section starts
    {
        if (src[i].input[0] == (char) 0) break;
        ++data_index;
    }
    return data_index + 1;                     //return start of data section
}

//converts the source cstr array into an array of integers.
//ends when it reaches the data section.
//also cuts up the instruction so you can reference the individual components
void split_data(struct instruction_data* data, struct raw_data* src, int ending_index)
{
    //printf("splitting data\n");
    int i;
    for (i = 0; i < ending_index; ++i)
    {
        sscanf(src[i].input, "%d", &data[i].instruction);
        data[i].opcode = (data[i].instruction & opmask) >> 26;
        data[i].rs = (data[i].instruction & rsmask) >> 21;
        data[i].rt = (data[i].instruction & rtmask) >> 16;
        data[i].rd = (data[i].instruction & rdmask) >> 11;
        data[i].immediate = (data[i].instruction & immask);
        data[i].shamt = (data[i].instruction & shiftmask) >> 6;
        data[i].func = (data[i].instruction & fnmask);
    }   //we can now reference each individual component of the instruction
    //(we store all of the components for all instructions, even though they won't all be used)
    for (; i < 100; ++i)        //zero out remaining elements
            data[i].instruction = 0;
}     

//print data for the current cycle
void print_data(int* regFile, struct memory_data* memory)
{
    int i;
    printf("********************\nState at the beginning of cycle X:\n\tPC = X\n\tData Memory:\n");

    for (i = 0; i < 16; ++i)
        printf("\t\tmemory_data[%d] = %d\t\tmemory_data[%d] = %d\n", i, memory[i].value, i + 16, memory[i + 16].value);

    printf("\tRegisters:\n");

    for (i = 0; i < 16; ++i)
        printf("\t\tregFile[%d] = %d\t\tregFile[%d] = %d\n", i, regFile[i], i + 16, regFile[i + 16]);

    printf("\tIF/ID:\n\t\tInstruction: X\n\t\tPCPlus4: X\n");
}

//return address of data section and populate data file
void load_memory(struct raw_data* src, struct memory_data* memory, int starting_index)
{
    //printf("loading memory\n");
    int i, j, address;
    j = 0;
    address = (starting_index - 1) * 4; //starting line * 4 is the byte address
    
    for (i = starting_index; i < 101; ++i)  //(minus 1 because we have an empty line)
    {
        sscanf(src[i].input, "%d", &memory[j].value);
        memory[j++].address = address;
        address += 4;
    }
}

void populate_branch_predictor(struct instruction_data* data)
{
    int i;
    for (i = 0; i < 100; ++i)
    {
        //put code here kappa
    }
}