#include <Arduino.h>
#include "./app/SaberController.h"

SaberController saber;

void setup() {
  saber.begin();
}

void loop() {
  saber.update();
}


