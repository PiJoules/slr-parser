#ifndef _SUBPROCESS_H
#define _SUBPROCESS_H

#include <string>
#include <vector>

namespace subprocess {
    typedef struct CompletedProcess CompletedProcess;
    struct CompletedProcess {
        int returncode;
        std::string stdout;
        std::string stderr;
    };

    class Subprocess {
        public:
            CompletedProcess run(const std::vector<std::string>&);
    };
};

#endif
