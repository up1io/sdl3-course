#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

SDL_AppResult
SDL_AppInit(void **appstate, int argc, char **argv)
{
    if (!SDL_Init(SDL_INIT_AUDIO)) 
    {
        SDL_Log("ERROR: Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    int i, num_devices;
    SDL_AudioDeviceID *devices = SDL_GetAudioPlaybackDevices(&num_devices);
    if (!devices) 
    {
        SDL_Log("ERROR: No audio devices found.");
        return SDL_APP_FAILURE;
    }

    for (i = 0; i < num_devices; ++i) 
    {
        SDL_AudioDeviceID instance_id = devices[i];
        SDL_Log("INFO: AudioDevice %s", SDL_GetAudioDeviceName(instance_id));
    }

    SDL_AudioSpec desired = { 0 };
    desired.freq = 44100;
    desired.format = SDL_AUDIO_S16;
    desired.channels = 1;

    SDL_AudioStream *stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &desired, NULL, NULL);
    if (!stream) 
    {
        SDL_Log("ERROR: Failed to open audio stream. %s", SDL_GetError());
        SDL_free(devices);
        return SDL_APP_FAILURE;
    }

    SDL_AudioDeviceID device = SDL_GetAudioStreamDevice(stream);
    if (!SDL_ResumeAudioDevice(device)) 
    {
        SDL_Log("ERROR: Failed resume audio device. %s", SDL_GetError());
        SDL_free(devices);
        return SDL_APP_FAILURE;
    }

    SDL_AudioSpec wav_spec;
    Uint8 *wav_buffer;
    Uint32 wav_length;
    if (!SDL_LoadWAV("audio/default.wav", &wav_spec, &wav_buffer, &wav_length)) {
        SDL_Log("ERROR: Could not load WAV file: %s\n", SDL_GetError());
        SDL_free(wav_buffer);
        SDL_free(devices);
        return SDL_APP_FAILURE;
    }

    if (!SDL_PutAudioStreamData(stream, wav_buffer, wav_length)) 
    {
        SDL_Log("ERROR: Failed to put audio stream data. %s", SDL_GetError());
        SDL_free(wav_buffer);
        SDL_free(devices);
        return SDL_APP_FAILURE;
    } 

    SDL_free(wav_buffer);
    SDL_free(devices);
    return SDL_APP_CONTINUE;
}

SDL_AppResult
SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if(event->type == SDL_EVENT_QUIT)
    {
        return SDL_APP_SUCCESS;
    }

    if(event->type == SDL_EVENT_KEY_DOWN)
    {
        return SDL_APP_SUCCESS;
    }

    return SDL_APP_CONTINUE;
}
SDL_AppResult
SDL_AppIterate(void *appstate)
{
    return SDL_APP_CONTINUE;
}

void
SDL_AppQuit(void *appstate, SDL_AppResult result)
{
}


