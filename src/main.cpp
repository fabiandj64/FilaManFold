#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_PN532.h>

#include "assign.h"
#include "weigh.h"

// WLAN-Daten
const char *ssid = ""; //change this
const char *password = ""; //change this

// URLs
const char *printerUrl = ""; // change this
const char *spoolmanUrl = ""; // change this

// I2C Pins PN532
#define PN532_IRQ 32
#define PN532_RST 33

Adafruit_PN532 nfc(PN532_IRQ, PN532_RST);

String oldSpoolId = "";
String spoolId = "";

// Modus-Schalter
const int modeSwitchPin = 4;

// NTAG215 Pages starten bei 4, NDEF beginnt dort üblicherweise
#define NTAG215_START_PAGE 4

// Liest 4 Bytes (1 Page) vom Tag, gibt true bei Erfolg
bool readPage(uint8_t page, uint8_t *buffer)
{
  return nfc.ntag2xx_ReadPage(page, buffer);
}

// Versucht, NDEF-Text aus dem Tag zu lesen (sehr einfache Version)
String readNdefText()
{
  uint8_t pageBuffer[4];
  String fullPayload = "";

  // Lesen der ersten 5 Pages (20 Bytes), das sollte für kleine NDEF-Daten reichen
  for (uint8_t page = NTAG215_START_PAGE; page < NTAG215_START_PAGE + 5; page++)
  {
    if (!readPage(page, pageBuffer))
    {
      Serial.printf("Fehler beim Lesen von Seite %d\n", page);
      break;
    }
    for (int i = 0; i < 4; i++)
    {
      fullPayload += (char)pageBuffer[i];
    }
  }

  // Suche im Payload nach "SPOOL:" als einfacher Filter
  int idx = fullPayload.indexOf("SPOOL:");
  if (idx == -1)
    return "";

  // Extrahiere ab "SPOOL:" bis Zeilenende oder max 10 Zeichen
  int end = fullPayload.indexOf('\n', idx);
  if (end == -1)
    end = idx + 10; // max Länge
  String spoolLine = fullPayload.substring(idx, end);
  spoolLine.trim();

  return spoolLine;
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  pinMode(modeSwitchPin, INPUT_PULLUP);

  WiFi.begin(ssid, password);
  Serial.print("Verbinde mit WLAN");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nVerbunden mit IP: " + WiFi.localIP().toString());

  nfc.begin();
  if (!nfc.getFirmwareVersion())
  {
    Serial.println("PN532 nicht gefunden!");
    delay(1000);
  }
  nfc.SAMConfig();
  Serial.println("NFC bereit.");

  setupScale();
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WLAN verloren. Verbinde neu...");
    WiFi.reconnect();
    delay(1000);
    return;
  }

  checkTaraButton();

  uint8_t uid[7];
  uint8_t uidLength;

  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 1000))
  {
    Serial.print("Tag UID: ");
    for (uint8_t i = 0; i < uidLength; i++)
    {
      Serial.printf("%02X ", uid[i]);
    }
    Serial.println();

    String ndefText = readNdefText();
    if (ndefText.length() > 0)
    {
      Serial.println("Gefundene NDEF Daten: " + ndefText);

      // Spool-ID aus "SPOOL:" extrahieren
      int idx = ndefText.indexOf("SPOOL:");
      if (idx >= 0)
      {
        oldSpoolId = ndefText.substring(idx + 6);
        oldSpoolId.trim();
        Serial.println("Spool-ID erkannt: " + oldSpoolId);

        String newNdefText = readNdefText();

        if (newNdefText.length() > 0)
        {
          Serial.println("Gefundene NDEF Daten: " + newNdefText);

          // Spool-ID aus "SPOOL:" extrahieren
          int idx = newNdefText.indexOf("SPOOL:");
          if (idx >= 0)
          {
            spoolId = newNdefText.substring(idx + 6);
            spoolId.trim();
            Serial.println("Spool-ID erkannt: " + spoolId);
          }
        }
        else
        {
          Serial.println("Keine NDEF-Daten gefunden.");
        }

        if (spoolId == oldSpoolId)
        {

          bool modeWeighing = digitalRead(modeSwitchPin) == HIGH;

          if (true) // TODO: Anpassen im finalen Code
          {
            weighSpool(spoolId, spoolmanUrl);
            assignSpool(spoolId, printerUrl);
          }
          else
          {
            assignSpool(spoolId, printerUrl);
          }

          spoolId, oldSpoolId = "";
        }
      }
    }
    else
    {
      Serial.println("Keine NDEF-Daten gefunden.");
    }
    delay(1000); // Debounce Zeit
  }
}
