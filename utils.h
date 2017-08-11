#ifndef _UTILS_H
#define _UTILS_H

#include <string>
#include <vector>

std::string join(const std::vector<std::string>& v, const std::string& delim);
std::string join(const std::vector<std::string>& v, const char* delim);

#endif
