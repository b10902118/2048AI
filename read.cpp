#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int main() {
    ifstream f("traj.txt");
    unsigned long long s_after = 0, s_next;
    int action;
    f >> s_after;
    cout << s_after;
    // while (f >> s_after >> s_next >> action) {
    //     cout << s_after << s_next << action;
    // }
}