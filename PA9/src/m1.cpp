#define GLM_FORCE_RADIANS
#include <GL/glew.h> // glew must be included before the main gl libs
#include <GL/glut.h> // doing otherwise causes compiler shouting
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>// for file and extended functuality with file 
#include <chrono>
#include <vector>// Included for dynamic upload
#include <Magick++.h>
#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> //Makes passing matrices to shaders easier
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include "shader.cpp"
#include <assimp/postprocess.h>
#include <assimp/color4.h>
#include <queue>
#include <ft2build.h> //FreeType Text engine headers
#include FT_FREETYPE_H // Pango is a layout engine, which we may need 
#include FT_OUTLINE_H // for advanced stuff.  Investigate later.

using namespace Magick;
using namespace std;

//--Data types
//This object will define the attributes of a vertex(position, color, etc...)
struct Vertex
{
    GLfloat position[3];
    GLfloat uv[2];
    btTriangleMesh* collide;
};

//Pass triangle number to add model
struct Geom
{
 GLuint vbo_geometry;
 int size;
};

struct model_pack
{
 glm::mat4 model;
 btRigidBody* body;
 btCollisionShape * shape;
 btCollisionObject* object;
 btVector3 dim=btVector3 (1.0f,1.0f,1.0f);
 float mass=1;
};

// I was getting a weird warning about btInfinityMask never being used,
// and this inline function is for the sole purpose of suppressing it.

/*inline int bullet_btInfinityMask()
{
 return btInfinityMask;
}*/

//--Evil Global variables
//Just for this example!
int w = 1280, h = 960;// Window size
GLuint program;// The GLSL program handle

Geom Geo[5];
GLuint vbo_texture[50];
GLuint samplerVar;

//uniform locations
GLint loc_mvpmat;// Location of the modelviewprojection matrix in the shader

//attribute locations
GLint loc_position;
GLint loc_texture;


const int maxi = 15;


//transform matrices
glm::mat4 view;//world->eye
glm::mat4 projection;//eye->clip
glm::mat4 mvp[maxi];//premultiplied modelviewprojection

// Our physics objects
model_pack physicsCube;
model_pack paddle1;
model_pack paddle2;
model_pack puck;
model_pack flr;
model_pack wall[8];

//glm::mat4 cube;
//Camera Controlls
glm::vec3 Position= glm::vec3 (0, 40.0, -.1);
glm::vec3 to_Push= glm::vec3 (0.0, 0, 0);
glm::vec3 From_Pos= glm::vec3 (0.0, 0.0, 0.0);
glm::vec3 Look_At = glm::vec3(0.0, 0.0, 0.0);
glm::vec3 From_Angle= glm::vec3(0.0, 1.0, 0.0);
bool finished=false;
bool stop=true;
bool scaled=false;
bool started=true;
bool game=false;
bool up,down,lft,rht;
bool stop_paddles=true;
float look=-1;
queue <glm::vec3> jumpto;
//jumpto.push(Position);

////////////////////////////////////////////////////////BULLET STUFF/////////////////////////////////////////////////////////////////
float toX=0;
float toY=0;



/////
btBroadphaseInterface *broadphase = new btDbvtBroadphase();
btDefaultCollisionConfiguration *collisionConfiguration = new btDefaultCollisionConfiguration();
btCollisionDispatcher *dispatcher = new btCollisionDispatcher(collisionConfiguration);
btSequentialImpulseConstraintSolver *solver = new btSequentialImpulseConstraintSolver;


btDiscreteDynamicsWorld *dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration); 

// FreeType resources ///////////////////////////////////////////////////
FT_Error FTErr;
FT_Library library;
FT_Face FTFace;


/////////////////////////////////////////////////////////////////////////


/////////////////////////////////Collision implementation/////////////////////
//What is this here for?
#define BIT(x) (1<<(x))

enum collisiontypes {
    COL_NOTHING = 0, //<Collide with nothing
    COL_BOUND= BIT(8),
    COL_PADDLE = BIT(4), //<Collide with paddles
    COL_WALL = BIT(2), //<Collide with walls
    COL_PUCK = BIT(1)//<collide with bounding wall>

};

int paddleColl = COL_PUCK|COL_WALL|COL_BOUND;
int wallColl = COL_PUCK|COL_PADDLE;
int puckColl = COL_PADDLE|COL_WALL;
int boundColl= COL_PADDLE;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Glut Menu Static vals
static int menu_id;
static int submenu_id;

//Global String for planet rotation
//const char* GL_STR="test";
int pacing=1;   
int currIndex=0;
bool spin=true;
bool rotate1=true;
bool flip=false;
bool psh=false;
//--GLUT Callbacks
void render();
void update();
void UpdateCamera(float,float,float,float);
int Operate(float&val, float stable, float dt,float scale);
void reshape(int n_w, int n_h);
void keyboard(unsigned char key, int x_pos, int y_pos);
void mouse(int button, int state,int x, int y);
void motion(int,int);
void special(int key, int x, int y);
void specialUp(int key, int x, int y);
void updatePaddleMouseMove();
void updatePaddleKeyboardMove();
//Add model and return adress
void addModel(int geometryIndex,int& modelIndex,int imageIndex, glm::mat4& set);

// The overloaded version of the function is for loading custom meshes into Bullet's
// physics system.
bool loadOBJ( const char* path, std::vector<Vertex> &out_vertices);
bool loadOBJ( const char* path, std::vector<Vertex> &out_vertices, btTriangleMesh* customShape);

void applyPhysics(btCollisionShape*& shape, btRigidBody*& body, float x, float y, float z, float m);

void createBoard(float x, float y, float z, float gap);
void updateItem(model_pack& pack);
btVector3 GetOGLPos(int x, int y);

//FreeType Implementations
void drawBitmap( FT_Bitmap*  bitmap, FT_Int x, FT_Int y);
//void showImage(unsigned char* image, int HEIGHT, int WIDTH);

//void printtext(int x, int y, string String); 
void loadTexture(int i, const char* name);
//Menu Functions
void menu(int number);
void options();

//--Resource management
bool initialize();
void cleanUp();

//--Random time things
float getDT();
std::chrono::time_point<std::chrono::high_resolution_clock> t1,t2;

// Set board Dimensions 

float board_x=20;
float board_y=3;
float board_z=10;
float board_width=1.5;

// Set wait frames
int waiting = 2000;
int clocking=waiting;
//--Main
int main(int argc, char **argv)
    {
    // Variable declaration
    up=false;
    down=false;
    lft=false;
    rht=false;
    
    //Set Camera start
    jumpto.push(Position);
    
    // Initialize ImageMagick package.
    InitializeMagick(argv[0]);
    
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(w, h);
    
    // Initialize FreeType
    FTErr = FT_Init_FreeType(&library);
    
    // Check for proper initialization.
    if(FTErr)
        {
        cout << "An error occurred during FreeType initialization." << endl;
        }
    
    // Load the typeface we'll be using. Use some default font for now.
    FTErr = FT_New_Face(library,
                        "/usr/share/fonts/truetype/freefont/FreeMono.ttf",
                        0,
                        &FTFace);
    
    // Check for errors
    if(FTErr == FT_Err_Unknown_File_Format)
        {
        cout << "Font format is unsupported." << endl;
        }
    
    else if(FTErr)
        {
        cout << "File could not be found, or there was an error opening the file." << endl;
        }
    
    // Set initial font size.
    FTErr = FT_Set_Char_Size(
                            FTFace, // Handle for typeface object
                            0,      // Character width.  0 means same as height.
                            16*64,  // Character height in 1/64th points.  AKA: this is 16 point font.
                            glutGet(GLUT_SCREEN_WIDTH),   // Screen horizontal resolution.
                            glutGet(GLUT_SCREEN_HEIGHT)); // Screen vertical resolution.
    
    // Name and create the Window
    glutCreateWindow("Mega Ultra SuperStar Hockey");
    
    // Now that the window is created the GL context is fully set up
    // Because of that we can now initialize GLEW to prepare work with shaders
    GLenum status = glewInit();
    if( status != GLEW_OK)
        {
        std::cerr << "[F] GLEW NOT INITIALIZED: ";
        std::cerr << glewGetErrorString(status) << std::endl;
        return -1;
        }
    
 
    //Set Gravity of world 
    dynamicsWorld->setGravity(btVector3(0, -10, 0));
   
    //Create Gameboard
    createBoard(board_x,board_y,board_z,board_width); 
   

    //Create Puck
    puck.shape= new btCylinderShape(puck.dim);
    applyPhysics(puck.shape,puck.body,0,1,4,10);
	puck.body->setRestitution(1);
	puck.dim=btVector3(1,1,1);

	dynamicsWorld->addRigidBody(puck.body, COL_PUCK, puckColl);
	puck.body->setRestitution(1);
		
   
  	//Create Boundry Wall
   physicsCube.dim=btVector3(25,2,.1);
   physicsCube.shape= new btBoxShape (physicsCube.dim);        
	applyPhysics(physicsCube.shape,physicsCube.body,0, 0, 0,0);
	dynamicsWorld->addRigidBody(physicsCube.body, COL_BOUND, boundColl);
	physicsCube.body->setRestitution(1);


	//Create Paddle 1	
	paddle1.dim=btVector3(1,1,1);		
	paddle1.shape= new btCylinderShape(paddle1.dim);
   applyPhysics(paddle1.shape,paddle1.body,0,20,15,500);
	dynamicsWorld->addRigidBody(paddle1.body, COL_PADDLE, paddleColl);
	
	//Create Paddle 2
	paddle2.dim=btVector3(1,1,1);		
	paddle2.shape= new btCylinderShape(paddle2.dim);
   applyPhysics(paddle2.shape,paddle2.body,0,20,-15,500);
	dynamicsWorld->addRigidBody(paddle2.body, COL_PADDLE, paddleColl);
	
	
	
    // Set all of the callbacks to GLUT that we need
    glutDisplayFunc(render);// Called when its time to display

    glutReshapeFunc(reshape);// Called if the window is resized

    //glutIdleFunc(update);// Called if there is nothing else to do

    glutKeyboardFunc(keyboard);// Called if there is keyboard input

    glutSpecialFunc(special);
    glutSpecialUpFunc(specialUp);

    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    	glutPassiveMotionFunc(motion);

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
	 int ind = 0, pen_x = 800, pen_y = 900;
	
    //FreeType resources
    FT_GlyphSlot glyph = FTFace->glyph;
    char text[] = "FreeType engine test. Can you hear me now?";
	
    //clear the screen
    glClearColor(0.75, 0.6, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLineWidth(2.5);
    
    // Placeholder text rendering code starts here.  Find a better place for it later.
    for(int i = 0; i < 43/*string length*/; i++)
        {
        // Load glyph image Load_Char just calls Get_Index then Load_Glyph
        FTErr = FT_Load_Char(
                            FTFace,             //Handle to the typeface.
                            text[i],            //Find and pass glyph index
                            FT_LOAD_RENDER);   //Load Flags
        
        if(FTErr)
            {
            continue;   // Ignore errors for now.
            }
        
/*        FTErr = FT_Render_Glyph(glyph, FT_RENDER_MODE_NORMAL); convert the glyph to a bitmap.
        
        if(FTErr)
            {
            continue;   // Ignore errors for now.
            }
*/        
        // Draw the character at the target position.  We must implement this.
        drawBitmap(&glyph ->bitmap,
                    pen_x + glyph->bitmap_left,
                    pen_y - glyph->bitmap_top);
        
        // Increment pen position
        pen_x += glyph->advance.x >> 6;
        pen_y += glyph->advance.y >> 6;
        }
    
    // Placeholder text rendering code ends here.
    
	//Render OBJ files
    addModel(2,ind,9,physicsCube.model);
    addModel(0,ind,11,paddle1.model);
    addModel(0,ind,11,paddle2.model);
    addModel(0,ind,8,puck.model);

    addModel(2,ind,10,flr.model);
    addModel(2,ind,7,wall[0].model);
    addModel(2,ind,7,wall[1].model);
    addModel(2,ind,7,wall[2].model);
    addModel(2,ind,7,wall[3].model);
    addModel(2,ind,6,wall[4].model);
    addModel(2,ind,7,wall[5].model);
    addModel(2,ind,7,wall[6].model);
    addModel(2,ind,6,wall[7].model);
    
    //clean up
    glDisableVertexAttribArray(loc_position);
    glDisableVertexAttribArray(loc_texture);


    glEnable(GL_DEPTH_TEST);
    //char str[64];
    //sprintf(str, GL_STR);
    // printtext(10,10,GL_STR); 

    update();
    
    

    //swap the buffers
    glutSwapBuffers();
    }

void update()
    {
	 
    float dt = getDT();// if you have anything moving, use dt.
    btTransform trans;
    if(!stop_paddles)
    {
		 if (flip)
		 {
		 updatePaddleMouseMove();
		 }
	 	 updatePaddleKeyboardMove();
 	 }

    
    //Special Move Functions
    
   
    
    //Step simulation
    dynamicsWorld->stepSimulation(dt, 10); 
    
    
    //Update and snap each item
    updateItem(paddle1);
    paddle1.body->setAngularFactor(btVector3(0,1,0));
    //paddle1.body->setLinearFactor(btVector3(1,0,1));

    updateItem(paddle2);
    paddle2.body->setAngularFactor(btVector3(0,1,0));
    //paddle2.body->setLinearFactor(btVector3(1,0,1));
    
    
    updateItem(puck);
    //puck.model*=glm::translate( glm::mat4(1.0f), glm::vec3(0, -8, 0));
		
    puck.body->getMotionState()->getWorldTransform(trans);
	 
	 if(clocking>1)
	 {
	 clocking-=1;
	 }
	 else if(clocking==1)
	 {
	  stop_paddles=false;
	  //puck.body->setLinearFactor(btVector3(1,1,1));
	  game=true;
	  clocking=0;
	 }
	 else if(clocking==0)
	 {
	     puck.body->setAngularFactor(btVector3(0,1,0));
    puck.body->setLinearFactor(btVector3(1,1,1));
    paddle1.body->setLinearFactor(btVector3(1,0,1));
    paddle2.body->setLinearFactor(btVector3(1,0,1));
	 }
	 
	 
	 //Check for goal condition
	 if(trans.getOrigin().getZ()>board_x)
	 {
	 //Player 2 Point stuff
	  btTransform transform = puck.body -> getCenterOfMassTransform();
	  transform.setOrigin(btVector3(0,1,0));
	 puck.body -> setCenterOfMassTransform(transform);
//reset
	 puck.body->setLinearVelocity(btVector3(.0001, 0,.0001));
	 }	 
	 else if(trans.getOrigin().getZ()<-board_x)
	 {
	 //Player 1 Point stuff
	  btTransform transform = puck.body -> getCenterOfMassTransform();
	  transform.setOrigin(btVector3(0,1,0));
	 puck.body -> setCenterOfMassTransform(transform);
	 //reset
	 puck.body->setLinearVelocity(btVector3(.0001, 0,.0001));
	 }

	 //Update walls, jic
	 updateItem(flr);
	 updateItem(physicsCube);
    updateItem(wall[0]);
    updateItem(wall[1]);
    updateItem(wall[2]);
    updateItem(wall[3]);
    updateItem(wall[4]);
    updateItem(wall[5]);    
    updateItem(wall[6]); 
    updateItem(wall[7]); 
    // Update the state of the scene
    glutPostRedisplay();//call the display callback

    UpdateCamera(dt,1,1,1);

    }


void reshape(int n_w, int n_h)
{
    w = n_w;
    h = n_h;
    //Change the viewport to be correct
    glViewport( 0, 0, w, h);
    //Update the projection matrix as well
    //See the init function for an explaination
    projection = glm::perspective(45.0f, float(w)/float(h), 0.01f, 100000000.0f);

}

void keyboard(unsigned char key, int x_pos, int y_pos)
    {
    // Handle keyboard input
    if(key == 27)//ESC to quit
        {
        exit(0);
        }
    /*//Change with new keys for 2nd player
    if(key == 119)//move cylander up
        {
        physicsCylinder.body->activate(true);
        physicsCylinder.body->applyCentralImpulse(btVector3(0, 0, 0.1));
        physicsCylinder.body->applyCentralForce(btVector3(0, 0, 10));
        }
    
    if(key == 97)//move cylander left
        {
        physicsCylinder.body->activate(true);
        physicsCylinder.body->applyCentralImpulse(btVector3(0.1, 0, 0));
        physicsCylinder.body->applyCentralForce(btVector3(10, 0, 0));
        }
    
    if(key == 115)//move Cylander down
        {
        physicsCylinder.body->activate(true);
        physicsCylinder.body->applyCentralImpulse(btVector3(0, 0, -0.1));
        physicsCylinder.body->applyCentralForce(btVector3(0, 0, -10));
        }
    
    if(key == 100)//move cylander right
        {
        physicsCylinder.body->activate(true);
        physicsCylinder.body->applyCentralImpulse(btVector3(-0.1, 0, 0));
        physicsCylinder.body->applyCentralForce(btVector3(-10, 0, 0));
        }
        */
    }

void special(int key, int x, int y)
	{

		//Set bool when key is down
		if(key== GLUT_KEY_UP)
			{
				up = true;
			}
		if(key== GLUT_KEY_DOWN)
			{
				down=true;
			}
		if(key== GLUT_KEY_RIGHT)
			{
				rht=true;
			}
		if(key==GLUT_KEY_LEFT)
			{
				lft=true;
			}

	}

void specialUp(int key, int x, int y)
	{

		//Clear when key is up
		if(key== GLUT_KEY_UP)
			{
				up = false;
			}
		if(key== GLUT_KEY_DOWN)
			{
				down=false;
			}
		if(key== GLUT_KEY_RIGHT)
			{
				rht=false;
			}
		if(key==GLUT_KEY_LEFT)
			{
				lft=false;
			}

	}

void mouse(int button, int state,int x, int y)
{
	btVector3 inter;
	btTransform trans;

	if(button==GLUT_LEFT_BUTTON&&state==GLUT_DOWN/*&&ObjectSelected*/)
	{
	
	flip=true;
	
	}
	 if(button==GLUT_LEFT_BUTTON&&state==GLUT_UP)
	{
	
	flip=false;
	
	
	}

	
}



void motion(int x, int y)
{
	float a;
	float b;
	btVector3 inter;
	btTransform trans;

	//Translate X and Y coordinants to x and z	
	a=GetOGLPos(x,y).getX();
	cout<<"X position is: "<<GetOGLPos(x,y).getX()<<endl;
	a/=.820;
	a*=-board_x;
	b=GetOGLPos(x,y).getY();
	cout<<"Z position is: "<<GetOGLPos(x,y).getY()<<endl;
	b/=.425;
	b*=10 ;
	toX=b;
	toY=a;
}

void updatePaddleKeyboardMove(/*we can put multiple controllers in here by placing an container here*/)
{
	//Set the paddle motions to active and set velocity to 0 *degrade velocity later
		paddle2.body->activate(true);
		paddle2.body->setLinearVelocity(btVector3(0, paddle2.body->getLinearVelocity().getY(),0));
		float horizontal=0;
		float vertical=0;
		

	//Set velocity depending on arrow press
		if(up)
			{
				if(look>0)
				{
					horizontal=-10;
				}
				else
				{
					vertical=10;
				}
			}
			
		if(down)
			{
				if(look>0)
				{
					horizontal=10;
				}
				else
				{
					vertical=-10;
				}
				
			}
		if(rht)
			{
				if(look>0)
				{
					vertical=-10;
				}
				else
				{
					horizontal=-10;
				}
			}
		if(lft)
			{
				if(look>0)
				{
					vertical=10;
				}
				else
				{
					horizontal=10;
				}
			}
	//Apply Velocity		
	paddle2.body->setLinearVelocity(btVector3(horizontal, paddle2.body->getLinearVelocity().getY(),vertical));
}

void updatePaddleMouseMove()
{
 	//Initialise Variables
	btTransform trans;
	btVector3 inter;
	//Call mouse position to get points


	
	
 	//Find where it is
	paddle1.body->getMotionState()->getWorldTransform(trans);
 	//Create a velocity vector between mouse and current puck position

	inter=btVector3((toX-trans.getOrigin().getX())*2,0,(toY-trans.getOrigin().getZ())*2);

	//Set active for movement
	paddle1.body->activate(true);

	//Get how fast its going now
    btVector3 velocity = paddle1.body->getLinearVelocity();
    btScalar speed = velocity.length();
    
/*    if(speed > 10)
        {
        float factor = 10.0/speed;
        
        
        }
*/
	if(speed < 1)
		{
		 paddle1.body->setLinearVelocity((velocity+inter)/2);
		}
   if(speed > 1) 
    	{
       velocity *= 1/speed;
       paddle1.body->setLinearVelocity(inter);          
      }

	//Get where it is
   paddle1.body->getMotionState()->getWorldTransform(trans);

	//Set invisible wall
	if(trans.getOrigin().getZ()<0.9)
	{
	btTransform transform = paddle1.body -> getCenterOfMassTransform();
	transform.setOrigin(btVector3(trans.getOrigin().getX(),trans.getOrigin().getY(),1.09));
	 paddle1.body -> setCenterOfMassTransform(transform);
	}
/*
if(abs(trans.getOrigin().getX())>board_x-2)
{
 if(trans.getOrigin().getX()<0)
 {
 sign=-1.0;
 }
 btTransform transform = paddle1.body -> getCenterOfMassTransform();
 transform.setOrigin(btVector3(sign*(board_x-2),trans.getOrigin().getY(),trans.getOrigin().getZ()));
 //paddle1.body -> setCenterOfMassTransform(transform);
}
*/
cout<< "x Coord: "<<trans.getOrigin().getX()<<endl;
cout<< "z coord: "<<trans.getOrigin().getZ()<<endl;
	
	
}


bool initialize()
{
    // Initialize basic geometry and shaders for this example
	 Shader nexus;
	 ofstream fout;
    std::vector<Vertex> vertices;
    string shaderCode;
    const char* tmp=new char[90];
    string pass;
    
    // btTriangleMesh holds Bullet's version of the vertices of our arbitrary mesh
    btTriangleMesh* customShape = new btTriangleMesh();
    
    // Start model loading.
    loadOBJ("cylinder.obj",vertices);
    
    glGenBuffers(1, &Geo[0].vbo_geometry);
    glBindBuffer(GL_ARRAY_BUFFER, Geo[0].vbo_geometry);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices.front(), GL_STATIC_DRAW);
    Geo[0].size=vertices.size();
    
    // This is the model we'll be using for the custom Collision shape.
    loadOBJ("sphere2.obj", vertices, customShape);
    
    glGenBuffers(1, &Geo[1].vbo_geometry);
    glBindBuffer(GL_ARRAY_BUFFER, Geo[1].vbo_geometry);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices.front(), GL_STATIC_DRAW);
    Geo[1].size=vertices.size();
    
    //This is the part where we register the shape with Bullet
    /*physicsSphere.shape = new btBvhTriangleMeshShape(customShape, true);
    btDefaultMotionState* extraCreditMotion = 
                new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), 
                                                    btVector3(0, 50, 0)));
                                                    
                                                
    btScalar mass(5);
    btVector3 inertia(0,0,0);
    physicsSphere.shape->calculateLocalInertia(mass, inertia);
    btRigidBody::btRigidBodyConstructionInfo shapeRigidBodyCI(
                                                            mass, 
                                                            extraCreditMotion,
                                                            physicsSphere.shape,
                                                            inertia);
    //shapeRigidBodyCI.m_friction=.7;                                                   
    physicsSphere.body = new btRigidBody(shapeRigidBodyCI);
    */
    //dynamicsWorld->addRigidBody(physicsSphere.body/*, COL_WALL, COL_WALL*/);
    
    // The above code is mostly redundant with applyPhysics, but it has no parameter
    // to pass a custom mesh, so I'm doing this for now.  Really should overload the function.
    //applyPhysics(physicsSphere.shape,physicsSphere.body,0, 50, 0,1);
    //btDefaultMotionState
    
    // Back to normal object loading
    loadOBJ("cube2.obj",vertices);
    
    glGenBuffers(2, &Geo[2].vbo_geometry);
    glBindBuffer(GL_ARRAY_BUFFER, Geo[2].vbo_geometry);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices.front(), GL_STATIC_DRAW);
    
	Geo[2].size=vertices.size();
    
    // Load textures, many of these are leftovers from the Solar System, investigate which ones
    // can be deleted. NO SPAGHETTI
  	loadTexture(1, "textures/mercury.jpg");
 	loadTexture(2, "textures/venus.jpg");   
	loadTexture(3, "textures/earth.jpg");
    loadTexture(4, "textures/mars.jpg");
    loadTexture(5, "textures/jupiter.jpg");
    loadTexture(6, "textures/saturn.jpg");
    loadTexture(7, "textures/uranus.jpg");
    loadTexture(8, "textures/neptune.jpg");
    loadTexture(9, "textures/pluto.jpg");
    loadTexture(10, "textures/luna.jpg");
    loadTexture(11, "textures/sun.jpg");                     
    
     
/*    loadOBJ("ring.obj",vertices);
    glGenBuffers(3, &Geo[3].vbo_geometry);
    glBindBuffer(GL_ARRAY_BUFFER, Geo[3].vbo_geometry);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices.front(), GL_STATIC_DRAW);
    Geo[3].size=vertices.size();*/
    
    
    //--Geometry done
    
    
    // Begin shader loading.
        //compile the shaders
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    GLint shader_status;
    pass=new char [900];
    unsigned int len;

    //Give a maximum number of attempts to load the vertex shader 
    int attempts=10;
    //Load tge vertex shader
    do
        {
        
        nexus.load("vs.txt");
        
        shaderCode = nexus.hold.c_str();
        len=nexus.length;
        pass=shaderCode;
        
        
        attempts-=1;
        }
    while(len!=pass.length()&& attempts>0);

    //Pass the temperary variable to a pointer
    tmp = pass.c_str();


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
    
    loc_texture = glGetAttribLocation(program,
                    const_cast<const char*>("v_texture"));
    
    if(loc_texture == -1)
        {
        std::cerr << "[F] TEXTURE NOT FOUND" << std::endl;
        return false;
        }
    
    
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
    //view = glm::lookAt( From_Pos,Look_At,From_Angle); 

    projection = glm::perspective( 45.0f, //the FoV typically 90 degrees is good which is what this is set to
                                   float(w)/float(h), //Aspect Ratio, so Circles stay Circular
                                   0.01f, //Distance to the near plane, normally a small value like this
                                   100000000.0f); //Distance to the far plane, 
    
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
    glDeleteBuffers(1, &Geo[1].vbo_geometry);
    //glDeleteBuffers(1, &Geo[2].vbo_geometry);
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
		else if(number==11) 
			{
				stop =true;
	
				//System[number-1].focus=true;
			}
		else if(number==13)
			{
				stop =false;
	
				//System[number-1].focus=true;
			}
	}

//Menu Display
void options()
	{
	//Create the submenu for the menu
		submenu_id = glutCreateMenu(menu);
		glutAddMenuEntry("Mercury", 1);

		
	//Create menu
		menu_id = glutCreateMenu(menu);
		glutAddMenuEntry("Start", 13); 
		glutAddSubMenu("Go To Planet", submenu_id);  
		glutAddMenuEntry("Put Time in a bottle", 11); 
		glutAddMenuEntry("Quit", 0);     
	//Make the menu appear on right click
		glutAttachMenu(GLUT_RIGHT_BUTTON);
	}

void addModel(int geometryIndex,int& modelIndex, int imageIndex, glm::mat4& set)
	{
	 mvp[modelIndex] = projection * view* set;

    //enable the shader program
    glUseProgram(program);
    glBindTexture(GL_TEXTURE_2D, vbo_texture[imageIndex]);  
    // Get the location of the sampler variable from the shader program.
    samplerVar = glGetUniformLocation(program, "texSampler");
    glUniform1i(samplerVar, 0);


    //upload the matrix to the shader
    glUniformMatrix4fv(loc_mvpmat, 1, GL_FALSE, glm::value_ptr(mvp[modelIndex]));
  
    //set up the Vertex Buffer Object so it can be drawn
    glEnableVertexAttribArray(loc_position);
    glEnableVertexAttribArray(1);  
    glBindBuffer(GL_ARRAY_BUFFER, Geo[geometryIndex].vbo_geometry);

    //set pointers into the vbo for each of the attributes(position and color)
    glVertexAttribPointer( loc_position,//location of attribute
                           3,//number of elements
                           GL_FLOAT,//type
                           GL_FALSE,//normalized?
                           sizeof(Vertex),//stride
                           0);//offset
        
    glVertexAttribPointer( loc_texture,
                           2,
                           GL_FLOAT,
                           GL_FALSE,
                           sizeof(struct Vertex),
                           (void*)offsetof(Vertex, uv)); 

    glDrawArrays(GL_TRIANGLES, 0, Geo[geometryIndex].size);//mode, starting index, count
    

    
    // Turn on the loaded texture.
    glBindTexture(GL_TEXTURE_2D, vbo_texture[imageIndex]);  
     
    glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, vbo_texture[imageIndex]);    

	modelIndex+=1;
	 
	}
	
// This version of loadOBJ is for normal loading of vertices to be rendered.
bool loadOBJ(const char* path, std::vector<Vertex> &out_vertices)
    {
    // Variable declarations
    Assimp::Importer importer;
    Vertex tmpVert;
    const aiScene *myScene = importer.ReadFile(path, aiProcess_Triangulate);
    unsigned int mNumFaces;
    
    
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
					 tmpVert.uv[1]=myScene->mMeshes[0]->mTextureCoords[0][myScene->mMeshes[0]->mFaces[x].mIndices[z]].y*-1;
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

// This second version of loadOBJ is for preparing custom meshes for bullet.
bool loadOBJ(const char* path, std::vector<Vertex> &out_vertices, btTriangleMesh* customShape)
    {
    // Variable declarations
    Assimp::Importer importer;
    Vertex tmpVert;
    const aiScene *myScene = importer.ReadFile(path, aiProcess_Triangulate);
    unsigned int mNumFaces;
    btVector3 triArray[3];
    
    mNumFaces = myScene->mMeshes[0]->mNumFaces;
    out_vertices.clear();
    
    
    
	for(unsigned int x=0; x<(mNumFaces);x++)
		{
		for(int z=0; z<3; z++)
			{
		    tmpVert.position[0]=myScene->mMeshes[0]->mVertices[myScene->mMeshes[0]->mFaces[x].mIndices[z]].x;          
          tmpVert.position[1]=myScene->mMeshes[0]->mVertices[myScene->mMeshes[0]->mFaces[x].mIndices[z]].y;
          tmpVert.position[2]=myScene->mMeshes[0]->mVertices[myScene->mMeshes[0]->mFaces[x].mIndices[z]].z;
            
            // prepare triArray for later loading into the bullet mesh structure.
            triArray[z] = btVector3(tmpVert.position[0], tmpVert.position[1], tmpVert.position[2]);
            
            if(myScene->mMeshes[0]->HasTextureCoords(0/* Change with num meshes*/ ))
                {
                tmpVert.uv[0]=myScene->mMeshes[0]->mTextureCoords[0][myScene->mMeshes[0]->mFaces[x].mIndices[z]].x;
                tmpVert.uv[1]=myScene->mMeshes[0]->mTextureCoords[0][myScene->mMeshes[0]->mFaces[x].mIndices[z]].y*-1;
                }
            else
                {
                tmpVert.uv[0]=0;
                tmpVert.uv[1]=0;
                }
            
            // add store the data in their correct structure and prepare for the next loop.
            out_vertices.push_back(tmpVert);
            }
        
        customShape->addTriangle(triArray[0], triArray[1], triArray[2]);
        }

      
    return true;
    }

void UpdateCamera(float dt, float xMod, float yMod, float zMod)
	{
	glm::vec3 Pos=jumpto.front();
//jumpto.front(glm::vec3 (0.0, 8.0, -16.0));
	 //check=test.front();
	 	
	//Check for finished camera motion
	if(!finished)
		{
		//Adjust camera
		Operate(From_Pos.x, Pos.x, dt, 1);

		Operate(From_Pos.y, Pos.y, dt, 2);

		Operate(From_Pos.z, Pos.z, dt, 1);
		 if(From_Pos==Position)
		 {	 
		 	finished=true;
		 	//Check intro camera
		 	if(started)
		 	{
		 		started=false;
		 	}
		 	else
		 	{
		 		stop=false;
		 	}
		 }
		}
	else
	 {
	  From_Pos=Position;
	 }

	    view = glm::lookAt( From_Pos,Look_At, From_Angle); 
	}
	
int Operate(float&val,float stable,float dt, float scale)
{
 //Check point comparison and slowly move to point	
 bool add,subtract;
 add=false;
 subtract=false;
		//Check positive or negative
		if((val<0&&stable>=0)||val<stable)
		{
		 add=true;
		}
		else if((val>0&&stable<=0)||val>stable)
		{
		subtract=true;

		}
		//Add values or subtract depending 
		if (add)
		{
		 val+=max(abs(stable),abs(val))*dt*scale;
		}
		else if(subtract)
		{
		
		 val-=max(abs(stable),abs(val))*dt*scale;
		 
		}
		//Set "snap point"
		if(abs(val)+1>abs(stable)&&abs(val)-1<abs(stable))
		{
		
		 val=stable;
		 
		}
		
		return 0;
			
	
}


void loadTexture(int i, const char* name)
{
    Image* textureImage = new Magick::Image(name);

    Blob* pixels = new Blob;
    textureImage->write(pixels, "RGBA");
    // Create an object to hold the textures
    glGenTextures(i, &vbo_texture[i]);
    glBindTexture(GL_TEXTURE_2D, vbo_texture[i]);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureImage->columns(), textureImage->rows(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 		 pixels->data());
    
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void applyPhysics(btCollisionShape*& shape, btRigidBody*& body, float x, float y, float z, float m)
{
		//Get shape and rigid body and pass to world
        btDefaultMotionState* MotionState = new btDefaultMotionState
        (btTransform(btQuaternion(0, 0, 0, 1), btVector3(x,y,z)));
        
        btScalar mass = m;
        btVector3 Inertia(0, 0, 0);
        
      //Check for static object
        if(m>0)
        {
        shape->calculateLocalInertia(mass, Inertia);       
        btRigidBody::btRigidBodyConstructionInfo RigidBodyCI(mass, MotionState, shape, Inertia);
                body = new btRigidBody(RigidBodyCI);
        dynamicsWorld->addRigidBody(body);
        }
        else
        {
         btRigidBody::btRigidBodyConstructionInfo RigidBodyCI(0, MotionState, shape, btVector3(0, 0, 0));
         
                 body = new btRigidBody(RigidBodyCI);
        dynamicsWorld->addRigidBody(body);
        }
  

}

void createBoard(float x, float y, float z, float gap)
{
			//Create floor
			flr.dim=btVector3(z,1,x);
			
         flr.shape= new btBoxShape (flr.dim);        
        	applyPhysics(flr.shape,flr.body,0, -1, 0,0);
        	dynamicsWorld->addRigidBody(flr.body, COL_WALL, wallColl);
        	flr.body->setFriction(0);
        	
        	
        	//Create Walls and collision of wall
       	wall[0].dim=btVector3(1,y,x);
        	wall[0].shape= new btBoxShape (wall[0].dim);
        	applyPhysics(wall[0].shape,wall[0].body,z, -1, 0,0);
        	dynamicsWorld->addRigidBody(wall[0].body, COL_WALL, wallColl);

        	wall[1].dim=btVector3(1,y,x);
        	wall[1].shape= new btBoxShape (wall[1].dim);
        	applyPhysics(wall[1].shape,wall[1].body,-z, -1, 0,0);
        	dynamicsWorld->addRigidBody(wall[1].body, COL_WALL, wallColl);

        	wall[2].dim=btVector3((z/2)-gap,y,1);
        	wall[2].shape= new btBoxShape (wall[2].dim);
        	applyPhysics(wall[2].shape,wall[2].body,(z/2)+gap, -1,x,0);
        	dynamicsWorld->addRigidBody(wall[2].body, COL_WALL, wallColl);

        	wall[3].dim=btVector3((z/2)-gap,y,1);
        	wall[3].shape= new btBoxShape (wall[3].dim);
        	applyPhysics(wall[3].shape,wall[3].body,(-z/2)-gap, -1, x,0);
        	dynamicsWorld->addRigidBody(wall[3].body, COL_WALL, wallColl);
        	
        	wall[4].dim=btVector3(gap*2,y,1);
        	wall[4].shape= new btBoxShape (wall[4].dim);
        	applyPhysics(wall[4].shape,wall[4].body,0, -1, x,0);
        	dynamicsWorld->addRigidBody(wall[4].body, COL_BOUND, boundColl);
        	
        	
        	
        	wall[5].dim=btVector3((z/2)-gap,y,1);
        	wall[5].shape= new btBoxShape (wall[5].dim);
        	applyPhysics(wall[5].shape,wall[5].body,(z/2)+gap, -1, -x,0);
        	dynamicsWorld->addRigidBody(wall[5].body, COL_WALL, wallColl);

        	wall[6].dim=btVector3((z/2)-gap,y,1);
        	wall[6].shape= new btBoxShape (wall[6].dim);
        	applyPhysics(wall[6].shape,wall[6].body,(-z/2)-gap, -1, -x,0);
        	dynamicsWorld->addRigidBody(wall[6].body, COL_WALL, wallColl);
        	
        	
        	wall[7].dim=btVector3(gap*2,y,1);
        	wall[7].shape= new btBoxShape (wall[7].dim);
        	applyPhysics(wall[7].shape,wall[7].body,0, -1, -x,0);
        	dynamicsWorld->addRigidBody(wall[7].body, COL_BOUND, boundColl);
        	
 	
 			//Set wall bounciness
        	wall[0].body->setRestitution (1);
        	wall[1].body->setRestitution (1);
        	wall[2].body->setRestitution (1);
        	wall[3].body->setRestitution (1);
         wall[4].body->setRestitution (1); 
         wall[5].body->setRestitution (1);   
         wall[6].body->setRestitution (1);  
         wall[7].body->setRestitution (1);	

}

void updateItem(model_pack& pack)
{
btTransform trans;
btScalar m[16]; 

		//Get body location and return mat4 matrix
		pack.body->getMotionState()->getWorldTransform(trans);
		trans.getOpenGLMatrix(m); 
		//Scale matrix appropriatly
		pack.model= glm::make_mat4(m)*glm::scale( glm::mat4(1.0f), glm::vec3(pack.dim.getX(),pack.dim.getY(),pack.dim.getZ()));

}

// basic function for drawing characters using Freetype.  Taken from the example1 tutorial at freetype.org.
    // write a better version if we have too much time on our hands.
void drawBitmap( FT_Bitmap*  bitmap, FT_Int x, FT_Int y)
    {
    FT_Int  i, j, p, q;
    FT_Int  x_max = x + bitmap->width;
    FT_Int  y_max = y + bitmap->rows;
    int HEIGHT = glutGet(GLUT_SCREEN_HEIGHT);
    int WIDTH = glutGet(GLUT_SCREEN_WIDTH);
    unsigned char image[HEIGHT][WIDTH];

    for (i = x, p = 0; i < x_max; i++, p++)
        {
        for (j = y, q = 0; j < y_max; j++, q++)
            {
            if (i < 0 || j < 0 || i >= WIDTH || j >= HEIGHT)
                {
                continue; //if the loop indices are outside the character box, ignore it.
                }
            
            image[j][i] |= bitmap->buffer[q * bitmap->width + p];
            }
        }
    
    //Put the bitmap on screen.
    //showImage(&image, HEIGHT, WIDTH);
    
    }

/*
// Second part of the character drawing FreeType implementations.
void showImage(unsigned char image, int HEIGHT, int WIDTH)
    {
    int  i, j;


    for ( i = 0; i < HEIGHT; i++ )
        {
        for ( j = 0; j < WIDTH; j++ )
            {
            putchar( image[i][j] == 0 ? ' ' : image[i][j] < 128 ? '+' : '*' );
            }
        putchar( '\n' );
        }
    }
*/

btVector3 GetOGLPos(int x, int y)//Code from http://nehe.gamedev.net/article/using_gluunproject/16013/ 
{
	
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY, winZ;
    GLdouble posX, posY, posZ;
 
    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    glGetDoublev( GL_PROJECTION_MATRIX, projection );
    glGetIntegerv( GL_VIEWPORT, viewport );
 
    winX = (float)x;
    winY = (float)viewport[3] - (float)y;
    glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
 
    gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
 
    return btVector3(posX, posY, posZ);
}
