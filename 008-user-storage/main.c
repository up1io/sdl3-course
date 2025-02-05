#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <string.h>

#define ORG "com.example"
#define APP_NAME "my-game"

void
APP_WriteUserData(const char *filename, void *data, Uint64 data_len)
{
    SDL_Storage *user = SDL_OpenUserStorage(ORG, APP_NAME, 0);
    if (user == NULL)
    {
        SDL_Log("ERROR: Failed to open user storage. %s", SDL_GetError());
        return;
    }

    while(!SDL_StorageReady(user))
    {
        SDL_Delay(1);
    }

    if (!SDL_WriteStorageFile(user, filename, data, data_len))
    {
        SDL_Log("ERROR: Failed to write storage file. %s", SDL_GetError());
        SDL_CloseStorage(user);
        return;
    }

    SDL_CloseStorage(user);
}

void
APP_ReadUserData(const char *filename)
{
    SDL_Storage *user = SDL_OpenUserStorage(ORG, APP_NAME, 0);
    if (user == NULL)
    {
        SDL_Log("ERROR: Failed to open user storage. %s", SDL_GetError());
        return;
    }

    while (!SDL_StorageReady(user))
    {
        SDL_Delay(1);
    }

    Uint64 data_len = 0;
    if (!SDL_GetStorageFileSize(user, filename, &data_len))
    {
        SDL_Log("ERROR: Failed to get file size. %s", SDL_GetError());
        SDL_CloseStorage(user);
        return;
    }

    if (data_len > 0)
    {
        void *data = SDL_malloc(data_len);
        if (!SDL_ReadStorageFile(user, "save0.sav", data, data_len))
        {
            SDL_Log("ERROR: Failed to read storage file. %s", SDL_GetError());
            SDL_CloseStorage(user);
            return;
        }

        SDL_Log("INFO: Save Data");
        SDL_Log("%s", (char *)data);

        SDL_free(data);
    }
    else
    {
        SDL_Log("INFO: No file content");
    }

    SDL_CloseStorage(user);
}

SDL_AppResult
SDL_AppInit(void **appstate, int argc, char **argv)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("ERROR: Failed to init sdl. %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }


    char *data = "<your game data from the user>";
    APP_WriteUserData("save0.sav", data, strlen(data));
    SDL_Log("INFO: Save data are written");

    APP_ReadUserData("save0.sav");
    

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
