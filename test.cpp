#include <iostream>
#include <sstream>
#include <ios>
#include <fstream>

class A {
    public:
        std::stringstream ss(std::ios_base::ate);
};

int main(){
    A a();
    return 0;
}
