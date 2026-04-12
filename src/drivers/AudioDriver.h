#pragma once

#include <Audio.h>
#include <AudioBoard.h>
#include <DriverPins.h>

class SelfAudioDriver {
public:
    SelfAudioDriver();
    void begin();
    void play(const char* file);
    void loop();
    void setVolume(int volume);
    bool isPlaying();

private:
    Audio audio;
    DriverPins pins;
    AudioBoard* board;
};