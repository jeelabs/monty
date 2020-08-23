// This is  a bit like cat - see https://stackoverflow.com/questions/12984816
//
// Read output from serial until it's idle for more than IDLE_MS milliseconds.
// The first character read is blocking, the timeout only applies afterwards.
//
// --jcw, 2020-06-22

#include <poll.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#define IDLE_MS 250

int main (int argc, const char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <outfile>\n", argv[0]);
        return 1;
    }

    FILE* fp = fopen(argv[1], "w");
    if (fp == 0) {
        perror(argv[1]);
        return 2;
    }

    if (isatty(0)) {
        struct termios ios;
        tcgetattr(0, &ios);
        cfsetspeed(&ios, B115200);
        cfmakeraw(&ios);
        tcsetattr(0, TCSAFLUSH, &ios);
    }

    struct pollfd fd;
    fd.fd = 0;
    fd.events = POLLIN;

    do { // first byte read is blocking
        char c;
        if (read(0, &c, 1) < 1 || c == 0)
            break;
        if (c & 0x80) // restart output when garbage (or start char) is received
            freopen(argv[1], "w", fp);
        else
            fputc(c, fp);
    } while (poll(&fd, 1, IDLE_MS) > 0);

    fclose(fp);
    return 0;
}
