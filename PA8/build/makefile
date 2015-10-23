# Linux
CC=g++
# ECC LIBS
LIBS= -lglut -lGLEW -lGL -lassimp -lMagick++ -lMagickWand -lMagickCore -lBulletDynamics -lBulletSoftBody -lBulletCollision -lLinearMath

# Home LIBS
#LIBS= -lglut -lGLEW -lGL -lassimp -lMagick++-6.Q16 -lMagickWand-6.Q16 -lMagickCore-6.Q16 -lBulletDynamics -lBulletSoftBody -lBulletCollision -lLinearMath

# For Macs uncomment the next line and comment out the previous one
#CC=clang++
#LIBS=  -framework OpenGL -framework GLUT -framework Cocoa -lGLEW -stdlib=libc++
# Assuming you want to use a recent compiler
#
# Compiler flags for ECC machines ###########################
CXXFLAGS= -g -Wall -std=c++0x -fopenmp -DMAGICKCORE_HDRI_ENABLE=0 -DMAGICKCORE_QUANTUM_DEPTH=16 -I/usr/include/x86_64-linux-gnu//ImageMagick -I/usr/include/ImageMagick -I/usr/include/bullet

# Compiler flags for home workstation #######################
#CXXFLAGS= -g -Wall -std=c++0x -fopenmp -DMAGICKCORE_HDRI_ENABLE=0 -DMAGICKCORE_QUANTUM_DEPTH=16 -I/usr/include/x86_64-linux-gnu//ImageMagick-6 -I/usr/include/ImageMagick-6 -I/usr/include/bullet

all: ../bin/Matrix

../bin/Matrix: ../src/main.cpp
	$(CC) $(CXXFLAGS) ../src/main.cpp -o ../bin/BulletTest $(LIBS)

clean:
	rm -rf *.o ../bin/BulletTest
