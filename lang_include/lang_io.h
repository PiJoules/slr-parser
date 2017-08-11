#ifndef _LANG_IO_H
#define _LANG_IO_H 

#include <iostream>
#include <string>

template <typename T>
void print(T t){
    std::cout << t << std::endl;
}

template <typename T, typename... Args>
void print(T t, Args... args){
    std::cout << t << " ";
    print(args...);
}

class str {
    private:
        std::string value_;

    public:
        str(const char* value): value_(value){}
        str(const std::string& value): value_(value){}
        str(const str& other): value_(other.value()){}

        std::string value() const { return value_; }

        str operator+(const str& rhs) const;
};

str operator+(const char* lhs, const str& rhs);
std::ostream& operator<<(std::ostream& out, const str& s);

str input(std::string const& s);
str input(const char* s);

#endif
