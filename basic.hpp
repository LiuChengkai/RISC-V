#ifndef RISC_V_BASIC_HPP
#define RISC_V_BASIC_HPP
#include <iostream>
#include <cstdio>
#include <cstring>
#include <bitset>
using namespace std;
typedef unsigned int uint;
//#define LOG


char mem[0x20000];
int reg[32];
int pc = 0;
int round = 0;

enum instT {
    LUI, AUIPC, JAL, JALR,  // 0~3
    BEQ, BNE, BLT, BGE, BLTU, BGEU, //4~9
    LB, LH, LW, LBU, LHU,  //10~14
    SB, SH, SW, //15~17
    ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI,   //18~26
    ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND,    //27~36
};

void view_reg() {
    cout << hex << pc << ' ';
    cout << dec;
    for (int i = 1; i < 32; ++i)
        cout << reg[i] << ' ';
    cout << endl;
}

struct Instruction {
    int inst, imm;
    instT type;
    int rd, rs1, rs2, shamt, opcode, funct3, funct7;

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
                //todo here I changed
                imm = inst >> 20;
                break;

                //imm[12|10:5], imm[4:1|11]
            case BEQ: case BNE: case BLT: case BGE: case BLTU: case BGEU:
                //todo here I changed
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

    void IF() {
        memcpy(&inst, mem + pc, 4);
    }

    void ID() {
        update();
        get_type();
        get_imm();
    }


    bool cal_type() {
        bool flag = true;
        switch (type) {
            case LUI:
                reg[rd] = imm;
#ifdef LOG
                printf("Round%d, LUI:\n", round);
                view_reg();
#endif
                break;

            case AUIPC:
                pc = pc + imm;
                reg[rd] = pc;
#ifdef LOG
                printf("Round%d, AUIPC:\n", round);
                view_reg();
#endif
                break;

            case ADDI:
                reg[rd] = reg[rs1] + imm;
#ifdef LOG
                printf("Round%d, ADDI:\n", round);
                view_reg();
#endif
                break;

            case SLTI:
                if (reg[rs1] < imm)
                    reg[rd] = 1;
                else
                    reg[rd] = 0;
#ifdef LOG
                printf("Round%d, SLTI:\n", round);
                view_reg();
#endif
                break;

            case SLTIU:
                if ((uint)reg[rs1] < (uint)imm)
                    reg[rd] = 1;
                else
                    reg[rd] = 0;
#ifdef LOG
                printf("Round%d, SLTIU:\n", round);
                view_reg();
#endif
                break;

            case XORI:
                reg[rd] = reg[rs1] ^ imm;
#ifdef LOG
                printf("Round%d, XORI:\n", round);
                view_reg();
#endif
                break;

            case ORI:
                reg[rd] = reg[rs1] | imm;
#ifdef LOG
                printf("Round%d, ORI:\n", round);
                view_reg();
#endif
                break;

            case ANDI:
                reg[rd] = reg[rs1] & imm;
#ifdef LOG
                printf("Round%d, ANDI:\n", round);
                view_reg();
#endif
                break;

            case SLLI:
                reg[rd] = reg[rs1] << shamt;
#ifdef LOG
                printf("Round%d, SLLI:\n", round);
                view_reg();
#endif
                break;

            case SRLI:
                reg[rd] = (uint) reg[rs1] >> shamt;
#ifdef LOG
                printf("Round%d, SRLI:\n", round);
                view_reg();
#endif
                break;

            case SRAI:
                reg[rd] = reg[rs1] >> shamt;
#ifdef LOG
                printf("Round%d, SRAI:\n", round);
                view_reg();
#endif
                break;

            case ADD:
                reg[rd] = reg[rs1] + reg[rs2];
#ifdef LOG
                printf("Round%d, ADD:\n", round);
                view_reg();
#endif
                break;

            case SUB:
                reg[rd] = reg[rs1] - reg[rs2];
#ifdef LOG
                printf("Round%d, SUB:\n", round);
                view_reg();
#endif
                break;

            case SLL:
                reg[rd] = reg[rs1] << reg[rs2];
#ifdef LOG
                printf("Round%d, SLL:\n", round);
                view_reg();
#endif
                break;

            case SLT:
                if (reg[rs1] < reg[rs2])
                    reg[rd] = 1;
                else
                    reg[rd] = 0;
#ifdef LOG
                printf("Round%d, SLT:\n", round);
                view_reg();
#endif
                break;

            case SLTU:
                if ((uint)reg[rs1] < (uint)reg[rs2])
                    reg[rd] = 1;
                else
                    reg[rd] = 0;
#ifdef LOG
                printf("Round%d, SLTU:\n", round);
                view_reg();
#endif
                break;

            case XOR:
                reg[rd] = reg[rs1] ^ reg[rs2];
#ifdef LOG
                printf("Round%d, XOR:\n", round);
                view_reg();
#endif
                break;

            case SRL:
                reg[rd] = (uint)reg[rs1] >> (uint)reg[rs2];
#ifdef LOG
                printf("Round%d, SRL:\n", round);
                view_reg();
#endif
                break;

            case SRA:
                reg[rd] = reg[rs1] >> reg[rs2];
#ifdef LOG
                printf("Round%d, SRA:\n", round);
                view_reg();
#endif
                break;

            case OR:
                reg[rd] = reg[rs1] | reg[rs2];
#ifdef LOG
                printf("Round%d, OR:\n", round);
                view_reg();
#endif
                break;

            case AND:
                reg[rd] = reg[rs1] & reg[rs2];
#ifdef LOG
                printf("Round%d, AND:\n", round);
                view_reg();
#endif
                break;

            default:
                flag = false;

        }
        if (flag)
            pc = pc + 4;
        return flag;
    }

    bool jump_type() {
        bool flag = true;
        switch (type) {
            case JAL:
                reg[rd] = pc + 4;
                pc = pc + imm;
#ifdef LOG
                printf("Round%d, JAL:\n", round);
                view_reg();
#endif
                break;

            case JALR:
                reg[rd] = pc + 4;
                pc = (reg[rs1] + imm) & (-2);
#ifdef LOG
                printf("Round%d, JALR:\n", round);
                view_reg();
#endif
                break;

            case BEQ:
                if (reg[rs1] == reg[rs2])
                    pc = pc + imm;
                else
                    pc = pc + 4;
#ifdef LOG
                printf("Round%d, BEQ:\n", round);
                view_reg();
#endif
                break;

            case BNE:
                if (reg[rs1] != reg[rs2])
                    pc = pc + imm;
                else
                    pc = pc + 4;
#ifdef LOG
                printf("Round%d, BNE:\n", round);
                view_reg();
#endif
                break;

            case BLT:
                if (reg[rs1] < reg[rs2])
                    pc = pc + imm;
                else
                    pc = pc + 4;
#ifdef LOG
                printf("Round%d, BLT:\n", round);
                view_reg();
#endif
                break;

            case BGE:
                if (reg[rs1] >= reg[rs2])
                    pc = pc + imm;
                else
                    pc = pc + 4;
#ifdef LOG
                printf("Round%d, BGE:\n", round);
                view_reg();
#endif
                break;

            case BLTU:
                if ((uint)reg[rs1] < (uint)reg[rs2]) {
                    pc = pc + imm;
                }
                else
                    pc = pc + 4;
#ifdef LOG
                printf("Round%d, BLTU:\n", round);
                view_reg();
#endif
                break;

            case BGEU:
                if ((uint)reg[rs1] >= (uint)reg[rs2])
                    pc = pc + imm;
                else
                    pc = pc + 4;
#ifdef LOG
                printf("Round%d, BGEU:\n", round);
                view_reg();
#endif
                break;

            default:
                flag = false;
        }
        return flag;
    }

    bool load_type() {
        bool flag = true;
        switch (type) {
            case LB:
                int8_t lbtmp;
                memcpy(&lbtmp, mem + reg[rs1] + imm, 1);
                reg[rd] = lbtmp;
#ifdef LOG
                printf("Round%d, LB:\n", round);
                view_reg();
#endif
                break;

            case LH:
                int16_t lhtmp;
                memcpy(&lhtmp, mem + reg[rs1] + imm, 2);
                reg[rd] = lhtmp;
#ifdef LOG
                printf("Round%d, LH:\n", round);
                view_reg();
#endif
                break;

            case LW:
                int lwtmp;
                memcpy(&lwtmp, mem + reg[rs1] + imm, 4);
                reg[rd] = lwtmp;
#ifdef LOG
                printf("Round%d, LW:\n", round);
                view_reg();
#endif
                break;

            case LBU:
                uint8_t lbutmp;
                memcpy(&lbutmp, mem + reg[rs1] + imm, 1);
                reg[rd] = lbutmp;
#ifdef LOG
                printf("Round%d, LBU:\n", round);
                view_reg();
#endif
                break;

            case LHU:
                uint16_t lhutmp;
                memcpy(&lhutmp, mem + reg[rs1] + imm, 2);
                reg[rd] = lhutmp;
#ifdef LOG
                printf("Round%d, LHU:\n", round);
                view_reg();
#endif
                break;

            default:
                flag = false;
        }
        if (flag)
            pc = pc + 4;
        return flag;
    }

    bool store_type() {
        bool flag = true;
        if (type == SB) {
            int8_t sbtmp = reg[rs2];
            memcpy(mem + reg[rs1] + imm, &sbtmp, 1);
#ifdef LOG
            printf("Round%d, SB:\n", round);
            view_reg();
#endif
        }
        else if (type == SH) {
            int16_t sbtmp = reg[rs2];
            memcpy(mem + reg[rs1] + imm, &sbtmp, 1);
#ifdef LOG
            printf("Round%d, SB:\n", round);
            view_reg();
#endif
        }
        else if (type == SW) {
            int swtmp = reg[rs2];
            memcpy(mem + reg[rs1] + imm, &swtmp, 4);
#ifdef LOG
            printf("Round%d, SW:\n", round);
            view_reg();
#endif
        }
        else {
            flag = false;
        }
        if (flag)
            pc = pc + 4;
        return flag;
    }

    bool type_flag;
    void EX() {
//        cal_type();
//        jump_type();
//        load_type();
//        store_type();
        type_flag = cal_type();
        if (!type_flag)
            type_flag = jump_type();
        else
            return;
        if (!type_flag)
            type_flag = load_type();
        else
            return;
        if (!type_flag)
            type_flag = store_type();
        else
            return;

        if (!type_flag)
            cerr << "error in EX" << endl;
    }

    void MA() {

    }

    void MA2() {

    }

    void MA3() {

    }

    void WB() {

    }
};

#endif //RISC_V_BASIC_HPP
