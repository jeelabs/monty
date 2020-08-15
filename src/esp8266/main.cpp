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

#include "monty.h"
#include "arch.h"
#include <cstdio>

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

void setup () {
    Serial.begin(115200);
    printf("main\n");

#if 0
    if (initWifi())
        printf("Wifi connected\n");
#else
    (void) initWifi;
#endif
    if (LittleFS.begin())
        printf("LittleFS mounted\n");

    const char* fname = "/demo.mpy";
    auto bcData = loadBytecode(fname);
    if (bcData == 0) {
        printf("can't open %s\n", fname);
        perror(fname);
        return;
    }

    static uintptr_t myMem [4096];
    Monty::setup(myMem, sizeof myMem);

    if (Monty::loadModule(bcData) == nullptr)
        printf("can't load module\n");
    else
        printf("done\n");

}

void loop () {}
