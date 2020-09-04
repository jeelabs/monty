#include "monty.h"
#include "arch.h"

#include <cassert>
#include "pyvm.h"

#include "../../ssidpass.h"

#include <ESP8266WebServer.h>
#include <LittleFS.h>

ESP8266WebServer server (80);

static const uint8_t* loadBytecode (const char* fname) {
    File f = LittleFS.open(fname, "r");
    if (!f)
        return 0;
    uint32_t bytes = f.size();
    printf("bytecode size %db\n", (int) bytes);
    auto buf = (uint8_t*) malloc(bytes);
    auto len = f.read(buf, bytes);
    f.close();
    if (len == bytes)
        return buf;
    free(buf);
    return 0;
}

void handleNotFound(){
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += server.method() == HTTP_GET ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); ++i)
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    server.send(404, "text/plain", message);
}

void setup () {
    Serial.begin(115200);
    printf("main\n");

    if (LittleFS.begin())
        printf("LittleFS mounted\n");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" connected to " WIFI_SSID);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    server.on("/", [](){
        server.send(200, "text/plain", "howdy monty");
    });
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started");

    const char* fname = "/demo.mpy";
    auto bcData = loadBytecode(fname);
    if (bcData == 0) {
        printf("can't open %s\n", fname);
        perror(fname);
        return;
    }

    constexpr auto N = 32*1024;
    auto myMem = malloc(N);
    Monty::setup(myMem, N);

    auto init = Monty::loader("__main__", bcData);
    if (init == nullptr) {
        printf("can't load module\n");
        return;
    }

    free((void*) bcData);

    {
        Monty::PyVM vm (*init);

        while (true || vm.isAlive()) {
            vm.scheduler();
            //archIdle();
            server.handleClient();
        }
    }

    printf("done\n");
}

void loop () {}
