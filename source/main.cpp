//
// Created by Bobby Lucero on 5/21/23.
//
#include "../headers/bob.h"

int main(int argc, char* argv[]){
    Bob bobLang;

    if(argc > 1) {
        bobLang.runFile(argv[1]);
    } else {
        bobLang.runPrompt();
    }

    return 0;
}
