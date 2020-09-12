// json.cpp - json and ihex message parser, for use in input streams

#include "monty.h"
#include <cassert>

#include <cstdio>

using namespace Monty;

enum State {
    START, SKIP, IHEX, SEQEND, STR, NUMBER, WORD,
};

void InputParser::feed (uint8_t b) {
    switch (state) {
        case SKIP:
            if (b != '\n') return;
            state = START; // fall through
        case START:
            tag = b;
            switch (b) {
                case '\n': return;
                case ':':  fill = 0;
                           state = IHEX;
                           return;
                case '(':  // fall through
                case '[':  // fall through
                case '{':  stack.append(b);
                           stack.append(new List);
                           return;
                case ')':  // fall through
                case ']':  // fall through
                case '}':  break;
                case '\'': // fall through
                case '"':  val = new Bytes;
                           state = STR;
                           return;
                case '-':  b = '0'; // fall through
                default:   if ('0' <= b && b <= '9') {
                               u64 = b - '0';
                               state = NUMBER;
                           } else if ('a' <= b && b <= 'z') {
                               fill = 0;
                               buf[fill++] = b;
                               state = WORD;
                           } else
                               state = SKIP; // ignore until next newline
                           return;
            }
            break;

        case IHEX: {
            int n = fill >> 1;
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
                buf[n] = (buf[n] << 4) + ((b & 0x40 ? b + 9 : b) & 0x0F);
                ++fill;
            }
            return;
        }

        case STR:
            assert(false);
            return;

        case NUMBER:
            if ('0' <= b && b <= '9') {
                u64 = 10 * u64 + (b - '0');
                return;
            }

            val = Int::make(tag == '-' ? -u64 : u64);
            break;

        case WORD:
            if ('a' <= b && b <= 'z' && fill < sizeof buf - 2) {
                buf[fill++] = b;
                return;
            }

            buf[fill] = 0;
            if (strcmp((char*) buf, "null") == 0)
                val = Null;
            else if (strcmp((char*) buf, "false") == 0)
                val = False;
            else if (strcmp((char*) buf, "true") == 0)
                val = True;
            else
                val = {};
            break;

        case SEQEND:
            break;

        default:
            assert(false);
    }

    if (b == '\n') {
        onMsg(val);
        stack.remove(0, stack.fill);
        state = START;
        return;
    }

    auto& v = stack.pop(-1).asType<List>();
    if (!val.isNil())
        v.append(val);

    switch (b) {
        case ':':
            stack.setAt(-1, b); // mark as dict, not a set
        case ',':
            stack.append(v);
            break;
        case ')': // fall through
        case ']': // fall through
        case '}': {
            val = v;
            tag = stack.pop(-1);
            switch (tag) {
                case '(':
                    val = Tuple::create({v, (int) v.size(), 0});
                case '[':
                    break;
                case '{':
                    if (v.size() > 0) {
                        val = Set::create({v, (int) v.size(), 0});
                        break;
                    }
                    // fall through, empty set is turned into empty dict
                case ':':
                    if (v.size() & 1)
                        val = {}; // ignore malformed dict
                    else {
                        auto p = new Dict;
                        p->adj(v.size());
                        for (uint32_t i = 0; i < v.size(); i += 2)
                            p->at(v[i]) = v[i+1]; // TODO avoid lookups
                        val = p;
                    }
                    break;
                default:
                    assert(false);
            }
            if (stack.fill > 0) {
                state = SEQEND;
                return;
            }
            onMsg(val);
            break;
        }
        default:
            assert(false);
    }

    state = START;
}
