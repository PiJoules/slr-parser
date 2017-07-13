#include "lang.h"

/**
 * Exceptions
 */
const char* lang::IndentationError::what() const throw() {
    std::ostringstream err;
    err << std::runtime_error::what();
    err << "Unexpected indentation on line " << lineno_ << ".";
    return err.str().c_str();
}
