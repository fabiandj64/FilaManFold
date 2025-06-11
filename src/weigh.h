#ifndef WEIGH_H
#define WEIGH_H

#include <Arduino.h>

void weighSpool(const String &spoolId, const String &baseUrl);
String scaleMeasure();
void setupScale();
void checkTaraButton();

#endif