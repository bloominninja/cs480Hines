Project 8 - Bullet Demo
========================================

Developed By: Bryan Hines, Greg Stayner
Version 1.0


Building This Example
---------------------
To build this example cd to P8 directory, then

>$ cd build
>$ make

*If you are using a Mac you will need to edit the makefile in the build directory*

The excutable will be put in the bin folder.

To Run executable
navigate to bin
.$ ./BulletTest


Additional Notes For OSX Users
------------------------------

Ensure that the latest version of the Developer Tools is installed.



Running This Example
---------------------
After starting the program with .$ ./BulletTest the program will land on a view of the board, onto which our objects will fall. The cube is stationary, but the sphere and cylinder can both be moved. The sphere is controlled with the arrow keys, and the cylinder takes input from the 'wasd' keys.

Press the escape key at any time to exit the program.

Extra Credit
---------------------
Our sphere is a triangle mesh loaded from a .obj file rather than a primitive. The code from lines 508 to 530 is where we call our overloaded loadOBJ function which fills the triangle mesh (implementation of which starts on line 845), then go through the process of adding it to the dynamics world.

Known Bugs/fixes
---------------------
Motions can be a little 'jerk-ey' in that they'll start ramping up, then suddenly gain rapidly.
