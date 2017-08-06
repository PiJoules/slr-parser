#ifndef _LANG_IO_H
#define _LANG_IO_H 

#include <iostream>

template <typename T>
void print(T t){
    std::cout << t << std::endl;
}

template <typename T, typename... Args>
void print(T t, Args... args){
    std::cout << t << " ";
    print(args...);
}

#endif
