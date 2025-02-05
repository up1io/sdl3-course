# SDL Mouse Example

Simple SDL example that prints the current mouse position within the SDL window every second.

## Quick Reference

```c
// Return whether a mouse is currently connected.
bool SDL_HasMouse(void);

// Get a list of currently connected mice.
SDL_MouseID * SDL_GetMice(int *count);

// Get the name of a mouse.
const char * SDL_GetMouseNameForID(SDL_MouseID instance_id);                                              

// Get the window which currently has mouse focus.
SDL_Window * SDL_GetMouseFocus(void);

// Query SDL's cache for the synchronous mouse button state and the window-relative SDL-cursor position.
SDL_MouseButtonFlags SDL_GetMouseState(float *x, float *y);

// Query the platform for the asynchronous mouse button state and the desktop-relative platform-cursor position.
SDL_MouseButtonFlags SDL_GetGlobalMouseState(float *x, float *y);

// Query SDL's cache for the synchronous mouse button state and accumulated mouse delta since last call.
SDL_MouseButtonFlags SDL_GetRelativeMouseState(float *x, float *y);

// Move the mouse cursor to the given position within the window.
void SDL_WarpMouseInWindow(SDL_Window *window, float x, float y);

// Move the mouse to the given position in global screen space.
bool SDL_WarpMouseGlobal(float x, float y);

// Set relative mouse mode for a window.
bool SDL_SetWindowRelativeMouseMode(SDL_Window *window, bool enabled);

// Query whether relative mouse mode is enabled for a window.
bool SDL_GetWindowRelativeMouseMode(SDL_Window *window);

// Capture the mouse and to track input outside an SDL window.
bool SDL_CaptureMouse(bool enabled);

// Create a cursor using the specified bitmap data and mask (in MSB format).
SDL_Cursor * SDL_CreateCursor(const Uint8 *data, const Uint8 *mask, int w, int h, int hot_x, int hot_y);

// Create a color cursor.
SDL_Cursor * SDL_CreateColorCursor(SDL_Surface *surface, int hot_x, int hot_y);

// Create a system cursor.
SDL_Cursor * SDL_CreateSystemCursor(SDL_SystemCursor id);

// Set the active cursor.
bool SDL_SetCursor(SDL_Cursor *cursor);

// Get the active cursor.
SDL_Cursor * SDL_GetCursor(void);

// Get the default cursor.
SDL_Cursor * SDL_GetDefaultCursor(void);

// Free a previously-created cursor.
void SDL_DestroyCursor(SDL_Cursor *cursor);

// Show the cursor.
bool SDL_ShowCursor(void);

// Hide the cursor.
bool SDL_HideCursor(void);

// Return whether the cursor is currently being shown.
bool SDL_CursorVisible(void);

```

For more info head over to [SDL Quick Reference](https://wiki.libsdl.org/SDL3/QuickReference).

