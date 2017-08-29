#ifndef _UTILS_H
#define _UTILS_H

#include <string>
#include <vector>
#include <unordered_set>

std::string join(const std::vector<std::string>& v, const std::string& delim);
std::string join(const std::vector<std::string>& v, const char* delim);
std::string join(const std::unordered_set<std::string>& s, const std::string& delim);
std::string join(const std::unordered_set<std::string>& s, const char* delim);

#endif
