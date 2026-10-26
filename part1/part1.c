#include <unistd.h>

int main()
{
    // exactly five system calls; only unistd.h is used to keep the trace short
    getpid();
    getppid();
    getuid();
    getgid();
    write(STDOUT_FILENO, "part1\n", 6);

    return 0;
}
