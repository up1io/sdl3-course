#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

SDL_AppResult
SDL_AppInit(void **appstate, int argc, char **argv)
{
    SDL_SetAppMetadata("Window", "1.0", "com.up.window");

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("ERROR: Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Window", 640, 800, 0, &window, &renderer))
    {
        SDL_Log("ERROR: Couldn't create winodw/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

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
    SDL_SetRenderDrawColor(renderer, 255, 50, 50, SDL_ALPHA_OPAQUE_FLOAT);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
 
    float x = 0; 
    float y = 0;

    SDL_GetMouseState(&x, &y);

    SDL_Log("INFO: Mouse Position x: %f y: %f", x, y);

    SDL_Delay(1000);
  
    return SDL_APP_CONTINUE;
}

void
SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    SDL_DestroyWindow(window);
}
