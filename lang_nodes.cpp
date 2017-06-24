#include "lang.h"

/**
 * LexTokenWrapper
 */
lang::LexTokenWrapper::LexTokenWrapper(const LexToken& token): token_(token){}

lang::LexToken lang::LexTokenWrapper::token() const { return token_; }

std::string lang::LexTokenWrapper::str() const { return lang::str(token_); }
