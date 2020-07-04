#include <FS.h>
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

#define printf Serial.printf

#include <assert.h>
#include <string.h>

#include "monty.h"
#include "arch.h"

#include "defs.h"
#include "qstr.h"
#include "builtin.h"
#include "interp.h"
#include "loader.h"

void Context::print (Value v) {
    switch (v.tag()) {
        case Value::Nil: printf("<nil>"); break;
        case Value::Int: printf("<Int %d>", (int) v); break;
        case Value::Str: printf("<Str '%s' at %p>",
                                 (const char*) v, (const char*) v); break;
        case Value::Obj: printf("<Obj %s at %p>",
                                 v.obj().type().name, &v.obj()); break;
    }
}

static const uint8_t* loadBytecode (const char* fname) {
    File f = SPIFFS.open(fname, "r");
    if (!f)
        return 0;
    size_t bytes = f.size();
    //printf("bytecode size %db\n", (int) bytes);
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

    mainMod->chain = &builtinDict;
    mainMod->atKey("__name__", DictObj::Set) = "__main__";
    mainMod->call(0, 0);

    vm.run();

    // must be placed here, before the vm destructor is called
    Object::gcStats();
    Context::gcTrigger();
    return true;
}

void setup () {
    Serial.begin(115200);
    if (initWifi())
        printf("Wifi connected\n");
    if (SPIFFS.begin())
        printf("SPIFFS mounted\n");

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

    printf("done\n");
}

void loop () {}
