//
// Created by vitor on 15/09/2022.
//

#include "AudioPlayer.h"
#include "LogUtils.h"
#include <jni.h>
#include <memory>

static AudioPlayer *player;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_testeopengl_NativeRenderer_CreateAudioWithDeviceId(JNIEnv *env, jclass clazz,
                                                                    jint deviceId) {
    delete player;
    player = new AudioPlayer;
    player->Init(deviceId);
}

float *p_audioBufferL = nullptr;
float *p_audioBufferR = nullptr;
int64_t p_audioPosition = 0;
int p_maxSize = 0;

void AudioPlayer::BeginPlay(void *bufferL, int maxSize, void *bufferR) const {
    p_maxSize = maxSize;
    p_audioBufferL = (float *) bufferL;
    p_audioBufferR = (float *) bufferR;
//    auto result = AAudioStream_requestStart(stream);
//    if(result != AAUDIO_OK){
//        LOG("Result is %d", result);
//    }
    m_audioStream->requestStart();
}

void renderSilence(float *start, int32_t numSamples) {
    for (int i = 0; i < numSamples; ++i) {
        start[i] = 0;
    }
}

oboe::DataCallbackResult
AudioPlayer::onAudioReady(oboe::AudioStream *stream, void *audioData, int32_t numFrames) {

//    float *floatData = (float *) audioData;
//    for (int i = 0; i < numFrames; ++i) {
//        float sampleValue = kAmplitude * sinf(mPhase);
//        for (int j = 0; j < m_channelCount; j++) {
//            floatData[i * m_channelCount + j] = sampleValue;
//        }
//        mPhase += mPhaseIncrement;
//        if (mPhase >= kTwoPi) mPhase -= kTwoPi;
//    }
    auto* audioOut = (float*)audioData;

    for(int i = 0; i < numFrames; i++){
        float sample = 0;
        audioOut[i * 2] = sample;
        audioOut[i * 2 + 1] = sample;
    }

    m_audioPlayer->renderAudio(audioOut, numFrames);
    return oboe::DataCallbackResult::Continue;
}

void AudioPlayer::Init(int audioId) {
    oboe::AudioStreamBuilder builder;
    auto result = builder.setSharingMode(oboe::SharingMode::Shared)
            ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
            ->setChannelCount(oboe::ChannelCount::Stereo)
            ->setSampleRate(m_sampleRate)
            ->setFormatConversionAllowed(true)
            ->setFormat(oboe::AudioFormat::Float)
            ->setSampleRateConversionQuality(oboe::SampleRateConversionQuality::Medium)
            ->setUsage(oboe::Usage::Media)
            ->setDirection(oboe::Direction::Output)
            ->setDataCallback(this)
            ->openStream(m_audioStream);

    auto sampleRate = m_audioStream->getSampleRate();

    if (result != oboe::Result::OK) {
        LOG("Result is not ok: %d", result);
    }

//    m_audioStream->setBufferSizeInFrames(m_audioStream->getFramesPerBurst() * kBufferSizeInBursts);
//
//    auto bsize = m_audioStream->getBufferSizeInFrames();
//    auto *buff = (float *) calloc(bsize * 2, sizeof(uint8_t));
//    m_audioStream->write(buff, bsize, 0);
//    free(buff);
//    m_audioStream->requestFlush();
}

int AudioPlayer::WriteAudio(int16_t *samples, int numFrames) const {
    return AAudioStream_write(aaudio_stream, samples, numFrames, 1600000);
}

AudioPlayer *AudioPlayer::get() {
    return player;
}
