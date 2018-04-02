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

struct ifid_reg
{
    struct instruction_data instruction;
    int PCPlus4;
};

struct idex_reg
{
    struct instruction_data instruction;
    int PCPlus4;
    int branchTarget;
    int readData1;
    int readData2;
};

struct exmem_reg
{
    struct instruction_data instruction;
    int aluResult;
    int writeDataReg;
    int writeReg;
};

struct memwb_reg
{
    struct instruction_data instruction;
    int writeDataMem;
    int writeDataALU;
    int writeReg;
};

struct pipeline_registers
{
    struct ifid_reg ifid;
    struct idex_reg idex;
    struct exmem_reg exmem;
    struct memwb_reg memwb;
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

int read_data(struct raw_data* src, int* ending_index);

void split_data(struct instruction_data* data, struct raw_data* src, int ending_index);

void print_data(struct memory_data* memory, int* reg_table, struct pipeline_registers* pipeline, int cycle, int pc);

void load_memory(struct raw_data* src, struct memory_data* memory, int starting_index, int ending_index);

void populate_branch_predictor(struct instruction_data* data);

void run_simulation(struct instruction_data* program, struct memory_data* mem, int* reg);

void zero_pipeline(struct pipeline_registers* pipeline);

void run_rtype(struct instruction_data instruction, int* reg);

int main(void)
{
    int i, ending_index;
    int address = 0;
    int data_index = 0;
    
    //create data arrays
    struct raw_data source_table[101];               //holds data in cstr form
    struct instruction_data instruction_table[100];        //table of instructions as ints
    struct branchpredictor branch_table[100];
   
    int reg_table[32];                //32 registers
    for (i = 0; i < 32; ++i)
        reg_table[i] = 0;

    struct memory_data memory[32];

    data_index = read_data(source_table, &ending_index);
    split_data(instruction_table, source_table, data_index - 1);
    load_memory(source_table, memory, data_index, ending_index);
    
    run_simulation(instruction_table, memory, reg_table);    //hit the gas my dude, lets go

    return 0;
}

//reads data into array of cstr
//returns the start of the data section
//sets ending_index to the final index of the whole program
int read_data(struct raw_data* src, int* ending_index)
{
    int data_index;
    int i;
    for (i = 0; i < 101; ++i)
    {
        if (gets(src[i].input) == NULL) break; //read in data as array of cstr
        //printf("%s\n", source_data[i].input);
    }

    *ending_index = i;

    //for (; i < 101; ++i)                       //zero out unused section of array
      //  src[i].input[0] = (char) 0;

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
void print_data(struct memory_data* memory, int* reg_table, struct pipeline_registers* pipeline, int cycle, int pc)
{
    int i;
    printf("********************\nState at the beginning of cycle %d:\n\tPC = %d\n\tData Memory:\n", cycle, pc);

    for (i = 0; i < 16; ++i)
        printf("\t\tmemory_data[%d] = %d\t\tmemory_data[%d] = %d\n", i, memory[i].value, i + 16, memory[i + 16].value);

    printf("\tRegisters:\n");

    for (i = 0; i < 16; ++i)
        printf("\t\treg_table[%d] = %d\t\treg_table[%d] = %d\n", i, reg_table[i], i + 16, reg_table[i + 16]);

    printf("\tIF/ID:\n\t\tInstruction: %d\n", pipeline->ifid.instruction.instruction);
    printf("\t\tPCPlus4: %d\n", pipeline->ifid.PCPlus4);
    
    printf("\tID/EX:\n\t\tInstruction: %d\n", pipeline->idex.instruction.instruction);
    printf("\t\tPCPlus4: %d\n", pipeline->idex.PCPlus4);
    printf("\t\tbranchTarget: %d\n", pipeline->idex.branchTarget);
    printf("\t\treadData1: %d\n", pipeline->idex.readData1);
    printf("\t\treadData2: %d\n", pipeline->idex.readData2);
    printf("\t\timmed: %d\n", pipeline->idex.instruction.immediate);
    printf("\t\trs: %d\n", pipeline->idex.instruction.rs);
    printf("\t\trt: %d\n", pipeline->idex.instruction.rt);
    printf("\t\trd: %d\n", pipeline->idex.instruction.rd);

    printf("\tEX/MEM:\n\t\tInstruction: %d\n", pipeline->exmem.instruction.instruction);
    printf("\t\taluResult: %d\n", pipeline->exmem.aluResult);
    printf("\t\twriteDataReg: %d\n", pipeline->exmem.writeDataReg);
    printf("\t\twriteReg: %d\n", pipeline->exmem.writeReg);

    printf("\tMEM/WB:\n\t\tInstruction: %d\n", pipeline->memwb.instruction.instruction);
    printf("\t\twriteDataMem: %d\n", pipeline->memwb.writeDataMem);
    printf("\t\twriteDataALU: %d\n", pipeline->memwb.writeDataALU);
    printf("\t\twriteReg: %d\n", pipeline->memwb.writeReg);
}

//return address of data section and populate data file
//side note:
//      I feel like theres an off by one error in here,
//      but it works, so i'm not going to fix it.
void load_memory(struct raw_data* src, struct memory_data* memory, int starting_index, int ending_index)
{
    int i, j, address;
    j = 0;                      //(minus 1 because we have an empty line in there)
    address = (starting_index - 1) * 4; //starting line * 4 is the byte address
    for (i = starting_index; i <= ending_index; ++i)
    {
        if (sscanf(src[i].input, "%d", &memory[j].value) == 0) break;
        memory[j++].address = address;      //copy values from the cstr data into the memory array
        address += 4;               //save address too
    }
    for (; j < 32; ++j)             //zero out the unused memory
        memory[j].value = 0;
}

void populate_branch_predictor(struct instruction_data* data)
{
    int i;
    for (i = 0; i < 100; ++i)
    {
        //put code here kappa
    }
}

void run_simulation(struct instruction_data* program, struct memory_data* mem, int* reg)
{
    struct pipeline_registers current, next;
    
    int i;
    int cycle = 1;
    int pc = 0;
    int stop = -1;
    
    zero_pipeline(&current);
    zero_pipeline(&next);

    for(i = 0; i < 5; i++)
    {
        print_data(mem, reg, &current, cycle, pc);
        //IFID//
        next.ifid.instruction = program[pc / 4];
        next.ifid.PCPlus4 = pc + 4;
        //IDEX//
        next.idex.instruction = current.ifid.instruction;
        next.idex.PCPlus4 = current.ifid.PCPlus4;
        next.idex.branchTarget = (next.idex.instruction.immediate * 4) + next.idex.PCPlus4;
        //readdata1
        //readdata2
        
        //EXMEM//
        next.exmem.instruction = current.idex.instuction;
        //aluresult
        //writedatareg
        //writereg

        //MEMWB//
        next.memwb.instruction = current.exmem.instruction;
        //writedatamem
        //writedataalu
        //writereg

        cycle++;
        pc += 4;
        current = next;
    }
}

void zero_pipeline(struct pipeline_registers* pipeline)
{
    pipeline->ifid.instruction.instruction = 0;
    pipeline->ifid.instruction.opcode = 0;
    pipeline->ifid.instruction.rs = 0;
    pipeline->ifid.instruction.rt = 0;
    pipeline->ifid.instruction.rd = 0;
    pipeline->ifid.instruction.immediate = 0;
    pipeline->ifid.instruction.shamt = 0;
    pipeline->ifid.instruction.func = 0;
    pipeline->ifid.PCPlus4 = 0;

    pipeline->idex.instruction.instruction = 0;
    pipeline->idex.instruction.opcode = 0;
    pipeline->idex.instruction.rs = 0;
    pipeline->idex.instruction.rt = 0;
    pipeline->idex.instruction.rd = 0;
    pipeline->idex.instruction.immediate = 0;
    pipeline->idex.instruction.shamt = 0;
    pipeline->idex.instruction.func = 0;
    pipeline->idex.PCPlus4 = 0;
    pipeline->idex.branchTarget = 0;
    pipeline->idex.readData1 = 0;
    pipeline->idex.readData2 = 0;

    pipeline->exmem.instruction.instruction = 0;
    pipeline->exmem.instruction.opcode = 0;
    pipeline->exmem.instruction.rs = 0;
    pipeline->exmem.instruction.rt = 0;
    pipeline->exmem.instruction.rd = 0;
    pipeline->exmem.instruction.immediate = 0;
    pipeline->exmem.instruction.shamt = 0;
    pipeline->exmem.instruction.func = 0;
    pipeline->exmem.aluResult = 0;
    pipeline->exmem.writeDataReg = 0;
    pipeline->exmem.writeReg = 0;
    //if u read this ur gay
    pipeline->memwb.instruction.instruction = 0;
    pipeline->memwb.instruction.opcode = 0;
    pipeline->memwb.instruction.rs = 0;
    pipeline->memwb.instruction.rt = 0;
    pipeline->memwb.instruction.rd = 0;
    pipeline->memwb.instruction.immediate = 0;
    pipeline->memwb.instruction.shamt = 0;
    pipeline->memwb.instruction.func = 0;
    pipeline->memwb.writeDataMem = 0;
    pipeline->memwb.writeDataALU = 0;
    pipeline->memwb.writeReg = 0;
}

void run_rtype(struct instruction_data inst, int* reg)
{
        if (inst.func == 32) //ADD
        {
            reg[inst.rd] = reg[inst.rs] + reg[inst.rt];
            return;
        }
        if (inst.func == 34)//SUB
        {
            reg[inst.rd] = reg[inst.rs] - reg[inst.rt];
            return;
        }
        //SLL
        reg[inst.rd] = reg[inst.rt] << inst.shamt;
        return;
}