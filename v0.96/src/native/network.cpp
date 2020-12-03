#include "monty.h"
#include "config.h"

#if INCLUDE_NETWORK

#include <cassert>
#include <cstdio>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

using namespace Monty;

static struct Socket* sockets [20];

static auto f_ifconfig (ArgVec const& args) -> Value {
    // nothing to do
    return {};
}

struct Socket : Object {
    static auto create (ArgVec const& args, Type const*) -> Value;
    static const Lookup attrs;
    static Type const info;

    Socket (int sd);
    ~Socket () override { dropFromPoll(); }

    Type const& type () const override;
    void marker () const override;
    Value attr (const char* key, Value& self) const override;

    auto bind (int arg) -> Value;
    auto connect (ArgVec const& args) -> Value;
    auto listen (ArgVec const& args) -> Value;
    auto read (Value arg) -> Value;
    auto write (Value arg) -> Value;
    auto close () -> Value;

    static auto poll (ArgVec const& args) -> Value;
private:
    void acceptSession ();
    void readData ();

    void addToPoll () {
        for (auto& s : sockets)
            if (s == 0) {
                s = this;
                break;
            }
    }

    void dropFromPoll () {
        for (auto& s : sockets)
            if (s == this)
                s = 0;
    }

    int sock;
    Callable* accepter = 0;
    List readQueue, writeQueue; // TODO don't use queues: fix suspend!
    Value toSend;
    Array* recvBuf = 0;
};

Socket::Socket (int sd) : sock (sd) {
    int on = 1;
    auto r1 = setsockopt(sock, SOL_SOCKET,  SO_REUSEADDR, &on, sizeof on);
    auto r2 = ioctl(sock, FIONBIO, &on);
    assert(r1 == 0 && r2 == 0);
    addToPoll();
}

auto Socket::create (ArgVec const& args, Type const*) -> Value {
    assert(args.num == 0);
    auto sd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sd >= 0);
    return new Socket (sd);
}

void Socket::marker () const {
    mark(readQueue);
    mark(writeQueue);
    mark(accepter);
    mark(toSend.obj());
    mark(recvBuf);
}

auto Socket::attr (const char* key, Value& self) const -> Value {
    return attrs.getAt(key);
}

auto Socket::poll (ArgVec const& args) -> Value {
    assert(args.num == 0);

    fd_set fdIn;
    FD_ZERO(&fdIn);
    int maxFd = 0;
    for (auto& s : sockets)
        if (s != 0 && (s->accepter != 0 || s->readQueue.len() > 0)) {
            FD_SET(s->sock, &fdIn);
            if (maxFd < s->sock)
                maxFd = s->sock;
        }

    static timeval tv; // cleared, i.e. immediate return
    if (select(maxFd + 1, &fdIn, 0, 0, &tv) > 0)
        for (auto& s : sockets)
            if (s != 0 && FD_ISSET(s->sock, &fdIn)) {
                if (s->accepter != 0)
                    s->acceptSession();
                else if (s->readQueue.len() > 0)
                    s->readData();
                else
                    assert(false);
            }

    return {};
}

auto Socket::bind (int arg) -> Value {
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(arg);
    auto r = ::bind(sock, (sockaddr*) &addr, sizeof addr);
    assert(r == 0);
    return {};
}

auto Socket::connect (ArgVec const& args) -> Value {
    assert(args.num == 2 && args[0].isStr() && args[1].isInt());
    // TODO
    return {};
}

auto Socket::listen (ArgVec const& args) -> Value {
    assert(args.num == 2 && args[1].isInt());
    auto& cao = args[0].asType<Callable>();
    assert(cao.bc.isGenerator());

    auto r = ::listen(sock, args[1]);
    assert(r == 0);
    accepter = &cao;
    return {};
}

void Socket::acceptSession () {
    auto newsd = ::accept(sock, 0, 0);
    assert(newsd >= 0);
    Value argv = new Socket (newsd);
    Value v = accepter->call(1, &argv);
    assert(!v.isNil());
    Interp::tasks.append(v);
}

auto Socket::read (Value arg) -> Value {
    recvBuf = arg.isInt() ? new Array ('B', arg) : &arg.asType<Array>();
    assert(recvBuf->isBuffer());
    Interp::suspend(readQueue);
    return {};
}

void Socket::readData () {
    uint8_t buf [100];
    auto len = ::recv(sock, buf, sizeof buf, 0);
    assert(len >= 0);
    if (len > 0) {
        int n = recvBuf->write(buf, len);
        //printf("n %d len %d\n", n, (int) len);
        assert(n == len); // TODO nope, read as much as needed!
        Context::wakeUp(readQueue.pop(0), recvBuf);
    } else
        close();
}

auto Socket::write (Value arg) -> Value {
    void const* p;
    int n;
    if (arg.isStr()) {
        p = (const char*) arg;
        n = strlen(arg);
    } else {
        auto& o = arg.asType<Bytes>();
        p = o.begin();
        n = o.size();
    }
    auto r = send(sock, p, n, 0);
    assert(r == n);
    return {};
}

auto Socket::close () -> Value {
    printf("\t CLOSE! %d r %d w %d\n",
            sock, (int) readQueue.len(), (int) writeQueue.len());
    dropFromPoll(); // TODO yuck ...
    ::close(sock);
    if (readQueue.len() > 0)
        readQueue.pop(0); // TODO throw Interp::tasks.append(readQueue.pop(0));
    assert(readQueue.len() == 0);
    if (writeQueue.len() > 0)
        writeQueue.pop(0); // TODO throw Interp::tasks.append(writeQueue.pop(0));
    assert(writeQueue.len() == 0);
    return {};
}

static const auto d_socket_bind = Method::wrap(&Socket::bind);
static const Method m_socket_bind = d_socket_bind;

static const auto d_socket_connect = Method::wrap(&Socket::connect);
static const Method m_socket_connect = d_socket_connect;

static const auto d_socket_listen = Method::wrap(&Socket::listen);
static const Method m_socket_listen = d_socket_listen;

static const auto d_socket_read = Method::wrap(&Socket::read);
static const Method m_socket_read = d_socket_read;

static const auto d_socket_write = Method::wrap(&Socket::write);
static const Method m_socket_write = d_socket_write;

static const auto d_socket_close = Method::wrap(&Socket::close);
static const Method m_socket_close = d_socket_close;

static const Lookup::Item socketMap [] = {
    { "bind", m_socket_bind },
    { "connect", m_socket_connect },
    { "listen", m_socket_listen },
    { "read", m_socket_read },
    { "write", m_socket_write },
    { "close", m_socket_close },
};

const Lookup Socket::attrs (socketMap, sizeof socketMap);

Type const Socket::info ("<socket>", Socket::create, &Socket::attrs);
auto Socket::type () const -> Type const& { return info; }

const Function fo_ifconfig = f_ifconfig;
const Function fo_poll = Socket::poll;

static const Lookup::Item lo_network [] = {
    { "ifconfig", fo_ifconfig },
    { "poll", fo_poll },
    { "socket", Socket::info },
};

static const Lookup ma_network (lo_network, sizeof lo_network);
const Module m_network (&ma_network);

#endif // INCLUDE_NETWORK
