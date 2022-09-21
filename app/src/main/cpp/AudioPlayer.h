//
// Created by vitor on 15/09/2022.
//

#ifndef TESTEOPENGL_AUDIOPLAYER_H
#define TESTEOPENGL_AUDIOPLAYER_H

#include <aaudio/AAudio.h>
#include <oboe/Oboe.h>
#include <memory>
#include <android/asset_manager.h>

#include "Player.h"

class AudioPlayer: public oboe::AudioStreamDataCallback {
public:
    AAudioStream* aaudio_stream{};
    int bufferSize = 0;
    AudioPlayer() = default;
    void Init(int id);
    void BeginPlay(void* bufferL, int maxSize, void* bufferR) const;
    int WriteAudio(int16_t* samples, int numFrames) const;
    static AudioPlayer* get();
    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) override;
    std::shared_ptr<oboe::AudioStream> m_audioStream;
    std::mutex mut;
    std::unique_ptr<Player> m_audioPlayer;
private:
    int m_channelCount = 2;
    static constexpr int m_sampleRate = 48000;
    // Wave params, these could be instance variables in order to modify at runtime
    static float constexpr kAmplitude = 0.5f;
    static float constexpr kFrequency = 440;
    static float constexpr kPI = M_PI;
    static float constexpr kTwoPi = kPI * 2;
    static double constexpr mPhaseIncrement = kFrequency * kTwoPi / (double) m_sampleRate;
    // Keeps track of where the wave is
    float mPhase = 0.0;
};

#endif //TESTEOPENGL_AUDIOPLAYER_H
