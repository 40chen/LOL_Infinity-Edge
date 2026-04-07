#pragma once

#include <Audio.h>
#include <AudioBoard.h>
#include <DriverPins.h>

class SaberAudioDriver {
public:
    SaberAudioDriver();
    void begin();
    void play(const char* file);
    void loop();
    void setVolume(int volume);

private:
    Audio audio;
    DriverPins pins;
    AudioBoard* board;
};