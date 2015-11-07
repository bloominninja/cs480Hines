Project 9 - Air HOOOOKEEEYYYY
========================================

Developed By: Bryan Hines, Greg Stayner
Version 1.0


Building This Example
---------------------
To build this example cd to P9 directory, then

>$ cd build
>$ make

*If you are using a Mac you will need to edit the makefile in the build directory*

The excutable will be put in the bin folder.

To Run executable
navigate to bin
.$ ./Hockey


Additional Notes For OSX Users
------------------------------

-Ensure that the latest version of the Developer Tools is installed.



Running This Example
---------------------
-After starting the program with .$ ./Hockey the arrow keys control the paddle on the right
the paddle on the right is controlled by clicking the mouse and dragging it where you want the
paddle to be

-Play against freinds to see who can get the biggest goal area

-Press the escape key at any time to exit the program.

Extra Credit
---------------------

-A dynamically increasing goal area, the more points a player has above their opponent the bigger 
their goal becomes

-Semi smooth transitioning camera for rotations

-Two player implementation

-AI implementation

//score

-Can Change Puck

-Controls adjust on rotate menu action


Known Bugs/fixes
---------------------
-Motions jump towards the end of the rotation path.

-Paddles sometimes rotate despite being locked


Listed bindings
---------------------
-q rotate left
-e rotate right
-w zoom in
-s zoom out
-(arrow keys move player 2)
-1 change player 1 puck to cube and back
-2 Change player 2 puck to cube and back