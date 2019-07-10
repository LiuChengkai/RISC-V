#ifndef RISC_V_BASIC_HPP
#define RISC_V_BASIC_HPP
#include <iostream>
#include <cstdio>
#include <cstring>
#include <bitset>
using namespace std;
typedef unsigned int uint;
//#define ROUND
//#define LOG

#ifdef ROUND
int round = 0;
#endif

enum instT {
    LUI, AUIPC, JAL, JALR,  // 0~3
    BEQ, BNE, BLT, BGE, BLTU, BGEU, //4~9
    LB, LH, LW, LBU, LHU,  //10~14
    SB, SH, SW, //15~17
    ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI,   //18~26
    ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND,    //27~36
};

bool is_branch(instT inst) {
    if (inst == JAL || inst == JALR || inst == BEQ || inst == BNE || inst == BLT || inst == BGE || inst == BLTU || inst == BGEU)
        return true;
    else
        return false;
}

bool is_ALU(instT inst) {
    if (inst == LUI || inst == AUIPC || inst == ADDI || inst == SLTI || inst == SLTIU || inst == XORI || inst == ORI || inst == SLLI || inst == SRLI || inst == SRAI
        || inst == ADD || inst == SUB  || inst == SLL || inst == SLT || inst == SLTU || inst == XOR || inst == SRL || inst == SRA || inst == OR || inst == AND)
        return true;
    else
        return false;
}

bool is_load_store(instT inst) {
    if (inst == LB || inst == LH || inst == LW || inst == LBU || inst == LHU || inst == SB || inst == SH || inst == SW)
        return true;
    else
        return false;
}

char mem[0x20000];
int reg[32];
int pc = 0;

void view_reg() {
    cout << hex << pc << ' ';
    cout << dec;
    for (int i = 1; i < 32; ++i)
        cout << reg[i] << ' ';
    cout << endl;
}

struct Instruction {
    instT type;
    int inst, imm;
    int rd, rs1, rs2;
    int shamt, opcode, funct3, funct7;

    void update() {
        rd = ((inst >> 7) & 0b11111);
        rs1 = ((inst >> 15) & 0b11111);
        rs2 = ((inst >> 20) & 0b11111);
        shamt = rs2;
        opcode = (inst & 0b1111111);
        funct3 = ((inst >> 12) & 0b111);
        funct7 = ((inst >> 30) & 1);
    }

    void get_type() {
        switch (opcode) {
            case 0b0110111:
                type = LUI;
                break;

            case 0b0010111:
                type = AUIPC;
                break;

            case 0b1101111:
                type = JAL;
                break;

            case 0b1100111:
                type = JALR;
                break;

            case 0b1100011:
                switch (funct3) {
                    case 0b000:
                        type = BEQ;
                        break;

                    case 0b001:
                        type = BNE;
                        break;

                    case 0b100:
                        type = BLT;
                        break;

                    case 0b101:
                        type = BGE;
                        break;

                    case 0b110:
                        type = BLTU;
                        break;

                    case 0b111:
                        type = BGEU;
                        break;
                }
                break;

            case 0b0000011:
                switch (funct3) {
                    case 0b000:
                        type = LB;
                        break;

                    case 0b001:
                        type = LH;
                        break;

                    case 0b010:
                        type = LW;
                        break;

                    case 0b100:
                        type = LBU;
                        break;

                    case 0b101:
                        type = LHU;
                        break;
                }
                break;

            case 0b0100011:
                switch (funct3) {
                    case 0b000:
                        type = SB;
                        break;

                    case 0b001:
                        type = SH;
                        break;

                    case 0b010:
                        type = SW;
                        break;
                }
                break;

            case 0b0010011:
                switch (funct3) {
                    case 0b000:
                        type = ADDI;
                        break;

                    case 0b010:
                        type = SLTI;
                        break;

                    case 0b011:
                        type = SLTIU;
                        break;

                    case 0b100:
                        type = XORI;
                        break;

                    case 0b110:
                        type = ORI;
                        break;

                    case 0b111:
                        type = ANDI;
                        break;

                    case 0b001:
                        type = SLLI;
                        break;

                    case 0b101:
                        if (funct7 == 0)
                            type = SRLI;
                        else
                            type = SRAI;
                        break;
                }
                break;

            case 0b0110011:
                switch (funct3) {
                    case 0b000:
                        if (funct7 == 0)
                            type = ADD;
                        else
                            type = SUB;
                        break;

                    case 0b001:
                        type = SLL;
                        break;

                    case 0b010:
                        type = SLT;
                        break;

                    case 0b011:
                        type = SLTU;
                        break;

                    case 0b100:
                        type = XOR;
                        break;

                    case 0b101:
                        if (funct7 == 0)
                            type = SRL;
                        else
                            type = SRA;
                        break;

                    case 0b110:
                        type = OR;
                        break;

                    case 0b111:
                        type = AND;
                        break;
                }
                break;
        }
    }

    void get_imm() {
        imm = 0;
        switch(type) {
            //imm[31:12]
            case LUI: case AUIPC:
                imm = ((inst >> 12) << 12);
                break;

                //imm[20|10:1|11|19:12]
            case JAL:
                imm = ((((inst >> 31) & 1) << 31) >> 11);
                imm += ((inst >> 21) & (0b1111111111)) << 1;
                imm += ((inst >> 20) & (0b1)) << 11;
                imm += ((inst >> 12) & (0b11111111)) << 12;
                break;

                //imm[11:0]
            case JALR:
            case LB: case LH: case LW: case LBU: case LHU:
            case ADDI: case XORI: case ORI: case ANDI:
            case SLTIU:
                imm = inst >> 20;
                break;

                //imm[12|10:5], imm[4:1|11]
            case BEQ: case BNE: case BLT: case BGE: case BLTU: case BGEU:
                imm = ((((inst >> 31) & 1) << 31) >> 19);
                imm += ((inst >> 7) & 0b1) << 11;
                imm += ((inst >> 25) & 0b111111) << 5;
                imm += ((inst >> 8) & 0b1111) << 1;
                break;

                //imm[11:5], imm[4:0]
            case SB: case SH: case SW:
                imm = ((inst >> 25) << 5);
                imm += (inst >> 7) & 0b11111;
                break;

            default:
                ;
        }
    }
};

struct IF_ID {
    Instruction IR;
    int NPC;

    bool todo;

} if_id;

struct ID_EX {
    Instruction IR;
    int NPC;
    int imm;
    int A, B;

    bool todo;

} id_ex;

struct EX_MEM {
    Instruction IR;
    int NPC;
    int ALUoutput;
    int cond;

    bool todo;
} ex_mem;

struct MEM_WB {
    Instruction IR;
    int NPC;
    int ALUoutput;

    bool todo;
} mem_wb;

int MEM_delay;

bool if_idle, id_idle, ex_idle, mem_idle, wb_idle;

void init() {
    if_id.todo = id_ex.todo = ex_mem.todo = mem_wb.todo = false;
    if_idle = id_idle = ex_idle = mem_idle = wb_idle = false;

}


bool IF() {
    memcpy(&if_id.IR.inst, mem + pc, 4);
    if ()

}

void ID() {
    if (id_idle)    return;
    if (!if_id.todo)    return;

    id_ex.npc = if_id.npc;
    id_ex.IR.inst = if_id.IR.inst;

    id_ex.IR.update();
    id_ex.IR.get_type();
    id_ex.IR.get_imm();


}

void EX() {
    if (ex_idle)    return;
    if (!id_ex.todo)    return;

    if (is_alu(id_ex.IR.type)) {
        ex_mem.IR = id_ex.IR;
        //分情况计算
    }
    else if (is_load_store(id_ex.IR.type)) {
        ex_mem.IR = id_ex.IR;
        //分情况计算

    }
    else if (is_jump(id_ex.IR.type)) {
        //分情况计算
    }


}

void MEM() {
    if (mem_idle)   return;
    if (!ex_mem.todo)   return;

    if (is_alu(id_ex.IR.type)) {
        ex_mem.IR = id_ex.IR;
        //分情况计算
    }
    else if (is_load_store(id_ex.IR.type)) {
        ex_mem.IR = id_ex.IR;
        //分情况计算
    } else return;
}

void WB() {
    if (wb_idle)    return;
    if (!mem_wb.todo)   return;

    if (is_alu(id_ex.IR.type)) {
        ex_mem.IR = id_ex.IR;
        //分情况计算
    }
    else if (is_load_store(id_ex.IR.type)) {
        ex_mem.IR = id_ex.IR;
        //分情况计算
    } else return;
}

#endif //RISC_V_BASIC_HPP
