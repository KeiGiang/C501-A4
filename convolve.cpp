#include <stdio.h>
#include <iostream>
#include <fstream>

using namespace std;
// using std::cout;
// using std::endl;

int main(int argc, char* argv[]) {

    if (argc != 4) {
        cout << "Check runtime parameters" << endl;
        return -1;
    }

    cout << "Hello World" << endl;

    return 0;
}
