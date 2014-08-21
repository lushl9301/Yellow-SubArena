#include <iostream>
#include <stdlib.h>

#include "DataSmoothing.h"

#define N 7

using namespace std;

int main() {
    DataSmoothing ds;
    int inputs[N];
    int *ptr = inputs;
    srand(time(NULL));
    
    for (int i = 0; i < N; ++i) {
        inputs[i] = rand() % 10 + 295;
    }
    int head = 6;
    int r;
    
    for (int i = 0; i < 12; ++i) {
        r = rand() % 100 + 45 * i;
        inputs[head] = r;
        --head;
        if (head < 0) {
            head = 6;
        }
        ds.windowFilter7(inputs, head);
        //for (int j = 0; j < N; ++j)
        //    cout << 
        cout << r << "    lol    " << inputs[head + 1] << endl;
    }
    for (int i = 0; i < 20; ++i) {
        r = rand() % 300 + 50;
        inputs[head] = r;
        --head;
        if (head < 0) {
            head = 6;
        }
        ds.windowFilter7(inputs, head);
        //for (int j = 0; j < N; ++j)
        //    cout << 
        cout << r << "    lol    " << inputs[head + 1] << endl;
    }

}
