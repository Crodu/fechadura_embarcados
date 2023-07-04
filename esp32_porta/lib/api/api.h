//create class api:

#ifndef ESP32_API_H
#define ESP32_API_H

#include <HTTPClient.h>

class Api {
public:
    Api(const String& ipAddress, HTTPClient& httpClient);

    bool send(const String& endpoint, const String& arguments);
    String get(const String& endpoint, const String& arguments);

private:
    String ip;
    HTTPClient& http;
};

#endif // ESP32_API_H