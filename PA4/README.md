Obj Loader
========================================

Building This Example
---------------------


To build this example cd to p4 directory, then

>$ cd build
>$ make

*If you are using a Mac you will need to edit the makefile in the build directory*

The excutable will be put in bin

Additional Notes For OSX Users
------------------------------

Ensure that the latest version of the Developer Tools is installed.


Running This Example
---------------------
Navigate to line 32 and replace name with the model obj you want to load

Place all obj files into bin

This Program has not been tested with multiple paths

Known Bugs
---------------------
The obj loader is incomplete, it loads three types of obj files
but does not display them correctly It was to my understanding
that vectors needed to be normalised, however I have found no 
such tutorial as to which vectors should be normalised. 

The table looks like a bunch of sticks

