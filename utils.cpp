#include "utils.h"
#include <cstdlib>
#include <cctype>

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

static const std::string NUM_CHARS = "0123456789";
static const std::string UPPERCASE_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const std::string LOWERCASE_CHARS = "abcdefghijklmnopqrstuvwxyz";
static const std::string ALNUM_CHARS = NUM_CHARS + UPPERCASE_CHARS + LOWERCASE_CHARS;

std::string rand_alphanum_str(std::size_t len){
    std::string s(len, 0);
    for (std::size_t i = 0; i < len; ++i){
        s[i] = ALNUM_CHARS[rand() % ALNUM_CHARS.size()];
    }
    return s;
}

/**
 * It is assumed that c is an alphabetic character.
 */
char circ_shift_alpha_char(char c){
    if (isupper(c)){
        std::size_t i = c - 'A';
        return UPPERCASE_CHARS[(i+1) % UPPERCASE_CHARS.size()];
    }
    else {
        std::size_t i = c - 'a';
        return LOWERCASE_CHARS[(i+1) % LOWERCASE_CHARS.size()];
    }
}

/**
 * It is assumed that c is a numeric character.
 */
char circ_shift_num_char(char c){
    std::size_t i = c - '0';
    return NUM_CHARS[(i+1) % NUM_CHARS.size()];
}
