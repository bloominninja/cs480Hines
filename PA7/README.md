Model Loader PA5
========================================

Building This Example
---------------------


To build this example cd to P5 directory, then

>$ cd build
>$ make

*If you are using a Mac you will need to edit the makefile in the build directory*

The excutable will be put in bin


Additional Notes For OSX Users
------------------------------

Ensure that the latest version of the Developer Tools is installed.



Running This Example
---------------------
Navigate to the bin and run the matrix executable along with the file name of the "obj" file to load. Example: "./Matrix dragon.obj". If obj is not found the loader will load the default dragon model.

Right click in the window to access the menu and change rotation speed

Left click to change the rotation of the model

Press escape or use the right click menu to quit


Known Bugs/fixes
---------------------
Working off the old PA2, stopping and rotating the model is not seamless. Models are now all displayed with one hard-coded color (as opposed to the leftover color interpolation from the first few projects' shaders), but there's no proper lighting or materials interaction yet so it's almost impossible to make out contours.

Dragon model loads on its side. Not sure what the cause is.

Greg: I thought I'd squashed the bug where the fragment shader fails to compile when loading sphere.obj, but it's back now. First few answers I found on stackexchange suggest that the problem is with our handling of the source code strings, but given that this seems specific to the sphere.obj file (even the dragon.obj loads fine, albeit sideways), I have my doubts. The threads in question:
-http://stackoverflow.com/questions/6400985/opengl-shader-compilation-issue-unexpected-eof
-http://stackoverflow.com/questions/10877386/opengl-shader-compilation-errors-unexpected-undefined-at-token-undefined
-http://stackoverflow.com/questions/11138705/is-there-anything-wrong-with-my-glsl-shader-loading-code
