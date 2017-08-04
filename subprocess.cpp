#include "subprocess.h"

#include <stdexcept>
#include <unistd.h>
#include <sys/wait.h>
#include <cassert>

#define READ_BUFF_SIZE 128

/* 
 * 3 pipes:
 * - Parent to child communication
 * - Child stdout to parent 
 * - Child stderr to parent
 */
#define NUM_PIPES 3
#define PARENT_TO_CHILD_PIPE 0
#define CHILD_STDOUT_PIPE 1
#define CHILD_STDERR_PIPE 2

#define READ_FD 0
#define WRITE_FD 1


std::string read_full_fd(int fd){
    char buffer[READ_BUFF_SIZE];
    std::string result;
    int bytes_read = read(fd, buffer, READ_BUFF_SIZE-1);
    while (bytes_read){
        if (bytes_read < 0){
            throw std::runtime_error("read error on stdout");
        }
        else {
            result.append(buffer, bytes_read);
        }
        bytes_read = read(fd, buffer, READ_BUFF_SIZE-1);
    }
    return result;
}

subprocess::CompletedProcess subprocess::Subprocess::run(const std::vector<std::string>& cmd){
    CompletedProcess completed;
    std::string stdout_result;
    std::string stderr_result;
    int returncode = 0;

    // Initialize pipes
    int pipes[NUM_PIPES][2];
    assert(!pipe(pipes[PARENT_TO_CHILD_PIPE]));
    assert(!pipe(pipes[CHILD_STDOUT_PIPE]));
    assert(!pipe(pipes[CHILD_STDERR_PIPE]));

    // Parent file descriptors
    int parent_write_fd = pipes[PARENT_TO_CHILD_PIPE][WRITE_FD];
    int parent_read_stdout_fd = pipes[CHILD_STDOUT_PIPE][READ_FD];  // What gets read from the child stdout
    int parent_read_stderr_fd = pipes[CHILD_STDERR_PIPE][READ_FD];

    // Child file descriptors 
    int child_read_fd = pipes[PARENT_TO_CHILD_PIPE][READ_FD];
    int child_write_stdout_fd = pipes[CHILD_STDOUT_PIPE][WRITE_FD];  // What gets written to the child stdout
    int child_write_stderr_fd = pipes[CHILD_STDERR_PIPE][WRITE_FD];

    pid_t pid = fork();
    if (!pid){
        // Child 
        // Run the command after converting the vector to a char** 
        std::vector<const char*> pointer_vec(cmd.size() + 1);
        for (std::size_t i = 0; i < cmd.size(); ++i){
            pointer_vec[i] = cmd[i].c_str();
        }
        pointer_vec.back() = 0;
        const char* const* c_cmd = pointer_vec.data();

        // Make what child reads in from child_read_fd
        // and child write to specified stdout/err write fd
        dup2(child_read_fd, STDIN_FILENO);
        dup2(child_write_stdout_fd, STDOUT_FILENO);
        dup2(child_write_stderr_fd, STDERR_FILENO);

        // None of these are required by child now 
        close(parent_write_fd);
        close(parent_read_stdout_fd);
        close(parent_read_stderr_fd);
        close(child_read_fd);
        close(child_write_stdout_fd);
        close(child_write_stderr_fd);

        // The const_cast looks bad, but execv only takes char *const argv[]
        // and not const char* argv[]. execv though does not actuallt change
        // the string array which is good.
        execvp(cmd.front().c_str(), const_cast<char* const*>(c_cmd));
    }
    else if (pid > 0){
        // Parent 
        close(child_read_fd);
        close(child_write_stdout_fd);
        close(child_write_stderr_fd);

        // Write to child stdin if any 
        // TODO: This later
        
        // Read from child stdout/err
        stdout_result = read_full_fd(parent_read_stdout_fd);
        stderr_result = read_full_fd(parent_read_stderr_fd);

        int waitstatus;
        wait(&waitstatus);
        returncode = WEXITSTATUS(waitstatus);

        close(parent_write_fd);
        close(parent_read_stdout_fd);
        close(parent_read_stderr_fd);
    }
    else {
        // Err 
        throw std::runtime_error("fork error");
    }

    completed.returncode = returncode;
    completed.stdout = stdout_result;
    completed.stderr = stderr_result;

    return completed;
}
