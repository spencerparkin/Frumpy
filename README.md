# Frumpy

This is a software-based renderer that doesn't need a GPU.  Of course, this also means it's
relatively slow, but it's still a fun exercise in computer graphics programming.  Note that it
is up to the host application to provide some way of getting the pixels to the screen.  Frumpy
only concerns itself with generating the pixel data.  Following is the latest screenshot.

![snapshot](https://github.com/spencerparkin/Frumpy/blob/master/Screenshot.png?raw=true)

Here I've compared what I'm rendering against what 3Ds Max renders.  As you can see, the Frumpy
render looks like total crap in comparison.  I still have some bugs to work out, clearly.
But it's getting there.

You can also see that it renders at an abismal 9.5 frames per second with only 400x400 pixel resolution.
I'm not sure how to get that to go faster at the moment.
