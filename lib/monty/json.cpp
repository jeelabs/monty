// json.cpp - json and ihex message parser, for use in input streams

#include "monty.h"
#include <cassert>

#include <cstdio>

using namespace Monty;

enum State {
    START, SKIP, IHEX, LIST, MAP, STR, NUMBER, WORD,
};

void InputParser::feed (uint8_t b) {
    switch (state) {
        case SKIP:
            if (b != '\n') break;
            // else fall through
        case START:
            fill = 0;
            switch (b) {
                case ':': fill = 0; state = IHEX; break;
                case '[': state = LIST; break;
                case '{': state = MAP; break;
                case '"': state = STR; break;
                case '-': fill = 1; b = '0'; // fall through
                default:  u64 = b - '0';
                          if ('0' <= b && b <= '9')
                              state = NUMBER;
                          else if ('a' <= b && b <= 'z')
                              state = WORD, buf[fill++] = b;
                          else
                              state = SKIP; // ignore until next newline
            }
            break;

        case IHEX: {
            int n = fill / 2;
            if (b == '\n') {
                if (n > 0 && n == buf[0] + 5) {
                    uint8_t sum = 0;
                    for (int i = 0; i < n; ++i)
                        sum += buf[i];
                    if (sum == 0) // call with args: type, addr, data, size
                        onBuf(buf[3], 256*buf[1] + buf[2], buf + 4, buf[0]);
                    // else ignore errors
                }
                state = START;
            } else if (n < (int) sizeof buf) {
                // don't care about malformed data, but avoid buffer overruns
                auto i = b - '0';
                if (b > '9')
                    i -= 7;
                buf[n] = (buf[n] << 4) + (i & 0x0F);
                ++fill;
            }
            break;
        }

        case LIST:
            assert(false);
            break;

        case MAP:
            assert(false);
            break;

        case STR:
            assert(false);
            break;

        case NUMBER:
            if ('0' <= b && b <= '9')
                u64 = 10 * u64 + (b - '0');
            else if (b == '\n')
                onMsg(Int::make(fill ? -u64 : u64));
            else
                assert(false);
            break;

        case WORD:
            if ('a' <= b && b <= 'z' && fill < sizeof buf - 2)
                buf[fill++] = b;
            else if (b == '\n') {
                buf[fill] = 0;
                if (strcmp((char*) buf, "null") == 0)
                    onMsg(Null);
                else if (strcmp((char*) buf, "false") == 0)
                    onMsg(False);
                else if (strcmp((char*) buf, "true") == 0)
                    onMsg(True);
                else
                    assert(false);
                state = START;
            }
            break;

        default:
            assert(false);
    }
}
