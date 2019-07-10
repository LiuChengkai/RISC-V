#include <iostream>
#include <bitset>
using namespace std;

int main() {
    int a = -1;
    unsigned int u;
    u = a;
    cout << u << endl;
    cout << bitset<32>(u) << endl;
}