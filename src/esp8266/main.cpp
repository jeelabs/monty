#include <LittleFS.h>
#include <ESP8266WiFi.h>

#include "../../../ssidpass.h"

static bool initWifi () {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        printf("can't connect to %s\n", WIFI_SSID);
        return false;
    }
    return true;
}

extern "C" int debugf (const char* fmt, ...) {
    va_list ap; va_start(ap, fmt);
    vprintf(fmt, ap); va_end(ap);
    return 0;
}

#include <assert.h>
#include <string.h>

#include "monty.h"
#include "arch.h"

#include "defs.h"
#include "qstr.h"
#include "builtin.h"
#include "interp.h"
#include "loader.h"

#include "xmonty.h"

static const uint8_t* loadBytecode (const char* fname) {
    File f = LittleFS.open(fname, "r");
    if (!f)
        return 0;
    size_t bytes = f.size();
    printf("bytecode size %db\n", (int) bytes);
    auto buf = (uint8_t*) malloc(bytes);
    auto len = f.read(buf, bytes);
    f.close();
    if (len == bytes)
        return buf;
    free(buf);
    return 0;
}

static bool runInterp (const uint8_t* data) {
    Interp vm;

    ModuleObj* mainMod = 0;
    if (data[0] == 'M' && data[1] == 5) {
        Loader loader;
        mainMod = loader.load (data);
        vm.qPool = loader.qPool;
    }

    if (mainMod == 0)
        return false;

    vm.start(*mainMod, builtinDict);

    while (vm.isAlive())
        vm.run();

    // must be placed here, before the vm destructor is called
    Object::gcStats();
    Context::gcTrigger();
    return true;
}

void setup () {
    Serial.begin(115200);
    printf("main qstr #%d %db %s\n",
            (int) qstrNext, (int) sizeof qstrData, VERSION);

    if (initWifi())
        printf("Wifi connected\n");
    if (LittleFS.begin())
        printf("LittleFS mounted\n");

    const char* fname = "/demo.mpy";
    auto bcData = loadBytecode(fname);
    if (bcData == 0) {
        printf("can't open %s\n", fname);
        perror(fname);
        return;
    }

    if (!runInterp(bcData)) {
        printf("can't load bytecode\n");
        return;
    }

    // TODO load using the new code ...
    static uintptr_t myMem [4096];
    Monty::setup(myMem, sizeof myMem);
    printf("new load %p\n", Monty::loadModule(bcData));

    printf("done\n");
}

void loop () {}
