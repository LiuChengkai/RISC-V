#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <bitset>
#include "basic.hpp"
using namespace std;
typedef unsigned int uint;

void init_memory() {
    string str;

    int cnt = 0;
    int inst = 0;

    while (cin >> str) {
        if (str[0] == '@') {
            pc = 0;
            for (int i = 1; i <= 8; ++i) {
                if (str[i] >= '0' && str[i] <= '9')
                    pc = (pc << 4) + str[i] - '0';
                else
                    pc = (pc << 4) + str[i] - 'A' + 10;
            }
        } else {
            if (cnt == 0) {
                if (str[0] >= '0' && str[0] <= '9') {
                    inst += ((str[0] - '0') << 4);
                } else {
                    inst += ((str[0] - 'A' + 10) << 4);
                }
                if (str[1] >= '0' && str[1] <= '9') {
                    inst += str[1] - '0';
                } else {
                    inst += str[1] - 'A' + 10;
                }
                cnt++;
                continue;
            }
            if (cnt == 1) {
                if (str[0] >= '0' && str[0] <= '9') {
                    inst += ((str[0] - '0') << 12);
                } else {
                    inst += ((str[0] - 'A' + 10) << 12);
                }
                if (str[1] >= '0' && str[1] <= '9') {
                    inst += ((str[1] - '0') << 8);
                } else {
                    inst += ((str[1] - 'A' + 10) << 8);
                }
                cnt++;
                continue;
            }
            if (cnt == 2) {
                if (str[0] >= '0' && str[0] <= '9') {
                    inst += ((str[0] - '0') << 20);
                } else {
                    inst += ((str[0] - 'A' + 10) << 20);
                }
                if (str[1] >= '0' && str[1] <= '9') {
                    inst += ((str[1] - '0') << 16);
                } else {
                    inst += ((str[1] - 'A' + 10) << 16);
                }
                cnt++;
                continue;
            }
            if (cnt == 3) {
                if (str[0] >= '0' && str[0] <= '9') {
                    inst += ((str[0] - '0') << 28);
                } else {
                    inst += ((str[0] - 'A' + 10) << 28);
                }
                if (str[1] >= '0' && str[1] <= '9') {
                    inst += ((str[1] - '0') << 24);
                } else {
                    inst += ((str[1] - 'A' + 10) << 24);
                }
//                *(int *)(mem + pc) = inst;
                memcpy(mem + pc, &inst, 4);
                pc += 4;

                inst = 0;
                cnt = 0;
            }
        }
    }
}

void view_memory() {
    for (int i = 0; i < 0x20000; i += 4) {
        cout.fill('0');
        cout.width(8);
        int inst;
//        inst = *(int *)(mem + i);
        memcpy(&inst, mem + i, 4);
        if (inst) {
            cout << hex << inst << endl;
        }
    }
}

void run() {
    pc = 0;
    while (true) {
        if (!WB())
            break;

        if (is_load_store(EX_MEM.type)) {
            MEM();
            MEM2();
            MEM3();
        }
        else {
            MEM();
        }

        EX();

        if (!ID())
            continue;

        IF();
    }

    cout << (((uint)reg[10]) & 255u) << endl;
}

int main() {
//    freopen("pi.data", "r", stdin);
    init_memory();
//    view_memory();
    run();
}