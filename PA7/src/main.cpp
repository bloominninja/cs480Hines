#define GLM_FORCE_RADIANS
#include <GL/glew.h> // glew must be included before the main gl libs
#include <GL/glut.h> // doing otherwise causes compiler shouting
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>// for file and extended functuality with file 
#include <chrono>
#include <vector>// Included for dynamic upload
using namespace std;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include "shader.cpp"
#include <assimp/postprocess.h>
#include <assimp/color4.h>
//#include "planet.h"

//--Data types
//This object will define the attributes of a vertex(position, color, etc...)
struct Vertex
{
    GLfloat position[3];
    GLfloat uv[2];
};

//Pass triangle number to add model
struct Geometry
{
 GLuint vbo_geometry;
 int size;
};


struct Satilite
{
	glm::mat4 Model;
	float scale;
	float distance;
	float rotation;
};

struct Planet
{
	Satilite Data;
	int Moon_Number;
	Satilite* moons=new Satilite[5];
};





//--Evil Global variables
//Just for this example!
int w = 640, h = 480;// Window size
GLuint program;// The GLSL program handle

Geometry Geo[5];// the wonderful shapes we loaded with our model loader


//uniform locations
GLint loc_mvpmat;// Location of the modelviewprojection matrix in the shader

//attribute locations
GLint loc_position;
GLint loc_color;

//Number of models in system
int total=0;
//maximum number of allowed models
const int maxi = 10;

Planet earth;

//transform matrices
glm::mat4 model[maxi];//obj->world each object should have its own model matrix
glm::mat4 view;//world->eye
glm::mat4 projection;//eye->clip
glm::mat4 mvp[maxi];//premultiplied modelviewprojection
glm::mat4 hold;

//Camera Controlls
glm::vec3 Position= glm::vec3 (0.0, 16.0, -64.0);
glm::vec3 From_Pos= glm::vec3 (0.0, 0.0, 0.0);
glm::vec3 Look_At = glm::vec3(0.0, 0.0, 0.0);
glm::vec3 From_Angle= glm::vec3(0.0, 1.0, 0.0);
bool finished=false;


//Glut Menu Static vals
static int menu_id;
static int submenu_id;

//Global String for planet rotation
//const char* GL_STR="test";
    
bool spin=true;
bool rotate1=true;
bool flip=true;
//--GLUT Callbacks
void render();
void update();
void UpdateCamera(float,float,float,float);
bool Operate(float&val, float stable, float dt,float scale);
void reshape(int n_w, int n_h);
void keyboard(unsigned char key, int x_pos, int y_pos);
void mouse(int button, int state,int x, int y);
void special(int key, int x, int y);
//Add model and return adress
void addModel(int geometryIndex,int& modelIndex, glm::mat4& set);

bool loadOBJ( const char * path,std::vector < Vertex > & out_vertices);
void Orbit_Planet(Planet&planet, float dt);
void Orbit_Moon(Satilite&moon, float dt, glm::mat4 transist);


//void printtext(int x, int y, string String); 

//Menu Functions
void menu(int number);
void options();

//--Resource management
bool initialize(char* arg);
void cleanUp();

//--Random time things
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;


//--Main
int main(int argc, char **argv)
    {
    // Variable declaration
        // File path variable
    char argument[128]="";

    // Find file specified by command line argument.
    ifstream fin;
    strcat(argument,argv[1]);
    cout << argv[1];
    fin.open(argument);
    if(fin.good())
        {
        cout<< " located." << endl;
        }
    else
        {
        strcpy(argument,"dragon.obj");
        }
    
    fin.close();
    
    // Initialize GLUT.
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    // Name and create the Window
    glutCreateWindow("SUPER AWESOME MODALLUUU LOAAAAAADER!");
    
    // Now that the window is created the GL context is fully set up
    // Because of that we can now initialize GLEW to prepare work with shaders
    GLenum status = glewInit();
    if( status != GLEW_OK)
        {
        std::cerr << "[F] GLEW NOT INITIALIZED: ";
        std::cerr << glewGetErrorString(status) << std::endl;
        return -1;
        }
    
    // Set all of the callbacks to GLUT that we need
    glutDisplayFunc(render);// Called when its time to display
    glutReshapeFunc(reshape);// Called if the window is resized
    glutIdleFunc(update);// Called if there is nothing else to do
    glutKeyboardFunc(keyboard);// Called if there is keyboard input
    glutMouseFunc(mouse);
    
    // Initialize all of our resources(shaders, geometry)
    bool init = initialize(argument);
    if(init)
        {
        t1 = std::chrono::high_resolution_clock::now();
        glutMainLoop();
        }
    
    
    // Clean up after ourselves
    cleanUp();
    return 0;
    }

//--Implementations
void render()
{
    //--Render the scene
	 int ind=0;
    //clear the screen
    glClearColor(0.0, 0.0, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	 
    //premultiply the matrix for each with addModel
    addModel(1,ind,earth.Data.Model);
    addModel(1,ind,earth.moons[0].Model);
     addModel(1,ind,model[0]);
    earth.Data.distance=5.0;
    earth.Data.scale=.5;
    earth.Moon_Number=1;
    earth.moons[0].scale=.25;
    earth.moons[0].distance=10;

    //clean up
    glDisableVertexAttribArray(loc_position);
    glDisableVertexAttribArray(loc_color);


	glEnable(GL_DEPTH_TEST);
     //char str[64];
    //sprintf(str, GL_STR);
   // printtext(10,10,GL_STR); 
              
    //swap the buffers
    glutSwapBuffers();


}

void update()
{
    //total time
    //Set rotational and translate angles to 0
    static float tangle = 0.0;
    static float rangle = 0.0;
    static float mtangle = 0.0;
    static float mrangle = 0.0;
	 static float sam=0.6;
    float dt = getDT();// if you have anything moving, use dt.
    float x_point;
    float y_point;
    float z_point;
    //Check for interrupt booleans
    if (rotate1)
	{
    	tangle += dt * M_PI/2;
mtangle += dt * M_PI;

	}
    else
	{
	tangle -= dt * M_PI/2;
mtangle += dt * M_PI;
	}

    if (flip&&spin)
	{
//GL_STR="Planet is rotating counter clockwise";
	rangle += dt * M_PI/2;
mrangle += dt * M_PI;
	}
    else if(!flip&&spin)
	{
//GL_STR="Planet is rotating clockwise";
	rangle -= dt * M_PI/2;
mrangle += dt * M_PI;
	}
	else if(!spin)
	{
//GL_STR="Planet is not rotating";

}
    //Set degrees to arbitrary points
    x_point=5.0 * sin(tangle);
    y_point=.1;
    z_point=5.0 * cos(tangle); //move through 90 degrees a second

if (spin)
{
//hold=glm::rotate(glm::mat4(1.0f),rangle, glm::vec3(0.0, 1.0, 0.0 ));
//
}
else
{
//hold=hold;

}
glm::mat4 test;


Orbit_Planet(earth,dt);
    test = glm::translate( glm::mat4(1.0f), glm::vec3(x_point, y_point, z_point));
	
//    earth.Data.Model=test;
 //   earth.Data.Model*=glm::rotate(glm::mat4(1.0f),rangle, glm::vec3(0.0, 1.0, 0.0 ));
//model[0].local;

//test*=glm::rotate(glm::mat4(1.0f),sam, glm::vec3(1.0, 0.0, 1.0 ));
//	test*=glm::scale( glm::mat4(1.0f), glm::vec3(2.5, 2.5, 2.5));
	// test*=glm::translate( glm::mat4(1.0f), glm::vec3( 4*sin(mtangle), y_point, 4*cos(mtangle)));
  //  earth.moons[0].Model =test;
  //  earth.moons[0].Model *=glm::rotate(glm::mat4(1.0f),rangle*2, glm::vec3(0.0, 1.0, 0.0 ));
//
    // Update the state of the scene



     
    glutPostRedisplay();//call the display callback

 UpdateCamera(dt,1,1,1);

/*	if(From_Pos.x < Position.x)
	{
	 From_Pos.x+=Position.x*dt;
	}
	if(From_Pos.y < Position.y)
	{
		 From_Pos.y+=Position.y*dt;
	}
	if(From_Pos.y < Position.y)
	{
	 From_Pos.z+=Position.z*dt;
	}*/


}


void reshape(int n_w, int n_h)
{
    w = n_w;
    h = n_h;
    //Change the viewport to be correct
    glViewport( 0, 0, w, h);
    //Update the projection matrix as well
    //See the init function for an explaination
    projection = glm::perspective(45.0f, float(w)/float(h), 0.01f, 100.0f);

}

void keyboard(unsigned char key, int x_pos, int y_pos)
{
    // Handle keyboard input
    if(key == 27)//ESC to quit
    {
        exit(0);
    }
    if(key == 97)//A to rotate
    {
        rotate1=!rotate1;
    }


}
void special(int key, int x, int y)
{

    if(key == GLUT_KEY_LEFT)//A to rotate
    {
        rotate1=true;
    }
    if(key == GLUT_KEY_RIGHT)//A to rotate
    {
        rotate1=false;
    }
}

void mouse(int button, int state,int x, int y)
{
if(button==GLUT_LEFT_BUTTON&&state==GLUT_UP)
{

flip=!flip;
}
}

bool initialize(char* arg)
{
    // Initialize basic geometry and shaders for this example
	 Shader nexus;
	 ofstream fout;
std::vector< Vertex > vertices;
    string shaderCode;
    const char* tmp=new char[500];
    char* pass;
    // Load the specified model.
    loadOBJ(arg,vertices);


    //this defines a cube, this is why a model loader is nice
    //you can also do this with a draw elements and indices, try to get that working
    Vertex geometry[] = { {{-1.0, -1.0, -1.0}, {0.0, 0.0}},
                          {{-1.0, -1.0, 1.0}, {0.0, 0.0}},
                          {{-1.0, 1.0, 1.0}, {0.0, 0.0}},

                          {{1.0, 1.0, -1.0}, {0.0, 0.0}},
                          {{-1.0, -1.0, -1.0}, {0.0, 0.0}},
                          {{-1.0, 1.0, -1.0}, {0.0, 0.0}},
                          
                          {{1.0, -1.0, 1.0}, {0.0, 0.0}},
                          {{-1.0, -1.0, -1.0}, {0.0, 0.0}},
                          {{1.0, -1.0, -1.0}, {0.0, 0.0}},
                          
                          {{1.0, 1.0, -1.0}, {0.0, 0.0}},
                          {{1.0, -1.0, -1.0}, {0.0, 0.0}},
                          {{-1.0, -1.0, -1.0}, {0.0, 0.0}},

                          {{-1.0, -1.0, -1.0}, {0.0, 0.0}},
                          {{-1.0, 1.0, 1.0}, {0.0, 0.0}},
                          {{-1.0, 1.0, -1.0}, {0.0, 0.0}},

                          {{1.0, -1.0, 1.0}, {0.0, 0.0}},
                          {{-1.0, -1.0, 1.0}, {0.0, 0.0}},
                          {{-1.0, -1.0, -1.0}, {0.0, 0.0}},

                          {{-1.0, 1.0, 1.0}, {0.0, 0.0}},
                          {{-1.0, -1.0, 1.0}, {0.0, 0.0}},
                          {{1.0, -1.0, 1.0}, {0.0, 0.0}},
                          
                          {{1.0, 1.0, 1.0}, {0.0, 0.0}},
                          {{1.0, -1.0, -1.0}, {0.0, 0.0}},
                          {{1.0, 1.0, -1.0}, {0.0, 0.0}},

                          {{1.0, -1.0, -1.0}, {0.0, 0.0}},
                          {{1.0, 1.0, 1.0}, {0.0, 0.0}},
                          {{1.0, -1.0, 1.0}, {0.0, 0.0}},

                          {{1.0, 1.0, 1.0}, {0.0, 0.0}},
                          {{1.0, 1.0, -1.0}, {0.0, 0.0}},
                          {{-1.0, 1.0, -1.0}, {0.0, 0.0}},

                          {{1.0, 1.0, 1.0}, {0.0, 0.0}},
                          {{-1.0, 1.0, -1.0}, {0.0, 0.0}},
                          {{-1.0, 1.0, 1.0}, {0.0, 0.0}},

                          {{1.0, 1.0, 1.0}, {0.0, 0.0}},
                          {{-1.0, 1.0, 1.0}, {0.0, 0.0}},
                          {{1.0, -1.0, 1.0}, {0.0, 0.0}}
                        };
                        
    // Create a Vertex Buffer object to store this vertex info on the GPU
    glGenBuffers(1, &Geo[1].vbo_geometry);
    glBindBuffer(GL_ARRAY_BUFFER, Geo[1].vbo_geometry); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(geometry), geometry, GL_STATIC_DRAW);
    
   glGenBuffers(1, &Geo[2].vbo_geometry);
   glBindBuffer(GL_ARRAY_BUFFER, Geo[2].vbo_geometry);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices.front(), GL_STATIC_DRAW);

// test
////test over


    //--Geometry done

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    //Shader Sources
    // Put these into files and write a loader in the future
    // Note the added uniform!


    //compile the shaders
    GLint shader_status;



 pass=new char [190];
unsigned int len;

//Give a maximum number of attempts to load the vertex shader 
int attempts=10;
//Load tge vertex shader
do
{

nexus.load("vs.txt");

shaderCode = nexus.hold.c_str();
len=nexus.length;
strcpy(pass,shaderCode.c_str());


attempts-=1;
}
while(len!=strlen(pass)&& attempts>0);

//Pass the temperary variable to a pointer
    tmp = pass;


    // Vertex shader first
    glShaderSource(vertex_shader, 1,&tmp, NULL);
    glCompileShader(vertex_shader);
    //check the compile status
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &shader_status);
    if(!shader_status)
    {
        std::cerr << "[F] FAILED TO COMPILE VERTEX SHADER!" << std::endl;
        return false;

    }
//Bad code but if I loaded these together fs became loaded with garbage after shader
//was applied will improve later
const char *fs=nexus.load("fs.txt");
fout.clear();
//fout.open("test2.txt");
fout.close();
    // Now the Fragment shader
    glShaderSource(fragment_shader, 1, &fs, NULL);
    glCompileShader(fragment_shader);
    //check the compile status
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &shader_status);
    if(!shader_status)
    {
        std::cerr << "[F] FAILED TO COMPILE FRAGMENT SHADER!" << std::endl;
        return false;
    }

    //Now we link the 2 shader objects into a program
    //This program is what is run on the GPU
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    //check if everything linked ok
    glGetProgramiv(program, GL_LINK_STATUS, &shader_status);
    if(!shader_status)
    {
        std::cerr << "[F] THE SHADER PROGRAM FAILED TO LINK" << std::endl;
        //return false;
    }

    //Now we set the locations of the attributes and uniforms
    //this allows us to access them easily while rendering
    loc_position = glGetAttribLocation(program,
                    const_cast<const char*>("v_position"));
    if(loc_position == -1)
    {
        std::cerr << "[F] POSITION NOT FOUND" << std::endl;
        //return false;
    }

    loc_color = glGetAttribLocation(program,
                    const_cast<const char*>("v_color"));
    if(loc_color == -1)
    {
        std::cerr << "[F] V_COLOR NOT FOUND" << std::endl;
        //return false;
    }

    loc_mvpmat = glGetUniformLocation(program,
                    const_cast<const char*>("mvpMatrix"));
    if(loc_mvpmat == -1)
    {
        std::cerr << "[F] MVPMATRIX NOT FOUND" << std::endl;
        //return false;
    }
    
    //--Init the view and projection matrices
    //  if you will be having a moving camera the view matrix will need to more dynamic
    //  ...Like you should update it before you render more dynamic 
    //  for this project having them static will be fine
    //view = glm::lookAt( From_Pos,Look_At,From_Angle); 

    projection = glm::perspective( 45.0f, //the FoV typically 90 degrees is good which is what this is set to
                                   float(w)/float(h), //Aspect Ratio, so Circles stay Circular
                                   0.01f, //Distance to the near plane, normally a small value like this
                                   100.0f); //Distance to the far plane, 


    //enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //attatch options window to scene
    options(); 



    //and its done

    return true;
}

void cleanUp()
{
    // Clean up, Clean up
    glDeleteProgram(program);
    glDeleteBuffers(1, &Geo[1].vbo_geometry);
    glDeleteBuffers(1, &Geo[2].vbo_geometry);
}

//returns the time delta
float getDT()
	{
	    float ret;
	    t2 = std::chrono::high_resolution_clock::now();
	    ret = std::chrono::duration_cast< std::chrono::duration<float> >(t2-t1).count();
	    t1 = std::chrono::high_resolution_clock::now();
	    return ret;
	}
//////////////////Added Functionality 

//Menu functions 
void menu(int number)
	{
		if (number==0)
			{
				exit(0);
			}
		else if(number==1)
			{
			//Disable cube rotations
				spin=false;
			}
			//Enable cube rotations
		else if(number==2)
			{
				spin=true;
			}
	}

//Menu Display
void options()
	{
	//Create the submenu for the menu
		submenu_id = glutCreateMenu(menu);
		glutAddMenuEntry("STOP THE PRESSES", 1);
		glutAddMenuEntry("Revv it up", 2);
	//Create menu
		menu_id = glutCreateMenu(menu);
		glutAddSubMenu("Spin speed", submenu_id);  
		glutAddMenuEntry("Quit", 0);     
	//Make the menu appear on right click
		glutAttachMenu(GLUT_RIGHT_BUTTON);
	}

void addModel(int geometryIndex,int& modelIndex, glm::mat4& set)
	{
	 mvp[modelIndex] = projection * view* set;

    //enable the shader program
    glUseProgram(program);

    //upload the matrix to the shader
    glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp[modelIndex]));
  
    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(loc_position);
    glEnableVertexAttribArray(loc_color);  
    glBindBuffer(GL_ARRAY_BUFFER, Geo[geometryIndex].vbo_geometry);

    //set pointers into the vbo for each of the attributes(position and color)
    glVertexAttribPointer( loc_position,//location of attribute
                           3,//number of elements
                           GL_FLOAT,//type
                           GL_FALSE,//normalized?
                           sizeof(Vertex),//stride
                           0);//offset

    glVertexAttribPointer( loc_color,
                           3,
                           GL_FLOAT,
                           GL_FALSE,
                           sizeof(Vertex),
                           (void*)offsetof(Vertex,uv));

    glDrawArrays(GL_TRIANGLES, 0, 36);//mode, starting index, count

	modelIndex+=1;
	 
	}
	
bool loadOBJ(const char * path,std::vector < Vertex > & out_vertices)
    {
    // Variable declarations
    Assimp::Importer importer;
    Vertex tmpVert;
    const aiScene *myScene = importer.ReadFile(path, aiProcess_Triangulate);
    unsigned int mNumFaces;

    // variables tried while troubleshooting, but weren't needed.  Delete them?
        //unsigned int mNumMeshes;
        //unsigned int *mIndices;
        //unsigned int mNumIndices=3; 
        //aiMesh **mMeshes;
        //aiFace *mFaces;
        //aiVector3D *mVertices;
        //mNumMeshes=myScene->mNumMeshes;
        //mMeshes=myScene->mMeshes;
    
    
    mNumFaces = myScene->mMeshes[0]->mNumFaces;
    out_vertices.clear();



	for(unsigned int x=0; x<(mNumFaces);x++)
		{
		for(int z=0; z<3; z++)
			{
		    tmpVert.position[0]=myScene->mMeshes[0]->mVertices[myScene->mMeshes[0]->mFaces[x].mIndices[z]].x;          
          tmpVert.position[1]=myScene->mMeshes[0]->mVertices[myScene->mMeshes[0]->mFaces[x].mIndices[z]].y;
          tmpVert.position[2]=myScene->mMeshes[0]->mVertices[myScene->mMeshes[0]->mFaces[x].mIndices[z]].z;

				if(myScene->mMeshes[0]->HasTextureCoords(0/* Change with num meshes*/ ))
					{
					 tmpVert.uv[0]=myScene->mMeshes[0]->mTextureCoords[0][myScene->mMeshes[0]->mFaces[x].mIndices[z]].x;
					 tmpVert.uv[1]=myScene->mMeshes[0]->mTextureCoords[0][myScene->mMeshes[0]->mFaces[x].mIndices[z]].y;
					}
				else
					{
					 tmpVert.uv[0]=0;
					 tmpVert.uv[1]=0;
					}

				out_vertices.push_back(tmpVert);

            }
		}

      
    return true;
    }
void UpdateCamera(float dt, float xMod, float yMod, float zMod)
	{

	if(!finished)
	 {
		if(Operate(From_Pos.y, Position.y, dt, 1),Operate(From_Pos.y, Position.y, dt, 1.9),
		Operate(From_Pos.z, Position.z, dt, 1.9))
		{
		 finished=true;
		}
	 }
	 if(finished)
	 {
	  From_Pos=Position;
	 }

	    view = glm::lookAt( From_Pos,Look_At, From_Angle); 
	}
	
bool Operate(float&val,float stable,float dt, float scale)
{
 		if(stable<0)
			{
			 if(val<stable+.1)
			 {
			 	val=stable;
			 	return true;
			 }
			 if(val>stable)
				 {
				  val+=stable*dt*scale;
				 }
			 else
				 {
				  val-=stable*dt*scale;
				 }
			}
		else
			{
			if(val>stable-.1)
			 {
			 	val=stable;
			 	return true;
			 }
			 if(val>stable&&val>stable+1)
				 {
				  val-=stable*dt*scale;
				 }
			 else
				 {
				  val+=stable*dt*scale;
				 }
			}	
			return false;
	
}

void Orbit_Planet(Planet& planet, float dt)
{
glm::mat4 test;
glm::mat4 cam;
float x_point;
float y_point;
float z_point;
static float tangle = 0.0;

tangle += dt * M_PI/2;


x_point=planet.Data.distance * sin(tangle);
y_point=.1;
z_point=planet.Data.distance * cos(tangle);

//Orbit_Planet(earth,dt);

    test = glm::translate( glm::mat4(1.0f), glm::vec3(x_point, y_point, z_point));
    test *= glm::scale( glm::mat4(1.0f), glm::vec3(planet.Data.scale,planet.Data.scale,planet.Data.scale));
	 cam=glm::translate( glm::mat4(1.0f), glm::vec3(x_point, y_point, z_point))* glm::translate( glm::mat4(1.0f), glm::vec3(x_point*2, y_point, 2*z_point));
    planet.Data.Model=test;
    //earth.Data.Model*=glm::rotate(glm::mat4(1.0f),angle, glm::vec3(0.0, 1.0, 0.0 ));

 planet.Data.Model*=glm::rotate(glm::mat4(1.0f),tangle, glm::vec3(0.0, 1.0, 0.0 ));
  //cam*=glm::rotate(glm::mat4(1.0f),tangle, glm::vec3(0, 0, 1 ));  

  //          view = glm::lookAt(glm::vec3(cam[3].x,cam[3].y+7,cam[3].z) ,glm::vec3(0,0,0), From_Angle);
            cout<<Position.x<<' '<<Position.y<<' '<<Position.z<<endl;

 for(int i=0; i<planet.Moon_Number;i++)
 {
 Orbit_Moon(planet.moons[i],dt ,test);
 }
}
void Orbit_Moon(Satilite&moon, float dt , glm::mat4 transist)
{
static float angle = 0.0;

angle += dt * M_PI/2;


	 transist*=glm::scale( glm::mat4(1.0f), glm::vec3(moon.scale, moon.scale, moon.scale));
	 transist*=glm::translate( glm::mat4(1.0f), glm::vec3( moon.distance*sin(angle*2), .1, moon.distance*cos(angle*2)));
    moon.Model =transist;
    moon.Model *=glm::rotate(glm::mat4(1.0f),angle*2, glm::vec3(0.0, 1.0, 0.0 ));

}
