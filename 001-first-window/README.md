# Simple Window

In this code snippet I create a simple window with a red background color via sdl3. 
This use the new callbacks instead of setup all via the main entrypoint.

## Build

Make sure you already installed the sdl3 on your system.

> Todo(john): setup a seperat guide for that.

```
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/local/lib/pkgconfig
gcc main.c -o main $(pkg-config --cflags --libs sdl3)
```
