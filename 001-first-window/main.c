#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

SDL_AppResult
SDL_AppInit (void **appstate, int argc, char **argv)
{
  SDL_SetAppMetadata ("Window", "1.0", "com.up.window");

  if (!SDL_Init (SDL_INIT_VIDEO))
    {
      SDL_Log ("Couldn't initialize SDL: %s", SDL_GetError ());
      return SDL_APP_FAILURE;
    }

  if (!SDL_CreateWindowAndRenderer ("Window", 640, 800, 0, &window, &renderer))
    {
      SDL_Log ("Couldn't create winodw/renderer: %s", SDL_GetError ());
      return SDL_APP_FAILURE;
    }

  return SDL_APP_CONTINUE;
}

SDL_AppResult
SDL_AppEvent (void *appstate, SDL_Event *event)
{
  if (event->type == SDL_EVENT_QUIT)
    {
      return SDL_APP_SUCCESS;
    }

  return SDL_APP_CONTINUE;
}

SDL_AppResult
SDL_AppIterate (void *appstate)
{
  SDL_SetRenderDrawColor (renderer, 255, 50, 50, SDL_ALPHA_OPAQUE_FLOAT);
  SDL_RenderClear (renderer);
  SDL_RenderPresent (renderer);

  return SDL_APP_CONTINUE;
}

void
SDL_AppQuit (void *appstate, SDL_AppResult result)
{
}
