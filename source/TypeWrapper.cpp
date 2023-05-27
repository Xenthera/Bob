//
// Created by Bobby Lucero on 5/27/23.
//
#include "../headers/TypeWrapper.h"
#include <iostream>

int Object::count;

Object::Object() {
    std::cout << "Inc"<<std::endl;
    Object::count += 1;
}

Object::~Object() {
    std::cout << "dec"<<std::endl;

    Object::count -= 1;
}
