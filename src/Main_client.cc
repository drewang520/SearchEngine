#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <vector>
#include <string>

int main() {
    const int CLIENT_NUM = 50;

    std::vector<int> write_fds;
    std::vector<pid_t> pids;

    for (int i = 0; i < CLIENT_NUM; ++i) {
        int fd[2];
        if (pipe(fd) == -1) {
            perror("pipe");
            return 1;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return 1;
        }

        if (pid == 0) {
            // ========== 子进程 ==========
            close(fd[1]);                 // 关闭写端
            dup2(fd[0], STDIN_FILENO);    // stdin ← pipe
            close(fd[0]);

            execl("../build/client", "client", nullptr);
            perror("execl");
            _exit(1);
        } else {
            // ========== 父进程 ==========
            close(fd[0]);      // 关闭读端
            write_fds.push_back(fd[1]);
            pids.push_back(pid);
        }
    }

    // ====== 父进程：从终端读，广播给所有 client ======
    std::string line;
    while (true) {
        std::cout << ">> input: ";
        if (!std::getline(std::cin, line)) {
            break;
        }

        line.push_back('\n');  // getline 需要换行

        for (int fd : write_fds) {
            write(fd, line.c_str(), line.size());
        }
    }

    // 关闭所有 client stdin
    for (int fd : write_fds) {
        close(fd);
    }

    for (pid_t pid : pids) {
        waitpid(pid, nullptr, 0);
    }

    return 0;
}

