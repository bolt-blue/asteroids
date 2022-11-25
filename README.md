# (eventually) Asteroids

A plain old Asteroids clone. Well, it _will_ be. Based on the original 1979
arcade cabinet version.

The game is a means by which to explore relatively low level, cross platform
software architecture.

We use as few libraries as possible. Ideally nothing used will require a user
to install anything extra onto their base system.

While the game itself should provide some nice challenges, it's a good
opportunity to explore what's really necessary to get an image drawn on the
screen (well, in a window on the screen), on various platforms.

### Controls

_Not yet fully implemented_

- `w`:      Thrust
- `ad`:     Rotate left/ right
- `Space`:  Fire (shoot)
- `h`:      Hyperspace
- `Esc`:    Quit


## Dependencies

### Linux

- [xcb](https://xcb.freedesktop.org/)


## Build

Currently we're only testing and functioning (within reason) on Linux.

```sh
$ make
$ ./build/asteroids
```


## Cross-Platform Window

We aim to as simply as possible, get a window handle regardless of what
platform we're compiling on.

More than just a handle though, **the aim is to provide a surface to draw on**.

We ultimately want to be able to manipulate a buffer under our control, and
simply blit straight to a surface for rendering.


## Software rendering

At least in the beginning, we shall handle all rendering on the CPU.


## TODO

Current priorities in some sort of priority order (from top down):

### Platform

- [x] Get basic X11 support up and running
  - [x] Get colour pixels on the screen (but Asteroids is black and white, you say)
  - [x] "Proper" event handling
- [ ] Memory management
  - [x] Basic
  - [ ] Satisfactory
  - [ ] Confirm we're actually doing this properly
- [x] Hot-loading for live editing
- [ ] Window resizing and fullscreen support

### Gameplay

- [ ] Collision detection
- [ ] Shooting
- [ ] Text support
- [ ] Modes - attract, ready-to-play, play, high score

### Graphics and Sound

- [x] Fixed frame rate
  - [ ] Handle this properly internally
- [x] Line drawing
- [ ] Sub-pixel rendering
- [ ] Antialiasing
- [ ] Sound
- [ ] Handle colour more correctly
  - [ ] Linear colour space
  - [ ] HSV and Lch support
- [ ] Rendering on GPU


## Platform Support

We are not targetting every platform in existence, but at least the following:

- [ ] Desktop
    - [ ] Unix
        - [x] X11
        - [ ] Wayland
    - [ ] OSX
    - [ ] Windows
- [ ] Mobile
    - [ ] Android
    - [ ] iOS


