#include "monty.h"
#include "config.h"

#if INCLUDE_SDCARD

#include <assert.h>
#include <jee.h>
#include <jee/spi-sdcard.h>

using namespace Monty;

// the FatFS implementation in JeeH is a bit weird, unfortunately ...

static SpiGpio< SDCARD_SPI_PINS > spi;
static SdCard< decltype (spi) > sd;
static FatFS< decltype (sd) > fs;

struct File : Object {
    static auto create (ArgVec const&, Type const*) -> Value;
    static Lookup const attrs;
    static Type const info;
    auto type () const -> Type const& override { return info; }

    File (const char* s) : file (fs), limit (file.open(s)) {}

    auto attr (const char* key, Value& self) const -> Value override;

    auto size () -> Value { return {}; }
    auto read (int arg) -> Value;

private:
    FileMap< decltype (fs), 32 > file;
    List readQueue; // TODO doesn't need to be a queue: fix suspend!
    size_t pos {0};
    size_t limit;
};

auto File::create (ArgVec const& args, Type const*) -> Value {
    assert(args.num == 1 && args[0].isStr());
    return new File (args[0]);
}

auto File::attr (const char* key, Value& self) const -> Value {
    return attrs.getAt(key);
}

auto File::read (int arg) -> Value {
    assert(arg == 512);
    if (pos >= limit)
        return {};
    // TODO allocate buffer
    auto ok = file.ioSect (false, pos/512, fs.buf);
    (void) ok; assert(ok);
    pos += 512;
    return {}; // TODO
}

static const auto d_file_size = Method::wrap(&File::size);
static const Method m_file_size = d_file_size;

static const auto d_file_read = Method::wrap(&File::read);
static const Method m_file_read = d_file_read;

static const Lookup::Item fileMap [] = {
    { "size", &m_file_size },
    { "read", &m_file_read },
};

const Lookup File::attrs (fileMap, sizeof fileMap);

Type const File::info ("<file>", File::create, &File::attrs);

static auto f_init (ArgVec const& args) -> Value {
    assert(args.num == 0);
    spi.init();
    if (!sd.init())
        return 0;
    fs.init();
    return 1;
}

static auto f_sdread (ArgVec const& args) -> Value {
    assert(args.num == 1 && args[0].isInt());
    sd.read512(args[0], fs.buf);
    // TODO return buf as bytes
    return {};
}

static auto f_sdwrite (ArgVec const& args) -> Value {
    assert(args.num == 2 && args[0].isInt());
    // TODO copy arggv[1] to fs.buf
    sd.write512(args[0], fs.buf);
    return {};
}

static const Function fo_init = f_init;
static const Function fo_sdread = f_sdread;
static const Function fo_sdwrite = f_sdwrite;

static const Lookup::Item lo_sdcard [] = {
    { "init", fo_init },
    { "open", File::info },
    { "sdread", fo_sdread },
    { "sdwrite", fo_sdwrite },
};

static const Lookup ma_sdcard (lo_sdcard, sizeof lo_sdcard);
const Module m_sdcard (&ma_sdcard);

#endif // INCLUDE_SDCARD
