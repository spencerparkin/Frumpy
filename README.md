# Frumpy

This is a software-based renderer that doesn't need a GPU.  Of course, this also means it's
relatively slow, but it's still a fun exercise in computer graphics programming.  Note that it
is up to the host application to provide some way of getting the pixels to the screen.  Frumpy
only concerns itself with generating the pixel data.  Following is the latest screenshot.

![snapshot](https://github.com/spencerparkin/Frumpy/blob/master/Screenshot.png?raw=true)

As you can see, it looks like crap, and there's something wrong with the texturing on the spout.
I'm not yet sure where the math is becoming too unstable or is perhaps just plain wrong.

You can also see that it renders at an abismal frame-rate, even at low resolution.
I'm not sure how to get that to go faster at the moment.
