#include "monty.h"
#include "config.h"

#if INCLUDE_SDCARD

#include <assert.h>
#include <jee.h>
#include <jee/spi-sdcard.h>

using namespace Monty;

// the FatFS implementation in JeeH is a bit weird, unfortunately ...

static SpiGpio< PINS_SDCARD > spi;
static SdCard< decltype (spi) > sd;
static FatFS< decltype (sd) > fs;

struct File : Object {
    static auto create (Context&,int,int,Type const* =nullptr) -> Value;
    static Lookup const attrs;
    static Type const info;
    auto type () const -> Type const& override { return info; }

    File (const char* s) : file (fs), readQueue (0, 0) {
        limit = file.open(s);
    }

    Value attr (const char* key, Value& self) const override;

    Value size () { return {}; }
    Value read (int arg);

private:
    FileMap< decltype (fs), 32 > file;
    List readQueue; // TODO doesn't need to be a queue: fix suspend!
    size_t pos = 0;
    size_t limit;
};

Value File::create (const Type&, int argc, Value argv[]) {
    assert(argc == 2 && argv[1].isStr());
    return new File (argv[1]);
}

Value File::attr (const char* key, Value& self) const {
    return attrs.at(key);
}

Value File::read (int arg) {
    assert(arg == 512);
    if (pos >= limit)
        return {};
    // TODO allocate buffer
    auto ok = file.ioSect (false, pos/512, fs.buf);
    (void) ok; assert(ok);
    pos += 512;
    return {}; // TODO
}

static const auto m_size = MethObj::wrap(&File::size);
static const MethObj mo_size = m_size;

static const auto m_read = MethObj::wrap(&File::read);
static const MethObj mo_read = m_read;

static const Lookup::Item fileMap [] = {
    { "size", &mo_size },
    { "read", &mo_read },
};

const Lookup File::attrs (fileMap, sizeof fileMap);

Type File::info ("<file>", File::create, &File::attrs);

static Value f_init (int argc, Value argv []) {
    assert(argc == 1);
    spi.init();
    if (!sd.init())
        return 0;
    fs.init();
    return 1;
}

static Value f_sdread (int argc, Value argv []) {
    assert(argc == 2 && argv[1].isInt());
    sd.read512(argv[1], fs.buf);
    // TODO return buf as bytes
    return {};
}

static Value f_sdwrite (int argc, Value argv []) {
    assert(argc == 3 && argv[1].isInt());
    // TODO copy arggv[2] to fs.buf
    sd.write512(argv[1], fs.buf);
    return {};
}

static const FunObj fo_init = f_init;
static const FunObj fo_sdread = f_sdread;
static const FunObj fo_sdwrite = f_sdwrite;

static const Lookup::Item lo_sdcard [] = {
    { "init", &fo_init },
    { "open", &File::info },
    { "sdread", &fo_sdread },
    { "sdwrite", &fo_sdwrite },
};

static const Lookup ma_sdcard (lo_sdcard, sizeof lo_sdcard);
const Module m_sdcard (&ma_sdcard);

#endif // INCLUDE_SDCARD
