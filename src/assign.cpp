#include "assign.h"
#include <HTTPClient.h>

void assignSpool(const String &spoolId, const String &baseUrl)
{

    HTTPClient http;
    String url = baseUrl + "/printer/gcode/script";

    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    String body = "{\"script\": \"MMU_GATE_MAP NEXT_SPOOL_ID=" + spoolId + "\"}";

    Serial.println("Sende (Zuweisen): " + body);

    int httpCode = http.POST(body);
    if (httpCode > 0)
    {
        Serial.printf("Antwortcode: %d\n", httpCode);
        Serial.println(http.getString());
    }
    else
    {
        Serial.printf("Fehler: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
}
