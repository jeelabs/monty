#include "monty.h"
#include "config.h"

#if INCLUDE_NETWORK

#include <assert.h>
#include <sys/socket.h>

static Value f_poll (int argc, Value argv []) {
    assert(argc == 1);
    // TODO
    return Value::nil;
}

static Value f_ifconfig (int argc, Value argv []) {
    static bool inited = false;
    if (!inited) {
        inited = true;
        // TODO
    }

    assert(argc == 5);
#if 0
    ip4_addr ifconf [4];
    for (int i = 0; i < 4; ++i) {
        assert(argv[i+1].isStr());
        auto ok = ip4addr_aton(argv[i+1], ifconf + i);
        (void) ok; assert(ok == 1);
    }
#endif

    // TODO

    return Value::nil;
}

struct SocketObj : Object {
    static Value create (const TypeObj&, int argc, Value argv[]);
    static const LookupObj attrs;
    static TypeObj info;

    SocketObj (void* p) : socket (p), readQueue (0, 0),
                             writeQueue (0, 0), toSend (Value::nil) {
        // TODO tcp_arg(socket, this);
    }

    TypeObj& type () const override;
    void mark (void (*gc)(const Object&)) const override;
    Value attr (const char* key, Value& self) const override;

    Value bind (int arg);
    Value connect (int argc, Value argv []);
    Value listen (int arg);
    Value accept (Value arg);
    Value read (Value arg);
    Value write (Value arg);
    Value close ();

private:
    void* socket;
    BytecodeObj* accepter = 0;
    ListObj readQueue, writeQueue; // TODO don't use queues: fix suspend!
    Value toSend;
    ArrayObj* recvBuf = 0;
};

Value SocketObj::create (const TypeObj&, int argc, Value argv[]) {
    assert(argc == 1);
    void* p = 0; // TODO tcp_new();
    assert(p != 0);
    return new SocketObj (p);
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

Value SocketObj::bind (int arg) {
    // TODO
    return Value::nil;
}

Value SocketObj::connect (int argc, Value argv []) {
    assert(argc == 2 && argv[0].isStr() && argv[1].isInt());
    // TODO
    return Value::nil;
}

Value SocketObj::listen (int arg) {
    // TODO
    return Value::nil;
}

Value SocketObj::accept (Value arg) {
    auto bco = arg.asType<BytecodeObj>();
    assert(bco != 0 && (bco->scope & 1) != 0); // make sure it's a generator
    accepter = bco;

    // TODO
    return Value::nil;
}

Value SocketObj::read (Value arg) {
    recvBuf = arg.isInt() ? new ArrayObj ('B', arg) : arg.asType<ArrayObj>();
    assert(recvBuf != 0 && recvBuf->isBuffer());
    Context::suspend(readQueue);

    // TODO
    return Value::nil;
}

Value SocketObj::write (Value arg) {
    assert(toSend.isNil()); // don't allow multiple outstanding sends
    // TODO
    return Value::nil;
}

Value SocketObj::close () {
    //printf("\t CLOSE! %p r %d w %d\n",
    //        socket, (int) readQueue.len(), (int) writeQueue.len());
    // TODO
    toSend = Value::nil;
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

static const auto m_accept = MethObj::wrap(&SocketObj::accept);
static const MethObj mo_accept = m_accept;

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
    { "accept", &mo_accept },
    { "read", &mo_read },
    { "write", &mo_write },
    { "close", &mo_close },
};

const LookupObj SocketObj::attrs (socketMap, sizeof socketMap / sizeof *socketMap);

TypeObj SocketObj::info ("<socket>", SocketObj::create, &SocketObj::attrs);
TypeObj& SocketObj::type () const { return info; }

const FunObj fo_poll = f_poll;
const FunObj fo_ifconfig = f_ifconfig;

static const LookupObj::Item lo_network [] = {
    { "ifconfig", &fo_ifconfig },
    { "poll", &fo_poll },
    { "socket", &SocketObj::info },
};

static const LookupObj ma_network (lo_network, sizeof lo_network / sizeof *lo_network);
const ModuleObj m_network (&ma_network);

#endif // INCLUDE_NETWORK
