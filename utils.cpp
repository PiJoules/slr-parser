#include "utils.h"

std::string join(const std::vector<std::string>& v, const std::string& delim) {
    std::string s;
    
    if (!v.empty()){
        s += v.front();
    }

    for (auto it = v.begin() + 1; it < v.end(); ++it){
        s += delim + *it;
    }

    return s;
}

std::string join(const std::vector<std::string>& v, const char* delim) {
    std::string delim_s(delim);
    return join(v, delim_s);
}

std::string join(const std::unordered_set<std::string>& s, const std::string& delim){
    const std::vector<std::string> v(s.begin(), s.end());
    return join(v, delim);
}

std::string join(const std::unordered_set<std::string>& s, const char* delim){
    std::string delim_s(delim);
    return join(s, delim_s);
}
