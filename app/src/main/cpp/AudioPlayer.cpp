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

AudioPlayer::AudioPlayer() {}

float* p_audioBufferL = nullptr;
float* p_audioBufferR = nullptr;
int64_t p_audioPosition = 0;
int p_maxSize = 0;
void AudioPlayer::BeginPlay(void* bufferL, int maxSize, void* bufferR) const {
    p_maxSize = maxSize;
    p_audioBufferL = (float*)bufferL;
    p_audioBufferR = (float*)bufferR;
//    auto result = AAudioStream_requestStart(stream);
//    if(result != AAUDIO_OK){
//        LOG("Result is %d", result);
//    }
    m_audioStream->requestStart();
}
aaudio_data_callback_result_t audio_callback(AAudioStream* stream, void* userData, void* audioData, int32_t numFrames){
//    auto movePosition = numFrames * 2;
//    if(p_audioPosition + movePosition < p_maxSize){
//        std::memcpy(audioData, p_audioBuffer + p_audioPosition, movePosition);
//        //p_audioPosition += movePosition;
//        p_audioPosition += movePosition;
//    }else{
//        p_audioPosition = 0;
//    }
    return AAUDIO_CALLBACK_RESULT_CONTINUE;
}
void renderSilence(float *start, int32_t numSamples){
    for (int i = 0; i < numSamples; ++i) {
        start[i] = 0;
    }
}
oboe::DataCallbackResult AudioPlayer::onAudioReady(oboe::AudioStream* stream, void* audioData, int32_t numFrames){
//    auto* outData = (float*)audioData;
//    if(p_audioPosition + numFrames < p_maxSize){
//        for(int i = 0; i < numFrames; ++i){
//            outData[i * m_channelCount] = p_audioBufferL[p_audioPosition];
//            //outData[i * m_channelCount + 1] = p_audioBufferR[p_audioPosition];
//            outData[i * m_channelCount + 1] = p_audioBufferR[p_audioPosition];
//        }
//        p_audioPosition += numFrames;
//    }else{
//        p_audioPosition = 0;
//    }
//    float *floatData = (float *) audioData;
//    for (int i = 0; i < numFrames; ++i) {
//        float sampleValue = kAmplitude * sinf(mPhase);
//        for (int j = 0; j < m_channelCount; j++) {
//            floatData[i * m_channelCount + j] = sampleValue;
//        }
//        mPhase += mPhaseIncrement;
//        if (mPhase >= kTwoPi) mPhase -= kTwoPi;
//    }

    m_audioPlayer->renderAudio((float*)audioData, numFrames);
    return oboe::DataCallbackResult::Continue;
}

void AudioPlayer::Init(int audioId) {
//    AAudioStreamBuilder *builder;
//    auto result = AAudio_createStreamBuilder(&builder);
//    AAudioStreamBuilder_setDeviceId(builder, audioId);
//    AAudioStreamBuilder_setDirection(builder, AAUDIO_DIRECTION_OUTPUT);
//    AAudioStreamBuilder_setSharingMode(builder, AAUDIO_SHARING_MODE_EXCLUSIVE);
//    AAudioStreamBuilder_setSampleRate(builder, 48000);
//    AAudioStreamBuilder_setFormat(builder, AAUDIO_FORMAT_PCM_I16);
//    //AAudioStreamBuilder_setChannelCount(builder, 2);
//    //AAudioStreamBuilder_setChannelMask(builder, AAUDIO_CHANNEL_STEREO);
//    //AAudioStreamBuilder_setBufferCapacityInFrames(builder, 10000);
//    AAudioStreamBuilder_setDataCallback(builder,
//                                        reinterpret_cast<AAudioStream_dataCallback>(audio_callback), nullptr);
//    result = AAudioStreamBuilder_openStream(builder, &stream);
//    int16_t* buffer = (int16_t*)malloc(100000);
//    std::memset(buffer, 0, sizeof(buffer));
//    AAudioStream_write(stream, buffer, 100000,100000);
//    free(buffer);
//    bufferSize = AAudioStream_getBufferSizeInFrames(stream);

    oboe::AudioStreamBuilder builder;
    auto result = builder.setSharingMode(oboe::SharingMode::Exclusive)
        ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
        ->setChannelCount(m_channelCount)
        ->setSampleRate(m_sampleRate)
        ->setFormat(oboe::AudioFormat::Float)
        ->setUsage(oboe::Usage::Media)
        ->setDirection(oboe::Direction::Output)
        ->setDataCallback(this)
        ->openStream(m_audioStream);

    if(result != oboe::Result::OK){
        LOG("Result is not ok: %d", result);
    }

    m_audioStream->setBufferSizeInFrames(m_audioStream->getFramesPerBurst() * 2);
    auto bsize = m_audioStream->getBufferSizeInFrames();
    float* buff = (float*)malloc(bsize * 2);
    m_audioStream->write(buff, bsize, 0);
    free (buff);
}

int AudioPlayer::WriteAudio(int16_t *samples, int numFrames) const {
    return AAudioStream_write(aaudio_stream, samples, numFrames, 1600000);
}

AudioPlayer *AudioPlayer::get() {
    return player;
}
