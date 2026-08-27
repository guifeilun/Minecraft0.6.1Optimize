#include "SoundSystemAL.h"
#include "../../util/Mth.h"
#include "../../world/phys/Vec3.h"
#include "../../client/sound/Sound.h"
#include "../log.h"
#include <math.h>

SoundSystemAL::SoundSystemAL()
    : _dsound(NULL), _playedCnt(0), _rotation(-9999.9f), _available(false)
{
    _available = initDirectSound();
}

SoundSystemAL::~SoundSystemAL()
{
    for (int i = 0; i < MAX_PLAYED; i++) {
        if (_buffers[i].buffer) {
            _buffers[i].buffer->Stop();
            _buffers[i].buffer->Release();
            _buffers[i].buffer = NULL;
        }
        _buffers[i].inUse = false;
    }

    if (_dsound) {
        _dsound->Release();
        _dsound = NULL;
    }
}

bool SoundSystemAL::initDirectSound()
{
    HRESULT hr = DirectSoundCreate8(NULL, &_dsound, NULL);
    if (FAILED(hr)) {
        LOGE("DirectSoundCreate8 failed: 0x%08X\n", hr);
        return false;
    }

    HWND hwnd = GetActiveWindow();
    if (!hwnd) {
        LOGE("Cannot get window handle for DirectSound\n");
        _dsound->Release();
        _dsound = NULL;
        return false;
    }

    hr = _dsound->SetCooperativeLevel(hwnd, DSSCL_NORMAL);
    if (FAILED(hr)) {
        LOGE("SetCooperativeLevel failed: 0x%08X\n", hr);
        _dsound->Release();
        _dsound = NULL;
        return false;
    }

    LOGI("DirectSound initialized successfully\n");
    return true;
}

void SoundSystemAL::enable(bool status)
{
    _available = status && (_dsound != NULL);
    LOGI("DirectSound enable: %d\n", _available);
}

void SoundSystemAL::setListenerPos(float x, float y, float z) {}
void SoundSystemAL::setListenerAngle(float deg) { _rotation = deg; }

void SoundSystemAL::removeStoppedSounds()
{
    _playedCnt = 0;
    for (int i = 0; i < MAX_PLAYED; i++) {
        if (!_buffers[i].buffer) continue;
        
        DWORD status;
        HRESULT hr = _buffers[i].buffer->GetStatus(&status);
        if (FAILED(hr)) {
            _buffers[i].buffer->Release();
            _buffers[i].buffer = NULL;
            _buffers[i].inUse = false;
            continue;
        }
        
        if (status & DSBSTATUS_PLAYING) {
            _playedCnt++;
        } else {
            _buffers[i].buffer->Release();
            _buffers[i].buffer = NULL;
            _buffers[i].inUse = false;
        }
    }
}

int SoundSystemAL::getFreeSourceIndex()
{
    removeStoppedSounds();
    
    for (int i = 0; i < MAX_PLAYED; i++) {
        if (!_buffers[i].inUse || !_buffers[i].buffer) {
            _buffers[i].inUse = true;
            return i;
        }
    }
    return -1;
}

bool SoundSystemAL::createBuffer(const SoundDesc& sound, LPDIRECTSOUNDBUFFER* outBuffer)
{
    if (!sound.isValid()) {
        LOGE("Invalid sound data\n");
        return false;
    }

    WAVEFORMATEX wf;
    ZeroMemory(&wf, sizeof(wf));
    wf.wFormatTag = WAVE_FORMAT_PCM;
    wf.nSamplesPerSec = sound.frameRate;
    wf.wBitsPerSample = sound.byteWidth * 8;
    wf.nChannels = sound.channels;
    wf.nBlockAlign = sound.channels * sound.byteWidth;
    wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
    wf.cbSize = 0;

    DSBUFFERDESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2;
    desc.dwBufferBytes = sound.size;
    desc.lpwfxFormat = &wf;

    LPDIRECTSOUNDBUFFER tmpBuffer = NULL;
    HRESULT hr = _dsound->CreateSoundBuffer(&desc, &tmpBuffer, NULL);
    if (FAILED(hr)) {
        LOGE("CreateSoundBuffer failed: 0x%08X (size=%d)\n", hr, sound.size);
        return false;
    }

    *outBuffer = tmpBuffer;

    void* writePtr = NULL;
    DWORD writeSize = 0;
    hr = (*outBuffer)->Lock(0, sound.size, &writePtr, &writeSize, NULL, 0, 0);
    if (FAILED(hr)) {
        LOGE("Lock failed: 0x%08X\n", hr);
        (*outBuffer)->Release();
        *outBuffer = NULL;
        return false;
    }

    memcpy(writePtr, sound.frames, sound.size);
    (*outBuffer)->Unlock(writePtr, writeSize, NULL, 0);

    return true;
}

void SoundSystemAL::playAt(const SoundDesc& sound, float x, float y, float z, float volume, float pitch)
{
    if (!_available || !_dsound) return;
    if (!sound.isValid()) {
        LOGE("Invalid sound: %s\n", sound.name.c_str());
        return;
    }

    int index = getFreeSourceIndex();
    if (index < 0) {
        return;
    }

    LPDIRECTSOUNDBUFFER buffer = NULL;
    if (!createBuffer(sound, &buffer)) {
        _buffers[index].inUse = false;
        return;
    }

    _buffers[index].buffer = buffer;

    LONG dsVolume = DSBVOLUME_MIN;
    if (volume > 0.0f) {
        float vol = (volume > 1.0f) ? 1.0f : volume;
        if (vol > 0.0f) {
            dsVolume = (LONG)(2000.0f * log10f(vol));
            if (dsVolume < DSBVOLUME_MIN) dsVolume = DSBVOLUME_MIN;
            if (dsVolume > DSBVOLUME_MAX) dsVolume = DSBVOLUME_MAX;
        }
    }
    buffer->SetVolume(dsVolume);

    HRESULT hr = buffer->Play(0, 0, 0);
    if (FAILED(hr)) {
        LOGE("Play failed: 0x%08X\n", hr);
        buffer->Release();
        _buffers[index].buffer = NULL;
        _buffers[index].inUse = false;
    }
}
