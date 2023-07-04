#include <HTTPClient.h>
#include <api.h>

Api::Api(const String& ipAddress, HTTPClient& httpClient)
    : ip(ipAddress), http(httpClient) {}

bool Api::send(const String& endpoint, const String& arguments) {
    String url = ip + endpoint + arguments;
    http.begin(url);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
        http.end();
        return true;
    } else {
        Serial.print("Error on HTTP request: ");
        Serial.println(httpResponseCode);
        http.end();
        return false;
    }
}

String Api::get(const String& endpoint, const String& arguments) {
    String url = ip + endpoint + arguments;
    http.begin(url);
    int httpResponseCode = http.GET();
    String payload;
    if (httpResponseCode > 0) {
        payload = http.getString();
    } else {
        Serial.print("Error on HTTP request: ");
        Serial.println(httpResponseCode);
    }
    http.end();
    return payload;
}