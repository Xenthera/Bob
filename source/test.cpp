#include "../headers/test.h"

Test::Test(std::string msg){
    this->message = msg;
}

void Test::Hello(){
    std::cout << this->message << std::endl;
}