#ifndef _UTILS_H
#define _UTILS_H

#include <string>
#include <vector>
#include <unordered_set>

/**
 * String join by a delimeter
 */
std::string join(const std::vector<std::string>& v, const std::string& delim);
std::string join(const std::vector<std::string>& v, const char* delim);
std::string join(const std::unordered_set<std::string>& s, const std::string& delim);
std::string join(const std::unordered_set<std::string>& s, const char* delim);

/**
 * Random
 */ 
std::string rand_alphanum_str(std::size_t len);

/**
 * String
 */ 
char circ_shift_alpha_char(char c);
char circ_shift_num_char(char c);

#endif
