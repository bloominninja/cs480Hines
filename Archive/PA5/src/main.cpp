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
    GLfloat color[3];
};

//--Evil Global variables
//Just for this example!
int w = 640, h = 480;// Window size
GLuint program;// The GLSL program handle
GLuint vbo_geometry;// VBO handle for our geometry

//Input model name here
char argument[128]="cube1.obj";


//temp variables
std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
std::vector< glm::vec3 > temp_vertices;
std::vector< glm::vec2 > temp_uvs;
std::vector< glm::vec3 > temp_normals;



//uniform locations
GLint loc_mvpmat;// Location of the modelviewprojection matrix in the shader

//attribute locations
GLint loc_position;
GLint loc_color;

//transform matrices
glm::mat4 model;//obj->world each object should have its own model matrix
glm::mat4 view;//world->eye
glm::mat4 projection;//eye->clip
glm::mat4 mvp;//premultiplied modelviewprojection
bool loadOBJ(const char * path,std::vector < glm::vec3 > & out_vertices, std::vector < glm::vec2 > & out_uvs,std::vector < glm::vec3 > & out_normals);
//--GLUT Callbacks
void render();
void update();
void reshape(int n_w, int n_h);
void keyboard(unsigned char key, int x_pos, int y_pos);

//--Resource management
bool initialize();
void cleanUp();

//--Random time things
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;


//--Main
int main(int argc, char **argv)
{
    // Initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    // Name and create the Window
    glutCreateWindow("Matrix Example");
	
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

    // Initialize all of our resources(shaders, geometry)
    bool init = initialize();
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
    glClearColor(0.0, 0.0, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //premultiply the matrix for this example
    mvp = projection * view * model;

    //enable the shader program
    glUseProgram(program);

    //upload the matrix to the shader
    glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp));

    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(loc_position);
    glEnableVertexAttribArray(loc_color);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
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
                           (void*)offsetof(Vertex,color));

    glDrawArrays(GL_TRIANGLES, 0, 36);//mode, starting index, count

    //clean up
    glDisableVertexAttribArray(loc_position);
    glDisableVertexAttribArray(loc_color);
                           
    //swap the buffers
    glutSwapBuffers();
}

void update()
{
    //total time
    static float angle = 0.0;
    float dt = getDT();// if you have anything moving, use dt.

    angle += dt * M_PI/2; //move through 90 degrees a second
 //   model = glm::translate( glm::mat4(1.0f), glm::vec3(5.0 * sin(angle), 0.0, 5.0 * cos(angle)))*glm::rotate(glm::mat4(1.0f),angle*2, glm::vec3(0.0, 1.0, 0.0 ));
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
    if(key == 27)//ESC
    {
        exit(0);
    }
}

bool initialize()
{
    // Initialize basic geometry and shaders for this example
Shader nexus;
ofstream fout;




    //this defines a cube, this is why a model loader is nice
    //you can also do this with a draw elements and indices, try to get that working

                               std::vector< glm::vec3 > vertices;
std::vector< glm::vec2 > uvs;
std::vector< glm::vec3 > normals; // Won't be used at the moment.
if( loadOBJ(argument, vertices, uvs, normals))
{
    // Create a Vertex Buffer object to store this vertex info on the GPU
    glGenBuffers(1, &vbo_geometry);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_geometry);
   glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

}
    //--Geometry done

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    //Shader Sources
    // Put these into files and write a loader in the future
    // Note the added uniform!


    //compile the shaders
    GLint shader_status;

const char *vs=nexus.load("vs.txt");

    // Vertex shader first
    glShaderSource(vertex_shader, 1,&vs, NULL);
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
fout.open("test2.txt");
fout<<fs;
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
    view = glm::lookAt( glm::vec3(0.0, 16.0, -32.0), //Eye Position
                        glm::vec3(0.0, 0.0, 0.0), //Focus point
                        glm::vec3(0.0, 1.0, 0.0)); //Positive Y is up

    projection = glm::perspective( 45.0f, //the FoV typically 90 degrees is good which is what this is set to
                                   float(w)/float(h), //Aspect Ratio, so Circles stay Circular
                                   0.01f, //Distance to the near plane, normally a small value like this
                                   100.0f); //Distance to the far plane, 

    //enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

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



bool loadOBJ(const char * path,std::vector < glm::vec3 > & out_vertices,
std::vector < glm::vec2 > & out_uvs,std::vector < glm::vec3 > & out_normals)
{
Assimp::Importer importer;
const aiScene *myScene =importer.ReadFile(path, aiProcess_Triangulate);
unsigned int mNumFaces;
unsigned int mNumMeshes;
unsigned int *mIndices;
unsigned int mNumIndices=3; 
glm::vec3 tmpVert;
aiMesh **mMeshes;
aiFace *mFaces;
aiVector3D *mVertices;

mNumMeshes=myScene->mNumMeshes;

mMeshes=myScene->mMeshes;
if(myScene->mMeshes[0]->HasFaces())
{
if(mNumMeshes==1)
{
	
	//Trouble point try 0/1
	mNumFaces=myScene->mMeshes[0]->mNumFaces;

	for(int x=0; x<mNumFaces;x++)
		{
		 tmpVert.x=myScene->mMeshes[0]->mVertices[myScene->mMeshes[0]->mFaces[x].mIndices[0]].x;
		 tmpVert.y=myScene->mMeshes[0]->mVertices[myScene->mMeshes[0]->mFaces[x].mIndices[1]].y;
		 tmpVert.z=myScene->mMeshes[0]->mVertices[myScene->mMeshes[0]->mFaces[x].mIndices[2]].x;

		out_vertices.push_back(tmpVert);
	  	// out_vertices[x*3]=myScene->mMeshes[0]->mVertices[x].x;
		//out_vertices[x*3+1]=myScene->mMeshes[0]->mVertices[x].y;
		//out_vertices[x*3+2]=myScene->mMeshes[0]->mVertices[x].z;
			
			/*for(int i=0; i<mNumIndices; i++)
			{
				
				
				//build tmpVert
				out_vertices.push_back(tmpVert);
			}*/
}
}
else
{
 //what... what you gunna do?
}

}

}


/*
bool loadOBJ(const char * path,std::vector < glm::vec3 > & out_vertices,
std::vector < glm::vec2 > & out_uvs,std::vector < glm::vec3 > & out_normals)
{
std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
std::vector< glm::vec3 > temp_vertices;
std::vector< glm::vec2 > temp_uvs;
std::vector< glm::vec3 > temp_normals;

	FILE * file = fopen(path, "r");
	ofstream fout("test.txt");
	char* test=new char [128];



	bool cont=true;

		if( file == NULL )
			{
				printf("Impossible to open the file !\n");
				return false;
			}
		else while(cont)
		{

		 char line[128];
		 int read =fscanf(file,"%s", line);
		 //fout<<line<<endl;
		 if (read==EOF)
		 	{
		 	 cont=false;
		 	 fout<<"end of file";
		 	 //break;
		 	}
		 else if (cont)
		  {
		   //Check Vertices
		 	if ( strcmp( line, "v" ) == 0 )
		 		{
    		 	 glm::vec3 vertex;
    			 fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
    			 temp_vertices.push_back(vertex);
		  		}
		  	//Check VT's
		  	else if ( strcmp( line, "vt" ) == 0 )
		  		{
    			 glm::vec2 uv;
    			 fscanf(file, "%f %f\n", &uv.x, &uv.y );
    			 temp_uvs.push_back(uv);
    			}
		  	// Check Normals	
		  	else if ( strcmp( line, "vn" ) == 0 )
		  		{
    			 glm::vec3 normal;
    			 fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
    			 temp_normals.push_back(normal);
    			 
    			 
    			}
    		else if ( strcmp( line, "f" ) == 0 )
    			{
    			
    			 std::string vertex1, vertex2, vertex3;
    			 unsigned int vertexIndex[100], uvIndex[100], normalIndex[100];
//435500
				//int matches = fscanf(file, "%d%d%d", &vertexIndex[0], &uvIndex[0], &normalIndex[0] );
					
				fscanf(file,"%d\n",&vertexIndex[0]);
					
				
				//LOOK AHEAD A BIT
				
				test=fgets(test, 3,file);
				fseek ( file, -2 , SEEK_CUR );
				
				
				//no uv index
				if ( strcmp(test, "//") == 0)
				{
				fout<<test<<' ';
				 fscanf(file, "//%d %d//%d %d//%d\n", &normalIndex[0], &vertexIndex[1],  
				 &normalIndex[1], &vertexIndex[2],  &normalIndex[2] );
				 vertexIndices.push_back(vertexIndex[0]);
    			 vertexIndices.push_back(vertexIndex[1]);
    			 vertexIndices.push_back(vertexIndex[2]);
				 normalIndices.push_back(normalIndex[0]);
    			 normalIndices.push_back(normalIndex[1]);
    			 normalIndices.push_back(normalIndex[2]);	
    			 
    			 		 
				}
				else if((test[0]== '/')&&strcmp(test, "//") != 0)
				{fout<<test<<' ';
				
				 fscanf(file, "/%d/%d %d/%d/%d %d/%d/%d\n", &uvIndex[0], &normalIndex[0], 
				 &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], 
				 &uvIndex[2],&normalIndex[2] );
				 vertexIndices.push_back(vertexIndex[0]);
    			 vertexIndices.push_back(vertexIndex[1]);
    			 vertexIndices.push_back(vertexIndex[2]);
				 uvIndices    .push_back(uvIndex[0]);
    			 uvIndices    .push_back(uvIndex[1]);
    			 uvIndices    .push_back(uvIndex[2]);
		       normalIndices.push_back(normalIndex[0]);
   		    normalIndices.push_back(normalIndex[1]);
  		       normalIndices.push_back(normalIndex[2]);
				}
				else
				{
fout<<test<<' ';
				 fscanf(file,"%d %d\n",&vertexIndex[1],&vertexIndex[2]);
				}
				





// For each vertex of each triangle

for( unsigned int i=0; i<vertexIndices.size(); i++ ){
unsigned int vertexIndex = vertexIndices[i];
glm::vec3 vertex = temp_vertices[ vertexIndex-1 ];
out_vertices.push_back(vertex);
		   } 
		 	
    			}

		  }
		 
		}

		 	
		return true;
}*/
