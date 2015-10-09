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

//--Data types
//This object will define the attributes of a vertex(position, color, etc...)
struct Vertex
    {
    GLfloat position[3];
    GLfloat uv[2];
    };

//--Evil Global variables
//Just for this example!
int w = 640, h = 480;// Window size
GLuint program;// The GLSL program handle
GLuint vbo_geometry;// VBO handle for our geometry

std::vector< Vertex > vertices;

//uniform locations
GLint loc_mvpmat;// Location of the modelviewprojection matrix in the shader

//attribute locations
GLint loc_position;

///Temperarily disabled for Vertex Modification
//GLint loc_color;

//transform matrices
glm::mat4 model;//obj->world each object should have its own model matrix
glm::mat4 view;//world->eye
glm::mat4 projection;//eye->clip
glm::mat4 mvp;//premultiplied modelviewprojection

//Glut Menu Static vals
static int menu_id;
static int submenu_id;
bool spin=true;
bool rotate2=true;
bool flip=true;

//--GLUT Callbacks
void render();
void update();
void reshape(int n_w, int n_h);
void keyboard(unsigned char key, int x_pos, int y_pos);
void mouse(int button, int state,int x, int y);

bool loadOBJ( const char * path,std::vector < Vertex > & out_vertices);
//Menu Functions
void menu(int number);
void options();

//--Resource management
bool initialize(char* fName);
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
    
    //clear the screen
    glClearColor(1, .9, 1, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //premultiply the matrix for this example
    mvp = projection * view * model;

    //enable the shader program
    glUseProgram(program);
    
    //upload the matrix to the shader
    glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp));
    
    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
    //set pointers into the vbo for each of the attributes(position and color)
    glVertexAttribPointer( 0,//location of attribute
                           3,//number of elements
                           GL_FLOAT,//type
                           GL_FALSE,//normalized?
                           sizeof(struct Vertex),//stride
                           0);//offset
    
    glVertexAttribPointer( 1,
                           3,
                           GL_FLOAT,
                           GL_FALSE,
                           sizeof(struct Vertex),
                           0);
    
/*    // Set material properties.  This code didn't work.
    glm::vec3* amb = new glm::vec3(1.0, 1.0, 1.0);
    glm::vec3* dif = new glm::vec3(0.001149, 0.002974, 0.250000);
    glm::vec3* spc = new glm::vec3(0.046403, 0.046950, 0.500000);
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, glm::value_ptr(amb));
    glMaterialfv(GL_FRONT, GL_DIFFUSE, glm::value_ptr(dif));
    glMaterialfv(GL_FRONT, GL_SPECULAR, glm::value_ptr(spc));
//    void glMaterialfv(GL_FRONT, GL_EMISSION, TYPE *param);
    glMaterialf(GL_FRONT, GL_SHININESS, 17.647059);
*/    
/*  // Nope, this didn't work either.  
    // Set the color
    glVertexAttrib3f(loc_color, 0.2f, 0.2f, 0.8f);
*/        


    glDrawArrays(GL_TRIANGLES, 0, vertices.size());//mode, starting index, count
    
    //clean up
    glDisableVertexAttribArray(loc_position);
    //glDisableVertexAttribArray(loc_color);
                           
    //swap the buffers
    glutSwapBuffers();
    }

void update()
    {
    //total time
    //Set rotational and translate angles to 0
    static float tangle = 0.0;
    static float rangle = 0.0;
    int spini;
    float dt = getDT();// if you have anything moving, use dt.

/*  Commented these out to get rid of warnings */
    //float x_point;
    //float y_point;
    //float z_point;
    //Check for interrupt booleans
    if (rotate2)
	    {
        tangle += dt * M_PI/2;
	    }
    else
	    {
	    tangle -= dt * M_PI/2;
	    }
    
    if (flip)
	    {
	    rangle += dt * M_PI/2;
	    }
    else
	    {
	    rangle -= dt * M_PI/2;
	    }
    //Set degrees to arbitrary points
/* Commented out to take care of warnings caused by suppression of translate function. 
    //x_point=5.0 * sin(tangle);
    //y_point=0.0;
    //z_point=5.0 * cos(tangle); //move through 90 degrees a second
*/
    if (spin)
        {
        spini=2;
        }
    
    else
        {
        spini=0;
        }
    
/*  // This is the code that makes the model orbit and rotate, since we're just showing off models
    // we only really need the rotation.
    model = glm::translate( glm::mat4(1.0f), 
                            glm::vec3(x_point, y_point, z_point))*glm::rotate(glm::mat4(1.0f),
                            rangle*spini, 
                            glm::vec3(0.0, 1.0, 0.0 ));

*/
    
    // Rotate the model relatively slowly so we can show off how pretty it is.
        // Second parameter for glm::rotate() is what controls the speed.
    model = glm::rotate(glm::mat4(1.0f), rangle*spini/4, glm::vec3(0, 1, 0));
    
    // Update the state of the scene
    glutPostRedisplay();//call the display callback
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
        rotate2=!rotate2;
        }
    }

void mouse(int button, int state,int x, int y)
    {
    if(button==GLUT_LEFT_BUTTON&&state==GLUT_UP)
        {
        flip=!flip;
        }
    }

bool initialize(char* fName)
    {
    // Initialize basic geometry and shaders for this example
    Shader nexus,nexus2;
    ofstream fout;
    // There was some weirdness going on with the shader loaders, so I changed the return type
    // to string and made a temporary character array to handle the shader code.
    string shaderCode;
    const char* tmp=new char[500];
    char* pass;
    // Load the specified model.
    loadOBJ(fName,vertices);
    
    // Create a Vertex Buffer object to store this vertex info on the GPU
    glGenBuffers(1, &vbo_geometry);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices.front(), GL_STATIC_DRAW);
    
    //--Geometry done
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    
    

    //compile the shaders
    GLint shader_status;
    
    // Load the Vertex shader, and extract the text from the string object.

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

    glShaderSource(vertex_shader, 1, &tmp, NULL);
    glCompileShader(vertex_shader);
    
    //check the compile status
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &shader_status);

/*  // Old shader error messages.  Delete?
    if(!shader_status)
        {
        std::cerr << "[F] FAILED TO COMPILE VERTEX SHADER!" << std::endl;
        return false;
        }
*/    
    
    // If there were errors compiling the shader, print information
        // code block taken from ogldev tutorial 4


    if (!shader_status) 
        {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(vertex_shader, sizeof(InfoLog), NULL, InfoLog);
        fprintf(stderr, "Vertex Shader %d: '%s'\n", vertex_shader, InfoLog);
        }
    
    // Get rid of vertex shader code; load fragment shader and extract text.
    shaderCode.clear();
nexus.output=false;
    shaderCode = nexus.load("fs.txt");
    tmp = shaderCode.c_str();
    
    // Now compile the Fragment shader
    glShaderSource(fragment_shader, 1, &tmp, NULL);
    glCompileShader(fragment_shader);

    //check the compile status
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &shader_status);

/*  // Old shader compilation error code.
    if(!shader_status)
        {
        std::cerr << "[F] FAILED TO COMPILE FRAGMENT SHADER!" << std::endl;
        return false;
        }
*/
    // If there were errors compiling the shader, print information
        // code block taken from ogldev tutorial 4
    if (!shader_status) 
        {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(fragment_shader, sizeof(InfoLog), NULL, InfoLog);
        fprintf(stderr, "Fragment Shader %d: '%s'\n", fragment_shader, InfoLog);
        }
    
    shaderCode.clear();
    
    
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
        return false;
        }

    //Now we set the locations of the attributes and uniforms
    //this allows us to access them easily while rendering
    loc_position = glGetAttribLocation(program,
                    const_cast<const char*>("v_position"));

    if(loc_position == -1)
        {
        std::cerr << "[F] POSITION NOT FOUND" << std::endl;
        return false;
        }

    //loc_color = glGetAttribLocation(program,
                 //   const_cast<const char*>("v_color"));
/*
    if(loc_color == -1)
        {
        std::cerr << "[F] V_COLOR NOT FOUND" << std::endl;
        return false;
        }
*/
    loc_mvpmat = glGetUniformLocation(program,
                    const_cast<const char*>("mvpMatrix"));

    if(loc_mvpmat == -1)
        {
        std::cerr << "[F] MVPMATRIX NOT FOUND" << std::endl;
        return false;
        }
    
    
    
    //--Init the view and projection matrices
    //  if you will be having a moving camera the view matrix will need to more dynamic
    //  ...Like you should update it before you render more dynamic 
    //  for this project having them static will be fine
    view = glm::lookAt( glm::vec3(0.0, 8.0, -16.0), //Eye Position
                        glm::vec3(0.0, 0.0, 0.0), //Focus point
                        glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up

    projection = glm::perspective( 45.0f, //the FoV typically 90 degrees is good which is what this is set to
                                   float(w)/float(h), //Aspect Ratio, so Circles stay Circular
                                   0.01f, //Distance to the near plane, normally a small value like this
                                   100.0f); //Distance to the far plane, 


    //enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

// Cull backfacing polygons
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	


    //attatch options window to scene
    options(); 

    //and its done
    return true;
    }

void cleanUp()
    {
    // Clean up, Clean up
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo_geometry);
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

