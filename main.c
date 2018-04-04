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
    int target;
    int state;
};

int read_data(struct raw_data* src, int* ending_index);

void split_data(struct instruction_data* data, struct raw_data* src, int ending_index);

void print_data(struct memory_data* memory, int* reg_table, struct pipeline_registers* pipeline, int cycle, int pc);

void print_instruction(struct instruction_data instruction);

void print_register(int reg);

void load_memory(struct raw_data* src, struct memory_data* memory, int starting_index, int ending_index);

void fill_branch_table(struct instruction_data* data, struct branchpredictor* branch_table);

void run_simulation(struct instruction_data* program, struct memory_data* mem, int* reg);

void zero_pipeline(struct pipeline_registers* pipeline);

int run_rtype(struct instruction_data instruction, int* reg);

int run_alu(struct instruction_data instruction, int* reg);

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
    //fill_branch_table(instruction_table, branch_table);
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

    for (; i < 101; ++i)                       //zero out unused section of array
        src[i].input[0] = (char) 0;

    for (i = 0; i < 101; ++i)                  //find where data section starts
    {
        if (src[i].input[0] == (char) 13 || src[i].input[0] == (char) 0) break;
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

    printf("\tIF/ID:\n\t\tInstruction: ");
    print_instruction(pipeline->ifid.instruction);
    printf("\n\t\tPCPlus4: %d\n", pipeline->ifid.PCPlus4);
    
    printf("\tID/EX:\n\t\tInstruction: ");
    print_instruction(pipeline->idex.instruction);
    printf("\n\t\tPCPlus4: %d\n", pipeline->idex.PCPlus4);
    printf("\t\tbranchTarget: %d\n", pipeline->idex.branchTarget);
    printf("\t\treadData1: %d\n", pipeline->idex.readData1);
    printf("\t\treadData2: %d\n", pipeline->idex.readData2);
    printf("\t\timmed: %d\n", pipeline->idex.instruction.immediate);
    printf("\t\trs: %d\n", pipeline->idex.instruction.rs);
    printf("\t\trt: %d\n", pipeline->idex.instruction.rt);
    printf("\t\trd: %d\n", pipeline->idex.instruction.rd);

    printf("\tEX/MEM:\n\t\tInstruction: ");
    print_instruction(pipeline->exmem.instruction);
    printf("\n\t\taluResult: %d\n", pipeline->exmem.aluResult);
    printf("\t\twriteDataReg: %d\n", pipeline->exmem.writeDataReg);
    printf("\t\twriteReg: %d\n", pipeline->exmem.writeReg);

    printf("\tMEM/WB:\n\t\tInstruction: ");
    print_instruction(pipeline->memwb.instruction);
    printf("\n\t\twriteDataMem: %d\n", pipeline->memwb.writeDataMem);
    printf("\t\twriteDataALU: %d\n", pipeline->memwb.writeDataALU);
    printf("\t\twriteReg: %d\n", pipeline->memwb.writeReg);
}

void print_instruction(struct instruction_data instruction)
{
    //printf("printing instruction with integer value: %d\n", instruction.instruction);
        //NOOP
    if (instruction.instruction == 0)
    {
        printf("NOOP");
        return;
    }
        //halt
    if (instruction.instruction == 1)
    {
        printf("halt");
        return;
    }
    
        //R-Type
    if (instruction.opcode == 0)
    {
        if (instruction.func == 32)
            printf("add ");
        else if (instruction.func == 34)
            printf("sub ");
        else
            printf("sll ");

        print_register(instruction.rd);
        printf(",");
        print_register(instruction.rs);
        printf(",");
        if (instruction.func == 0)
            printf("%d", instruction.shamt);
        else
            print_register(instruction.rt);
        return;
    }

    if (instruction.opcode == 35 || instruction.opcode == 43)
    {
        if (instruction.opcode == 35)
            printf("lw ");
        else
            printf("sw ");
        print_register(instruction.rs);
        printf(", %d(", instruction.shamt);
        print_register(instruction.rt);
        printf(")");
        return;
    }

    if (instruction.opcode == 13)
        printf("ori ");
    else if (instruction.opcode == 12)
        printf("andi ");
    else 
        printf("bne ");
    print_register(instruction.rs);
    printf(",");
    print_register(instruction.rt);
    printf(",%d", instruction.immediate);

    //printf("%d", instruction.instruction);
}

void print_register(int reg)
{
    if (reg == 0)
        printf("$0");
    else if (7 < reg && reg < 16)
        printf("$t%d", reg - 8);
    else if (15 < reg && reg < 24)
        printf("$s%d", reg - 16);
    else
        printf("%d", reg);
}

//return address of data section and populate data file
//side note:
//      I feel like theres an off by one error in here,
//      but it works, so i'm not going to fix it.
void load_memory(struct raw_data* src, struct memory_data* memory, int starting_index, int ending_index)
{
    printf("loading memory with starting_index = %d\tending_index = %d\n", starting_index, ending_index);
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
    int cycle = 1;      //cycle counter
    int pc = 0;         //pc counter (increments in fours)
    int stop = 0;      //pseudo boolean used to stop the program
    int ifid_halted, idex_halted, exmem_halted;

        //these mark the pipelines as being shut down, after finding a halt
    ifid_halted = idex_halted = exmem_halted = 0;
    

    zero_pipeline(&current);
    zero_pipeline(&next);

    while(1)
    {
        print_data(mem, reg, &current, cycle, pc);
        if (stop) break;

        ////////
        //IFID//
        ////////
        if (current.ifid.instruction.instruction != 1 && !ifid_halted)
            next.ifid.instruction = program[pc / 4];
        else if (!ifid_halted)
        {
            ifid_halted = 1;
            next.ifid.instruction.instruction = 0;
        }
            next.ifid.PCPlus4 = pc + 4;

        ////////
        //IDEX//
        ////////
        if (current.idex.instruction.instruction != 1 && !idex_halted)
            next.idex.instruction = current.ifid.instruction;
        else if (!idex_halted)
        {
            idex_halted = 1;
            next.idex.instruction.instruction = 0;
        }
        next.idex.PCPlus4 = current.ifid.PCPlus4;
        next.idex.branchTarget = (next.idex.instruction.immediate * 4) + next.idex.PCPlus4;
        next.idex.readData1 = reg[next.idex.instruction.rt];
        next.idex.readData1 = reg[next.idex.instruction.rd];
        
        /////////
        //EXMEM//
        /////////
        if (current.exmem.instruction.instruction != 1 && !exmem_halted)
            next.exmem.instruction = current.idex.instruction;
        else if (!exmem_halted)
        {
            exmem_halted = 1;
            next.exmem.instruction.instruction = 0;
        }
        next.exmem.aluResult = run_alu(next.exmem.instruction, reg);
        next.exmem.writeDataReg = reg[next.exmem.instruction.rt];
        if (next.exmem.instruction.opcode == 0) next.exmem.writeReg = next.exmem.instruction.rd;
        else next.exmem.writeReg = next.exmem.instruction.rt;

        /////////
        //MEMWB//
        /////////
        next.memwb.instruction = current.exmem.instruction;
        
        if (next.memwb.instruction.instruction == 1)
            stop = 1;
        next.memwb.writeDataMem = mem[current.exmem.aluResult / 4].value;
        next.memwb.writeDataALU = current.exmem.aluResult;
        if (next.memwb.instruction.opcode == 0) next.memwb.writeReg = next.memwb.instruction.rd;
        else next.memwb.writeReg = next.memwb.instruction.rt;


        if (next.memwb.instruction.instruction != 0 && stop != 1)
        {
            if (next.memwb.instruction.opcode == 0)//r type
                reg[next.memwb.instruction.rd] = current.exmem.aluResult;
            
            if (next.memwb.instruction.opcode == 12 || next.memwb.instruction.opcode == 13) //andi ori
                reg[next.memwb.instruction.rt] = current.exmem.aluResult;
            
            if (next.memwb.instruction.opcode == 35)//lw
                reg[next.memwb.instruction.rt] = mem[(current.exmem.aluResult - mem[0].address) / 4].value;
            
            if (next.memwb.instruction.opcode == 43)//sw
                mem[(current.exmem.aluResult - mem[0].address) / 4].value = reg[next.memwb.instruction.rt];
        }

        cycle++;
        pc += 4;
        current = next;
    }
}

int run_alu(struct instruction_data instruction, int* reg)
{
    if (instruction.instruction == 0 || instruction.instruction == 1) return 0;

    if (instruction.opcode == 0) //r type
        return run_rtype(instruction, reg);

    if (instruction.opcode == 12)//andi
        return reg[instruction.rs] & instruction.immediate;

    if (instruction.opcode == 13)//ori
        return reg[instruction.rs] | instruction.immediate;

    if (instruction.opcode == 35 || instruction.opcode == 43)//LW | SW
        return instruction.immediate + reg[instruction.rs];

    return reg[instruction.rs] - reg[instruction.rt];
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

int run_rtype(struct instruction_data inst, int* reg)
{
        if (inst.func == 32) //ADD
            return reg[inst.rs] + reg[inst.rt];    
        if (inst.func == 34)//SUB
            return reg[inst.rs] - reg[inst.rt];
        //SLL
        return reg[inst.rt] << inst.shamt;
}