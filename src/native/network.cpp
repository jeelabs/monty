#include "monty.h"
#include "config.h"

#if INCLUDE_NETWORK

#include <assert.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

const int MAX_SOCK = 20;

static struct SocketObj* sockets [MAX_SOCK];

static Value f_ifconfig (int argc, Value argv []) {
    // nothing to do
    return Value::nil;
}

struct SocketObj : Object {
    static Value create (const TypeObj&, int argc, Value argv[]);
    static const LookupObj attrs;
    static TypeObj info;

    SocketObj (int sd) : sock (sd), readQueue (0, 0), writeQueue (0, 0) {
        int on = 1;
        auto r1 = setsockopt(sock, SOL_SOCKET,  SO_REUSEADDR, &on, sizeof on);
        auto r2 = ioctl(sock, FIONBIO, &on);
        assert(r1 == 0 && r2 == 0);
        addSock();
    }

    ~SocketObj () override { delSock(); }

    TypeObj& type () const override;
    void mark (void (*gc)(const Object&)) const override;
    Value attr (const char* key, Value& self) const override;

    Value bind (int arg);
    Value connect (int argc, Value argv []);
    Value listen (int argc, Value argv []);
    Value read (Value arg);
    Value write (Value arg);
    Value close ();

    static Value poll (int argc, Value argv []);
private:
    int sock;
    BytecodeObj* accepter = 0;
    ListObj readQueue, writeQueue; // TODO don't use queues: fix suspend!
    Value toSend = Value::nil;
    ArrayObj* recvBuf = 0;

    void addSock () {
        for (auto& s : sockets)
            if (s == 0) {
                s = this;
                break;
            }
    }

    void delSock () {
        for (auto& s : sockets)
            if (s == this)
                s = 0;
    }
};

Value SocketObj::create (const TypeObj&, int argc, Value argv[]) {
    assert(argc == 1);
    auto sd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sd >= 0);
    return new SocketObj (sd);
}

void SocketObj::mark (void (*gc)(const Object&)) const {
    gc(readQueue);
    gc(writeQueue);
    if (accepter != 0)
        gc(*accepter);
    if (toSend.isObj())
        gc(toSend.obj());
    if (recvBuf != 0)
        gc(*recvBuf);
}

Value SocketObj::attr (const char* key, Value& self) const {
    self = Value::nil;
    return attrs.at(key);
}

Value SocketObj::poll (int argc, Value argv []) {
    assert(argc == 1);

    fd_set fdIn;
    FD_ZERO(&fdIn);
    for (auto& s : sockets)
        if (s != 0 && (s->accepter != 0 || s->readQueue.len() > 0)) {
            FD_SET(s->sock, &fdIn);
        }

    static timeval tv; // cleared
    auto r = select(MAX_SOCK, &fdIn, 0, 0, &tv);
    assert(r >= 0);

    if (r > 0)
        for (auto& s : sockets) {
            if (s == 0 || !FD_ISSET(s->sock, &fdIn))
                continue;
            if (s->accepter != 0) {
                auto newsd = ::accept(s->sock, 0, 0);
                assert(newsd >= 0);
                Value argv = new SocketObj (newsd);
                Value v = s->accepter->call(1, &argv);
                assert(v.isObj());
                Context::tasks.append(v);
                continue;
            }
            if (s->readQueue.len() > 0) {
                uint8_t buf [100];
                auto len = ::recv(s->sock, buf, sizeof buf, 0);
                assert(len >= 0);
                if (len > 0) {
                    int n = s->recvBuf->write(buf, len);
                    assert(n == len); // TODO nope, read as much as needed!
                    Context::wakeUp(s->readQueue.pop(0), s->recvBuf);
                } else
                    s->close();
                continue;
            }
            assert(false); // TODO called addSock too soon?
        }

    return Value::nil;
}

Value SocketObj::bind (int arg) {
    sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(arg),
    };
    auto r = ::bind(sock, (sockaddr*) &addr, sizeof addr);
    assert(r == 0);
    return Value::nil;
}

Value SocketObj::connect (int argc, Value argv []) {
    assert(argc == 2 && argv[0].isStr() && argv[1].isInt());
    // TODO
    return Value::nil;
}

Value SocketObj::listen (int argc, Value argv[]) {
    assert(argc == 3 && argv[2].isInt());
    auto bco = argv[1].asType<BytecodeObj>();
    assert(bco != 0 && (bco->scope & 1) != 0); // make sure it's a generator

    auto r = ::listen(sock, argv[2]);
    assert(r == 0);
    accepter = bco;
    return Value::nil;
}

Value SocketObj::read (Value arg) {
    recvBuf = arg.isInt() ? new ArrayObj ('B', arg) : arg.asType<ArrayObj>();
    assert(recvBuf != 0 && recvBuf->isBuffer());
    Context::suspend(readQueue);
    return Value::nil;
}

Value SocketObj::write (Value arg) {
    const void* p;
    int n;
    if (arg.isStr()) {
        p = (const char*) arg;
        n = strlen(arg);
    } else {
        auto o = arg.asType<BytesObj>();
        assert(o != 0);
        p = (const uint8_t*) *o;
        n = o->len();
    }
    auto r = send(sock, p, n, 0);
    assert(r == n);
    return Value::nil;
}

Value SocketObj::close () {
    printf("\t CLOSE! %d r %d w %d\n",
            sock, (int) readQueue.len(), (int) writeQueue.len());
    delSock(); // TODO yuck ...
    ::close(sock);
    if (readQueue.len() > 0)
        readQueue.pop(0); // TODO throw Context::tasks.append(readQueue.pop(0));
    assert(readQueue.len() == 0);
    if (writeQueue.len() > 0)
        writeQueue.pop(0); // TODO throw Context::tasks.append(writeQueue.pop(0));
    assert(writeQueue.len() == 0);
    return Value::nil;
}

static const auto m_bind = MethObj::wrap(&SocketObj::bind);
static const MethObj mo_bind = m_bind;

static const auto m_connect = MethObj::wrap(&SocketObj::connect);
static const MethObj mo_connect = m_connect;

static const auto m_listen = MethObj::wrap(&SocketObj::listen);
static const MethObj mo_listen = m_listen;

static const auto m_read = MethObj::wrap(&SocketObj::read);
static const MethObj mo_read = m_read;

static const auto m_write = MethObj::wrap(&SocketObj::write);
static const MethObj mo_write = m_write;

static const auto m_close = MethObj::wrap(&SocketObj::close);
static const MethObj mo_close = m_close;

static const LookupObj::Item socketMap [] = {
    { "bind", &mo_bind },
    { "connect", &mo_connect },
    { "listen", &mo_listen },
    { "read", &mo_read },
    { "write", &mo_write },
    { "close", &mo_close },
};

const LookupObj SocketObj::attrs (socketMap, sizeof socketMap / sizeof *socketMap);

TypeObj SocketObj::info ("<socket>", SocketObj::create, &SocketObj::attrs);
TypeObj& SocketObj::type () const { return info; }

const FunObj fo_ifconfig = f_ifconfig;
const FunObj fo_poll = SocketObj::poll;

static const LookupObj::Item lo_network [] = {
    { "ifconfig", &fo_ifconfig },
    { "poll", &fo_poll },
    { "socket", &SocketObj::info },
};

static const LookupObj ma_network (lo_network, sizeof lo_network / sizeof *lo_network);
const ModuleObj m_network (&ma_network);

#endif // INCLUDE_NETWORK
