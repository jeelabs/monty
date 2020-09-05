#include "monty.h"
#include "arch.h"

#include <cassert>
#include "pyvm.h"

#include "../../ssidpass.h"

#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

AsyncWebServer server(80);
AsyncWebSocket ws ("/ws");
AsyncWebSocket ws2 ("/live");

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
    char const* t = "type?";
    switch (type) {
        case WS_EVT_CONNECT:    t = "connect"; break;
        case WS_EVT_DISCONNECT: t = "disconnect"; break;
        case WS_EVT_ERROR:      t = "error"; break;
        case WS_EVT_PONG:       t = "pong"; break;
        case WS_EVT_DATA:       t = "data"; break;
    }
    auto url = server->url();
    auto cid = client->id();
    char hdr [30];
    sprintf(hdr, "ws:%.10s@%u", url, cid);
    Serial.printf("%s %s\n", hdr, t);

    if (type == WS_EVT_CONNECT)
        client->ping();
    else if (type == WS_EVT_DATA) {
        auto info = (AwsFrameInfo*) arg;
        bool isText = info->opcode == WS_TEXT;
        auto msgType = isText ? "text" : "binary";

        String msg = "";
        if (info->final && info->index == 0 && info->len == len) {
            //the whole message is in a single frame and we got all of it's data
            Serial.printf("%s %s[%llu]: ", hdr, msgType, info->len);

            if (isText)
                for(size_t i = 0; i < info->len; ++i)
                    msg += (char) data[i];
            else {
                char buff [4];
                for(size_t i = 0; i < info->len; ++i) {
                    sprintf(buff, "%02x ", data[i]);
                    msg += buff ;
                }
            }
            Serial.printf("%s\n", msg.c_str());
        } else {
            //message is comprised of multiple frames or the frame is split into multiple packets
            if (info->index == 0) {
                if (info->num == 0)
                    Serial.printf("%s %s start\n", hdr, msgType);
                Serial.printf("%s frame[%u] start[%llu]\n", hdr, info->num, info->len);
            }

            Serial.printf("%s frame[%u] %s[%llu - %llu]: ", hdr, info->num, msgType, info->index, info->index + len);

            if (isText)
                for(size_t i = 0; i < len; ++i)
                    msg += (char) data[i];
            else {
                char buff [4];
                for(size_t i = 0; i < len; ++i) {
                    sprintf(buff, "%02x ", data[i]);
                    msg += buff ;
                }
            }
            Serial.printf("%s\n",msg.c_str());

            if (info->index + len != info->len)
                return;
            Serial.printf("%s frame[%u] end[%llu]\n", hdr, info->num, info->len);
            if (info->final)
                Serial.printf("%s %s end\n", hdr, msgType);
        }
        if (isText)
            client->text("[123,345,567]\n");
        else
            client->binary("I got your binary message");
    }
}

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

    server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "howdy monty");
    });

    server.serveStatic("/", LittleFS, "www/").setDefaultFile("index.html");

    ws.onEvent(onWsEvent);
    server.addHandler(&ws);

    ws2.onEvent(onWsEvent);
    server.addHandler(&ws2);

    server.onNotFound([](AsyncWebServerRequest *request) {
        Serial.printf("not found: http://%s%s\n", request->host().c_str(), request->url().c_str());

        if (request->contentLength()) {
            Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
            Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
        }

        int headers = request->headers();
        for (int i = 0;i<headers;++i) {
            AsyncWebHeader* h = request->getHeader(i);
            Serial.printf("HEADER %s: %s\n", h->name().c_str(), h->value().c_str());
        }

        int params = request->params();
        for (int i = 0;i<params;++i) {
            AsyncWebParameter* p = request->getParam(i);
            if (p->isFile()) {
                Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
            } else if (p->isPost()) {
                Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
            } else {
                Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
            }
        }

        request->send(404);
    });

    server.begin();
    Serial.println("HTTP server started");

    const char* fname = "/demo.mpy";
    auto bcData = loadBytecode(fname);
    if (bcData == 0) {
        printf("can't open %s\n", fname);
        perror(fname);
        return;
    }

    constexpr auto N = 8*1024;
    auto myMem = malloc(N);
    if (myMem == nullptr) {
        printf("can't alloc %db memory pool\n", N);
        return;
    }
    Monty::setup(myMem, N);

    auto init = Monty::loader("__main__", bcData);
    if (init == nullptr) {
        printf("can't load module\n");
        return;
    }

    free((void*) bcData);

    if (false) {
        Monty::PyVM vm (*init);

        while (true || vm.isAlive()) {
            vm.scheduler();
            //archIdle();
            ws.cleanupClients();
        }
    }

    printf("done\n");
}

void loop () {
    ws.cleanupClients();
}
