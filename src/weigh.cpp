#include "weigh.h"
#include <HTTPClient.h>
#include <HX711.h>

#define DOUT 18
#define SCK 19
#define TARE_BUTTON_PIN 5  // Pin für Tara-Taster

HX711 scale;

void weighSpool(const String& spoolId, const String& spoolmanUrl) {

    const String measured_weight = scaleMeasureFixed();
    HTTPClient http;
    String url = spoolmanUrl + spoolId;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    String body = "{\"remaining_weight\": " + measured_weight + "}";
    Serial.println("Sende (Wiegen): " + body);

    int httpCode = http.PATCH(body);
    if (httpCode > 0) {
        Serial.printf("Antwortcode: %d\n", httpCode);
        Serial.println(http.getString());
    } else {
        Serial.printf("Fehler: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
}

String scaleMeasureFixed() {

    int weight = 200;
    return (String)weight;
}

void setupScale() {
    
    pinMode(TARE_BUTTON_PIN, INPUT_PULLUP);

    scale.begin(DOUT, SCK);

    if (!scale.is_ready()) {
        Serial.println("HX711 nicht bereit.");
        while (1);
    }

    scale.set_scale(4.242);  // <-- Kalibrierfaktor hier anpassen!
    scale.tare();
    Serial.println("Waage initialisiert und nullgestellt.");
}

void checkTaraButton() {
    static bool lastButtonState = HIGH;
    bool buttonState = digitalRead(TARE_BUTTON_PIN);

    if (lastButtonState == HIGH && buttonState == LOW) { // gedrückt
        Serial.println("Tara-Taster gedrückt. Waage wird genullt...");
        scale.tare();
        delay(1000);  // Entprellen
    }

    lastButtonState = buttonState;
}

String scaleMeasure() {

    long gewicht = scale.get_units(5);  // 5-fach gemittelt
    Serial.printf("Gemessenes Gewicht: %ld g\n", gewicht);
    return String(gewicht);
}

