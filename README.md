# Fractal Finder

## About

Fractal Finder is a simple game about exploring deeper and deeper into various computer-generated
fractals and trying to identify patterns and points of interest displayed on the right of the
screen. The game has twelve levels in total, exploring six unique fractals that get progressively
more difficult as you progress.

Fractal Finder was made as part of the Ludum Dare 48

## Controls

* Hold the right mouse button and drag the mouse to move around the fractal.
* Use the scroll wheel to zoom in and out of the fractal.
* Press the middle mouse button to reset your view.
* When you think you've found an area that matches up with one of the previews on the right, use
the left mouse button to select it. The preview will fade out to show you've found it.
* If you're stumped on a point and just want to move on, you can use number keys 1-4 to jump to
the corresponding area on the fractal.

## Performance

Rendering fractals in real-time is quite a demanding task. I've done my best to optimize the
OpenGL code I can but I imagine the game will get quite choppy on older graphics cards. I can
only speak from my personal experience, but my GTX1060 can manage a reasonable framerate at
1080p, your mileage may vary.
