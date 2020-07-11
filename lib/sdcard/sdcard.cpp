#include "monty.h"
#include "config.h"

#if INCLUDE_SDCARD

#include <assert.h>
#include <jee.h>
#include <jee/spi-sdcard.h>

// the FatFS implementation in JeeH is a bit weird, unfortunately ...

static SpiGpio< PINS_SDCARD > spi;
static SdCard< decltype (spi) > sd;
static FatFS< decltype (sd) > fs;

struct FileObj : Object {
    static Value create (const TypeObj&, int argc, Value argv[]);
    static const LookupObj attrs;
    static TypeObj info;

    FileObj (const char* s) : file (fs), readQueue (0, 0) {
        limit = file.open(s);
    }

    TypeObj& type () const override;
    Value attr (const char* key, Value& self) const override;

    Value size ();
    Value read (int arg);
    Value close ();

private:
    FileMap< decltype (fs), 32 > file;
    ListObj readQueue; // TODO doesn't need to be a queue: fix suspend!
    size_t pos = 0;
    size_t limit;
};

Value FileObj::create (const TypeObj&, int argc, Value argv[]) {
    assert(argc == 2 && argv[1].isStr());
    return new FileObj (argv[1]);
}

Value FileObj::attr (const char* key, Value& self) const {
    self = Value::nil;
    return attrs.at(key);
}

Value FileObj::size () {
    return Value::nil;
}

Value FileObj::read (int arg) {
    assert(arg == 512);
    if (pos >= limit)
        return Value::nil;
    // TODO allocate buffer
    static uint8_t buf [512];
    auto ok = file.ioSect (false, pos/512, buf);
    assert(ok);
    pos += 512;
    return Value::nil; // TODO
}

Value FileObj::close () {
    return Value::nil;
}

static const auto m_size = MethObj::wrap(&FileObj::size);
static const MethObj mo_size = m_size;

static const auto m_read = MethObj::wrap(&FileObj::read);
static const MethObj mo_read = m_read;

static const auto m_close = MethObj::wrap(&FileObj::close);
static const MethObj mo_close = m_close;

static const LookupObj::Item fileMap [] = {
    { "size", &mo_size },
    { "read", &mo_read },
    { "close", &mo_close },
};

const LookupObj FileObj::attrs (fileMap, sizeof fileMap / sizeof *fileMap);

TypeObj FileObj::info ("<file>", FileObj::create, &FileObj::attrs);
TypeObj& FileObj::type () const { return info; }

static Value f_init (int argc, Value argv []) {
    assert(argc == 1);
    spi.init();
    if (!sd.init())
        return 0;
    fs.init();
    return 1;
}

static const FunObj fo_init = f_init;

static const LookupObj::Item lo_sdcard [] = {
    { "init", &fo_init },
    { "open", &FileObj::info },
};

static const LookupObj ma_sdcard (lo_sdcard, sizeof lo_sdcard / sizeof *lo_sdcard);
const ModuleObj m_sdcard (&ma_sdcard);

#endif // INCLUDE_SDCARD
