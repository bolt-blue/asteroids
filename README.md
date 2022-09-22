# (eventually) Asteroids

A plain old Asteroids clone. Well, it _will_ be.

The game is a means by which to explore relatively low level, cross platform
software architecture.

We use as few libraries as possible. Ideally nothing used will require a user
to install anything extra onto their base system.

While the game itself should provide some nice challenges, it's a good
opportunity to explore what's really necessary to get an image drawn on the
screen (well, in a window on the screen), on various platforms.


## Dependencies

### Linux

- [xcb](https://xcb.freedesktop.org/)


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

- [ ] Get X11 support up and running
  - [x] Get colour pixels on the screen (but Asteroids is black and white, you say)
  - [ ] Proper event handling
- [x] Line drawing
- [ ] Memory management
  - [x] Basic
- [x] Fixed frame rate
  - [ ] Confirm we're actually doing this properly
- [ ] Hot-loading for live editing
- [ ] Collision detection
- [ ] Text support
- [ ] Sub-pixel rendering
- [ ] Antialiasing
- [ ] Sound
- [ ] Window resizing and fullscreen support
- [ ] Handle colour more properly
  - [ ] Linear colour space
  - [ ] HSV and Lch support


## Platform Support

We are not targetting every platform in existence, but at least the following:

- [ ] Desktop
    - [ ] Linux
        - [ ] X11
        - [ ] Wayland
    - [ ] OSX
    - [ ] Windows
- [ ] Mobile
    - [ ] Android
    - [ ] iOS


