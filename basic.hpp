#ifndef RISC_V_NEW_HPP
#define RISC_V_NEW_HPP

#include <iostream>
#include <cstdio>
#include <cstring>
#include <bitset>
using namespace std;
typedef unsigned int uint;

//#define WBLOG
//#define LOG

char mem[0x20000];
int reg[32];
int pc = 0;
int round = 0;


void cout_hex(int t) {
    cout.width(8);
    cout.fill('0');
    cout << hex << t << endl;
    cout << dec;
}

void cerr_hex(int t) {
    cerr.width(8);
    cerr.fill('0');
    cerr << hex << t << endl;
    cerr << dec;
}

enum instT {
    LUI, AUIPC, JAL, JALR,  // 0~3
    BEQ, BNE, BLT, BGE, BLTU, BGEU, //4~9
    LB, LH, LW, LBU, LHU,  //10~14
    SB, SH, SW, //15~17
    ADDI, SLTI, SLTIU, XORI, ORI, ANDI, SLLI, SRLI, SRAI,   //18~26
    ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND,    //27~36
};

bool is_ALU(instT inst) {
    return (inst == LUI || inst == AUIPC || inst == ADDI || inst == SLTI || inst == SLTIU || inst == XORI || inst == ORI || inst == ANDI || inst == SLLI || inst == SRLI || inst == SRAI
            || inst == ADD || inst == SUB  || inst == SLL || inst == SLT || inst == SLTU || inst == XOR || inst == SRL || inst == SRA || inst == OR || inst == AND);
}

bool is_load_store(instT inst) {
    return (inst == LB || inst == LH || inst == LW || inst == LBU || inst == LHU || inst == SB || inst == SH || inst == SW);
}

bool is_load(instT inst) {
    return (inst == LB || inst == LH || inst == LW || inst == LBU || inst == LHU);
}

bool is_store(instT inst) {
    return (inst == SB || inst == SH || inst == SW);
}

bool is_branch(instT inst) {
    return (inst == JAL || inst == JALR || inst == BEQ || inst == BNE || inst == BLT || inst == BGE || inst == BLTU || inst == BGEU);
}

struct _IF_ID {
    int IR;
    int NPC;

} IF_ID;

struct _ID_EX {
    int IR;
    int NPC;
    int A, B;
    int imm;

    instT type;

    int rd, rs1, rs2;
    int shamt, opcode, funct3, funct7;

    void update() {
        rd = ((IR >> 7) & 0b11111);
        rs1 = ((IR >> 15) & 0b11111);
        rs2 = ((IR >> 20) & 0b11111);
        shamt = rs2;
        opcode = (IR & 0b1111111);
        funct3 = ((IR >> 12) & 0b111);
        funct7 = ((IR >> 30) & 1);
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
            case LUI: case AUIPC:
                imm = ((IR >> 12) << 12);
                break;

            case JAL:
                imm = ((((IR >> 31) & 1) << 31) >> 11);
                imm += ((IR >> 21) & (0b1111111111)) << 1;
                imm += ((IR >> 20) & (0b1)) << 11;
                imm += ((IR >> 12) & (0b11111111)) << 12;
                break;

            case JALR:
            case LB: case LH: case LW: case LBU: case LHU:
            case ADDI: case XORI: case ORI: case ANDI:
            case SLTIU:
                imm = IR >> 20;
                break;

            case BEQ: case BNE: case BLT: case BGE: case BLTU: case BGEU:
                imm = ((((IR >> 31) & 1) << 31) >> 19);
                imm += ((IR >> 7) & 0b1) << 11;
                imm += ((IR >> 25) & 0b111111) << 5;
                imm += ((IR >> 8) & 0b1111) << 1;
                break;

            case SB: case SH: case SW:
                imm = ((IR >> 25) << 5);
                imm += (IR >> 7) & 0b11111;
                break;

            default:;
        }
    }
} ID_EX;

struct _EX_MEM {
    int IR;
    int NPC;
    int ALUoutput;
    int B;
    int cond;
    int rd;

    instT type;
} EX_MEM;

struct _MEM_WB {
    int IR;
    int NPC;
    int ALUoutput;
    int LMD;
    int rd;

    instT type;
} MEM_WB;

void IF() {
    memcpy(&IF_ID.IR, mem + pc, 4);
    //处理control hazard
    if (EX_MEM.IR && is_branch(EX_MEM.type) && EX_MEM.cond) {
        pc = IF_ID.NPC = EX_MEM.ALUoutput;
        IF_ID.IR = 0;
        ID_EX.IR = 0;
    }
    else {
        IF_ID.NPC = pc = pc + 4;
    }
    return;
}

bool ID() {
    ID_EX.IR = IF_ID.IR;
    if (IF_ID.IR == 0)
        return 1;

    ID_EX.NPC = IF_ID.NPC;
    ID_EX.update();
    ID_EX.get_type();
    ID_EX.get_imm();

    //处理data hazard, rs1存在冲突时
    if (EX_MEM.IR && (is_ALU(EX_MEM.type) || is_load(EX_MEM.type))
        && EX_MEM.rd == ID_EX.rs1) {
        if (is_ALU(EX_MEM.type)) {
            ID_EX.A = EX_MEM.ALUoutput;
        }
        else if (is_load(EX_MEM.type)) {
            ID_EX.IR = 0;
            return 0;
        }
    }
    else if (MEM_WB.IR && (is_ALU(MEM_WB.type) || is_load(MEM_WB.type))
             && MEM_WB.rd == ID_EX.rs1) {
        if (is_ALU(MEM_WB.type)) {
            ID_EX.A = MEM_WB.ALUoutput;
        }
        else if (is_load(MEM_WB.type)) {
            ID_EX.A = MEM_WB.LMD;
        }
    }
    else {
        ID_EX.A = reg[ID_EX.rs1];
    }

    //处理data hazard, rs2存在冲突时
    if (EX_MEM.IR && (is_ALU(EX_MEM.type) || is_load(EX_MEM.type))
        && EX_MEM.rd == ID_EX.rs2) {
        if (is_ALU(EX_MEM.type)) {
            ID_EX.B = EX_MEM.ALUoutput;
        }
        else if (is_load(EX_MEM.type)) {
            ID_EX.IR = 0;
            return 0;
        }
    }
    else if (MEM_WB.IR && (is_ALU(MEM_WB.type) || is_load(MEM_WB.type))
             && MEM_WB.rd == ID_EX.rs2) {
        if (is_ALU(MEM_WB.type)) {
            ID_EX.B = MEM_WB.ALUoutput;
        }
        else if (is_load(MEM_WB.type)) {
            ID_EX.B = MEM_WB.LMD;
        }
    }
    else {
        ID_EX.B = reg[ID_EX.rs2];
    }


    IF_ID.IR = 0;
    return 1;
}


void ALU_EX() {
    switch (EX_MEM.type) {
        case LUI:
            EX_MEM.ALUoutput = ID_EX.imm;
            break;
        case AUIPC:
            EX_MEM.ALUoutput = ID_EX.NPC + ID_EX.imm - 4;
            break;
        case ADDI:
            EX_MEM.ALUoutput = ID_EX.A + ID_EX.imm;
            break;
        case SLTI:
            EX_MEM.ALUoutput = (ID_EX.A < ID_EX.imm);
            break;
        case SLTIU:
            EX_MEM.ALUoutput = ((uint) ID_EX.A < (uint) ID_EX.imm);
            break;
        case XORI:
            EX_MEM.ALUoutput = ID_EX.A ^ ID_EX.imm;
            break;
        case ORI:
            EX_MEM.ALUoutput = ID_EX.A | ID_EX.imm;
            break;
        case ANDI:
            EX_MEM.ALUoutput = ID_EX.A & ID_EX.imm;
            break;
        case SLLI:
            EX_MEM.ALUoutput = ID_EX.A << ID_EX.shamt;
            break;
        case SRLI:
            EX_MEM.ALUoutput = (uint) ID_EX.A >> ID_EX.shamt;
            break;
        case SRAI:
            EX_MEM.ALUoutput = ID_EX.A >> ID_EX.shamt;
            break;
        case ADD:
            EX_MEM.ALUoutput = ID_EX.A + ID_EX.B;
            break;
        case SUB:
            EX_MEM.ALUoutput = ID_EX.A - ID_EX.B;
            break;
        case SLL:
            EX_MEM.ALUoutput = ID_EX.A << ID_EX.B;
            break;
        case SLT:
            EX_MEM.ALUoutput = (ID_EX.A < ID_EX.B);
            break;
        case SLTU:
            EX_MEM.ALUoutput = ((uint) ID_EX.A < (uint) ID_EX.B);
            break;
        case XOR:
            EX_MEM.ALUoutput = ID_EX.A ^ ID_EX.B;
            break;
        case SRL:
            EX_MEM.ALUoutput = (uint) ID_EX.A >> (uint) ID_EX.B;
            break;
        case SRA:
            EX_MEM.ALUoutput = ID_EX.A >> ID_EX.B;
            break;
        case OR:
            EX_MEM.ALUoutput = ID_EX.A | ID_EX.B;
            break;
        case AND:
            EX_MEM.ALUoutput = ID_EX.A & ID_EX.B;
            break;
        default:;
    }
}

void LS_EX() {
    EX_MEM.ALUoutput = ID_EX.A + ID_EX.imm;
    EX_MEM.B = ID_EX.B;
}

void branch_EX() {
    //JALR是特殊情况，ALUOutput(pc)需要重新计算，其他指令类型只需要计算cond
    EX_MEM.ALUoutput = ID_EX.NPC - 4 + ID_EX.imm;
    switch (EX_MEM.type) {
        case JAL:
            EX_MEM.cond = 1;
            break;
        case JALR:
            EX_MEM.ALUoutput = (ID_EX.A + ID_EX.imm) & (-2);
            EX_MEM.cond = 1;
            break;
        case BEQ:
            EX_MEM.cond = (ID_EX.A == ID_EX.B);
            break;
        case BNE:
            EX_MEM.cond = (ID_EX.A != ID_EX.B);
            break;
        case BLT:
            EX_MEM.cond = (ID_EX.A < ID_EX.B);
            break;
        case BGE:
            EX_MEM.cond = (ID_EX.A >= ID_EX.B);
            break;
        case BLTU:
            EX_MEM.cond = ((uint)ID_EX.A < (uint)ID_EX.B);
            break;
        case BGEU:
            EX_MEM.cond = ((uint)ID_EX.A >= (uint)ID_EX.B);
            break;
        default:;
    }
}

void EX() {
    EX_MEM.IR = ID_EX.IR;
    if (ID_EX.IR == 0)
        return;
    if (ID_EX.IR == 0x00c68223)
        return;

    EX_MEM.NPC = ID_EX.NPC;
    EX_MEM.type = ID_EX.type;
    EX_MEM.rd = ID_EX.rd;

    if (is_ALU(EX_MEM.type)) {
        ALU_EX();
    }
    else if (is_load_store(EX_MEM.type)) {
        LS_EX();
    }
    else if (is_branch(EX_MEM.type)) {
        branch_EX();
    }

    ID_EX.IR = 0;
    return;
}

void load_MEM() {
    switch (MEM_WB.type) {
        case LB:
            int8_t lbtmp;
            memcpy(&lbtmp, mem + EX_MEM.ALUoutput, 1);
            MEM_WB.LMD = lbtmp;
            break;
        case LH:
            int16_t lhtmp;
            memcpy(&lhtmp, mem + EX_MEM.ALUoutput, 2);
            MEM_WB.LMD = lhtmp;
            break;
        case LW:
            int lwtmp;
            memcpy(&lwtmp, mem + EX_MEM.ALUoutput, 4);
            MEM_WB.LMD = lwtmp;
            break;
        case LBU:
            uint8_t lbutmp;
            memcpy(&lbutmp, mem + EX_MEM.ALUoutput, 1);
            MEM_WB.LMD = lbutmp;
            break;
        case LHU:
            uint16_t lhutmp;
            memcpy(&lhutmp, mem + EX_MEM.ALUoutput, 2);
            MEM_WB.LMD = lhutmp;
            break;
        default:;
    }
}

void store_MEM() {
    if (MEM_WB.type == SB) {
        int8_t sbtmp = EX_MEM.B;
        memcpy(mem + EX_MEM.ALUoutput, &sbtmp, 1);
    }
    else if (MEM_WB.type == SH) {
        int16_t shtmp = EX_MEM.B;
        memcpy(mem + EX_MEM.ALUoutput, &shtmp, 2);
    }
    else if (MEM_WB.type == SW) {
        int swtmp = EX_MEM.B;
        memcpy(mem + EX_MEM.ALUoutput, &swtmp, 4);
    }
}

void MEM() {
    MEM_WB.IR = EX_MEM.IR;
    if (EX_MEM.IR == 0)
        return;
    if (EX_MEM.IR == 0x00c68223)
        return;

    MEM_WB.NPC = EX_MEM.NPC;
    MEM_WB.type = EX_MEM.type;
    MEM_WB.rd = EX_MEM.rd;

    if (is_ALU(MEM_WB.type)) {
        MEM_WB.ALUoutput = EX_MEM.ALUoutput;
    }
    else if (is_load(MEM_WB.type)) {
        load_MEM();
    }
    else if (is_store(MEM_WB.type)) {
        store_MEM();
    }

    EX_MEM.IR = 0;
    return;
}

void MEM2() {}
void MEM3() {}

void view_reg() {
    cout << "type:" << MEM_WB.type << ' ';
    cout << round << ' ';
    cout << dec << MEM_WB.NPC - 4 << ' ';
    cout << dec;
    for (int i = 1; i < 32; ++i)
        cout << reg[i] << ' ';
    cout << endl;
}

bool WB() {
    if (MEM_WB.IR == 0) {
        return 1;
    }
    if (MEM_WB.IR == 0x00c68223)
        return 0;

    if (is_ALU(MEM_WB.type)) {
        reg[MEM_WB.rd] = MEM_WB.ALUoutput;
    }
    else if (is_load(MEM_WB.type)) {
        reg[MEM_WB.rd] = MEM_WB.LMD;
    }
    else if (MEM_WB.type == JAL || MEM_WB.type == JALR) {
        reg[MEM_WB.rd] = MEM_WB.NPC;
    }

#ifdef LOG
    ++round;
    view_reg();
#endif

    MEM_WB.IR = 0;
    return 1;
}

#endif //RISC_V_NEW_HPP
