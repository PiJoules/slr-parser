#include "lang_io.h"

str str::operator+(const str& rhs) const {
    str new_str(value_ + rhs.value());
    return new_str;
}

str operator+(const char* lhs, const str& rhs){
    str wrapper(lhs);
    return wrapper + rhs;
}

std::ostream& operator<<(std::ostream& out, const str& s){
    out << s.value();
    return out;
}

str input(const char* s){ 
    std::string wrapped = s;
    return input(wrapped); 
}

str input(std::string const& s){
    std::cout << s;
    std::string in_str;
    std::cin >> in_str;

    str wrapped(in_str);
    return wrapped;
}
