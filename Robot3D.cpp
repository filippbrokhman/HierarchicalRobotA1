/*******************************************************************
		   Hierarchical Multi-Part Model Example
********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <gl/glut.h>
#include <utility>
#include <vector>
#include "VECTOR3D.h"
#include "cube.h"
#include "QuadMesh.h"

#define M_PI       3.14159265358979323846   // pi

const int vWidth  = 650;    // Viewport width in pixels
const int vHeight = 500;    // Viewport height in pixels

// Note how everything depends on robot body dimensions so that can scale entire robot proportionately
// just by changing robot body scale
float robotBodyWidth = 8.0;
float robotBodyLength = 24.0;
float robotBodyDepth = 8.0;
float headWidth = 0.7*robotBodyWidth;
float headLength = 0.4 * robotBodyWidth;
float headDepth = 0.7 * robotBodyWidth;
float innerRadiusSensor = headLength * 0.104;
float outerRadiusSensor = headLength * 0.27;
float turretBaseWidth = headWidth / 2;
float turretBaseLength = headLength / 2;
float turretBaseDepth = headDepth / 2;
float upperArmLength = robotBodyLength/1.5;
float upperArmWidth = 0.125*robotBodyWidth;
float turretLength = upperArmLength / 3;
float turretWidth = upperArmWidth;
float gunLength = upperArmLength / 4.0;
float gunWidth = upperArmWidth;
float gunDepth = upperArmWidth;
float stanchionLength = robotBodyLength;
float stanchionRadius = 0.1*robotBodyDepth;
float baseWidth = 2 * robotBodyWidth;
float baseLength = 0.25*stanchionLength;
float wheelInternalRadius = robotBodyLength / 6;
float wheelInternalLength = robotBodyWidth;

// Control Robot body rotation on base
float robotAngle = 0.0;
float wheelTurn = 0.0;

// Control arm rotation
float shoulderAngle = -40.0;
float gunAngle = -25.0;
float headAngle = 0.0;
float maxHeadAngleUp = -40;
float maxHeadAngleDown = 20;

// movement of robot
float movementPositionX = 0.0;
float movementPositionZ = 0.0; 
float deltaPositionX = 0.0;
float deltaPositionZ = 0.0;

// Spin Cube Mesh
float cubeAngle = 0.0;

// Lighting/shading and material properties for robot - upcoming lecture - just copy for now
// Robot RGBA material properties (NOTE: we will learn about this later in the semester)
GLfloat robotBody_mat_ambient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat robotBody_mat_specular[] = { 0.45f,0.55f,0.45f,1.0f };
GLfloat robotBody_mat_diffuse[] = { 0.1f,0.35f,0.1f,1.0f };
GLfloat robotBody_mat_shininess[] = { 32.0F };


GLfloat robotArm_mat_ambient[] = { 0.0215f, 0.1745f, 0.0215f, 0.55f };
GLfloat robotArm_mat_diffuse[] = { 0.5f,0.0f,0.0f,1.0f };
GLfloat robotArm_mat_specular[] = { 0.7f, 0.6f, 0.6f, 1.0f };
GLfloat robotArm_mat_shininess[] = { 32.0F };

GLfloat gun_mat_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat gun_mat_diffuse[] = { 0.01f,0.0f,0.01f,0.01f };
GLfloat gun_mat_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat gun_mat_shininess[] = { 100.0F };

GLfloat robotLowerBody_mat_ambient[] = { 0.25f, 0.25f, 0.25f, 1.0f };
GLfloat robotLowerBody_mat_diffuse[] = { 0.4f, 0.4f, 0.4f, 1.0f };
GLfloat robotLowerBody_mat_specular[] = { 0.774597f, 0.774597f, 0.774597f, 1.0f };
GLfloat robotLowerBody_mat_shininess[] = { 76.8F };


// Light properties
GLfloat light_position0[] = { -4.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_position1[] = { 4.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };


// Mouse button
int currentButton;

// A template cube mesh
CubeMesh *cubeMesh = createCubeMesh();

// A flat open mesh
QuadMesh *groundMesh = NULL;

// Structure defining a bounding box, currently unused
typedef struct BoundingBox {
    VECTOR3D min;
	VECTOR3D max;
} BBox;

// Default Mesh Size
int meshSize = 16;

// Prototypes for functions in this module
void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void mouseMotionHandler(int xMouse, int yMouse);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
void animationHandler(int param);
void drawRobot();
void drawBody();
void drawHead();
void drawLowerBody();
void drawLeftArm();
void drawRightArm();
void drawWheel();
void drawTireCube();
void drawTurretBase();
void drawTurret();
void drawSensor();
void drawLeftCap();
void drawRightCap();

int main(int argc, char **argv)
{
	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(vWidth, vHeight);
	glutInitWindowPosition(200, 30);
	glutCreateWindow("3D Hierarchical Example");

	// Initialize GL
	initOpenGL(vWidth, vHeight);

	// Register callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotionHandler);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(functionKeys);

	// Start event loop, never returns
	glutMainLoop();

	return 0;
}


// Set up OpenGL. For viewport and projection setup see reshape(). 
void initOpenGL(int w, int h)
{
	// Set up and enable lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
	
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);   // This second light is currently off

	// Other OpenGL setup
	glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
	glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see 
	glClearColor(0.4F, 0.4F, 0.4F, 0.0F);  // Color and depth for glClear
	glClearDepth(1.0f);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	// Other initializatuion
	// Set up ground quad mesh
	VECTOR3D origin = VECTOR3D(-16.0f, 0.0f, 16.0f);
	VECTOR3D dir1v = VECTOR3D(1.0f, 0.0f, 0.0f);
	VECTOR3D dir2v = VECTOR3D(0.0f, 0.0f, -1.0f);
	groundMesh = new QuadMesh(meshSize, 32.0);
	groundMesh->InitMesh(meshSize, origin, 32.0, 32.0, dir1v, dir2v);

	VECTOR3D ambient = VECTOR3D(0.0f, 0.05f, 0.0f);
	VECTOR3D diffuse = VECTOR3D(0.4f, 0.8f, 0.4f);
	VECTOR3D specular = VECTOR3D(0.04f, 0.04f, 0.04f);
	float shininess = 0.2;
	groundMesh->SetMaterial(ambient, diffuse, specular, shininess);

}


// Callback, called whenever GLUT determines that the window should be redisplayed
// or glutPostRedisplay() has been called.
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	// Create Viewing Matrix V
	// Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
	gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	// Draw Robot

	// Apply modelling transformations M to move robot
	// Current transformation matrix is set to IV, where I is identity matrix
	// CTM = IV
	drawRobot();

	// Example of drawing a mesh (closed cube mesh)
	glPushMatrix();
	// spin cube
	glTranslatef(8.0, 0, 3.0);
	glRotatef(cubeAngle, 0.0, 1.0, 0.0);
	glTranslatef(-8.0, 0, -3.0);
	// position and draw cube
	glTranslatef(8.0, 0, 3.0); 
	drawCubeMesh(cubeMesh);
	glPopMatrix();

	// Draw ground
	glPushMatrix();
	glTranslatef(0.0, -20.0, 0.0);
	groundMesh->DrawMesh(meshSize);
	glPopMatrix();

	glutSwapBuffers();   // Double buffering, swap buffers
}

void drawRobot()
{
	glPushMatrix();
	 // spin robot on base. 
	 glTranslatef(movementPositionX, 0.0, movementPositionZ);
	 glRotatef(robotAngle, 0.0, 1.0, 0.0);

	 //drawBody();
	//drawLeftArm();
	drawHead();
	drawRightArm();
	drawWheel();
	glPopMatrix();
	
	// don't want to spin fixed base
	//drawLowerBody();

	glPopMatrix();
}


void drawBody()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotBody_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotBody_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotBody_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotBody_mat_shininess);

	glPushMatrix();
	glScalef(robotBodyWidth, robotBodyLength, robotBodyDepth);
	glutSolidCube(1.0);
	glPopMatrix();
}

void drawHead()
{
	glPushMatrix();
	// Position head with respect to arm
	glTranslatef(-(upperArmWidth),0.0,0.0);// this will be done last
	glTranslatef(0, 0.5*upperArmLength-0.5*headLength, 0); 

	// rotate head
	glRotatef(headAngle, 1.0, 0.0, 0.0);

	drawTurretBase();
	drawSensor();

	// Set robot material properties per body part. Can have seperate material properties for each part
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotBody_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotBody_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotBody_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotBody_mat_shininess);

	// Build Head
	glPushMatrix();
	glScalef(headWidth, headLength, headDepth);
	glutSolidCube(1.0);
	glPopMatrix();

	glPopMatrix();
}

void drawLowerBody()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotLowerBody_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotLowerBody_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotLowerBody_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotLowerBody_mat_shininess);

	glPushMatrix();
	// Position stanchion and base with respect to body
	glTranslatef(0, -1.5*robotBodyLength, 0.0); // this will be done last

	// stanchion
	glPushMatrix();
	glScalef(stanchionRadius, stanchionLength, stanchionRadius);
	glRotatef(-90.0, 1.0, 0.0, 0.0);
	gluCylinder(gluNewQuadric(), 1.0, 1.0, 1.0, 20, 10);
	glPopMatrix();

	// base
	glPushMatrix();
	// Position base with respect to parent stanchion
	glTranslatef(0.0, -0.25*stanchionLength, 0.0);
	// Build base
	glScalef(baseWidth, baseLength, baseWidth);
	glRotatef(-90.0, 1.0, 0.0, 0.0);
	glutSolidCube(1.0);
	glPopMatrix();

	glPopMatrix();
}

void drawTireCube() {
	glPushMatrix();
	glTranslatef(-(0.5 * wheelInternalLength + 0.5), 0.5 * wheelInternalLength + 0.5, 0);
	glutSolidCube(1.0);
	glPopMatrix();
}

void drawWheel() {
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotArm_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotArm_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotArm_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotArm_mat_shininess);

	glPushMatrix();
	// Position wheel with respect to parent (robot)
	glTranslatef(0.5*robotBodyWidth, -0.5 * robotBodyLength, 0); // this will be done last
	glRotatef(wheelTurn, 1, 0, 0);

	drawRightCap();
	drawLeftCap();
	drawTireCube();

	// wheel
	glPushMatrix();
	glScalef(wheelInternalLength, wheelInternalRadius, wheelInternalRadius);
	glRotatef(-90.0, 0.0, 1.0, 0.0);
	gluCylinder(gluNewQuadric(), 1.0, 1.0, 1.0, 20, 10);
	glPopMatrix();

	glPopMatrix();
}

void drawLeftCap() {
	glPushMatrix();
	// Position cap with respect to parent (wheel)
	glTranslatef(-wheelInternalLength, 0, 0); // this will be done last

	// cap
	glPushMatrix();
	glRotatef(-90.0, 0.0, 1.0, 0.0);
	gluDisk(gluNewQuadric(), 0, wheelInternalRadius, 40, 40);
	glPopMatrix();

	glPopMatrix();
}

void drawRightCap() {
	glPushMatrix();
	// Position cap with respect to parent (wheel)
	//glTranslatef(wheelInternalLength, 0, 0); // this will be done last

	// cap
	glPushMatrix();
	glRotatef(90.0, 0.0, 1.0, 0.0);
	gluDisk(gluNewQuadric(), 0, wheelInternalRadius, 40, 40);
	glPopMatrix();

	glPopMatrix();
}

void drawTurretBase() {
	glPushMatrix();
	// Position turrets base with respect to the head
	//glTranslatef(-(upperArmWidth), 0.0, 0.0); // this will be done last
	glTranslatef(0, 0.5 * headLength + 0.5 * turretBaseLength, 0); 

	drawTurret();

	// Build Head
	glPushMatrix();
	glScalef(turretBaseWidth, turretBaseLength, turretBaseDepth);
	glutSolidCube(1.0);
	glPopMatrix();

	glPopMatrix();
}

void drawTurret()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotArm_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotArm_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotArm_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotArm_mat_shininess);

	glPushMatrix();

	// Rotate arm at shoulder
	/*
	glTranslatef(-(0.5*robotBodyWidth + 0.5*upperArmWidth), 0.5*upperArmLength, 0.0);
	glRotatef(shoulderAngle, 1.0, 0.0, 0.0);
	glTranslatef((0.5*robotBodyWidth + 0.5*upperArmWidth), -0.5*upperArmLength, 0.0);
	*/

	//position arm down with respect to the wheel
	//glTranslatef(0.0, -(0.5 * wheelInternalRadius), 0.0);
	// Position turret with respect to the robot, attaching it to turrets base
	glTranslatef(0.0, 0.0, 0.5*turretLength);
	//glTranslatef(-(upperArmWidth), 0.5 * upperArmLength + 0.5*turretWidth, 0);
	glRotatef(90.0, 1.0, 0.0, 0.0); //rotate to face gun forward.

	// build arm
	glPushMatrix();
	glScalef(turretWidth, turretLength, turretWidth);
	glutSolidCube(1.0);
	glPopMatrix();

	glPopMatrix();


}

void drawSensor() {
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotArm_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotArm_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotArm_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotArm_mat_shininess);

	glPushMatrix();

	
	glTranslatef(0, 0, 0.5*headDepth);
	glutSolidTorus(innerRadiusSensor,outerRadiusSensor,4,60);

	glPopMatrix();
}

void drawLeftArm()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotArm_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotArm_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotArm_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotArm_mat_shininess);

	glPushMatrix();
    // Position arm with respect to parent body
	glTranslatef(0.5*robotBodyWidth + 0.5*upperArmWidth, 0, 0.0); // this will be done last

	// build arm
	glPushMatrix();
	glScalef(upperArmWidth, upperArmLength, upperArmWidth);
	glutSolidCube(1.0);
	glPopMatrix();

	glPopMatrix();
}

void drawRightArm()
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, robotArm_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, robotArm_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, robotArm_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, robotArm_mat_shininess);

	glPushMatrix();

	// Rotate arm at shoulder
	/*
	glTranslatef(-(0.5*robotBodyWidth + 0.5*upperArmWidth), 0.5*upperArmLength, 0.0);
	glRotatef(shoulderAngle, 1.0, 0.0, 0.0);
	glTranslatef((0.5*robotBodyWidth + 0.5*upperArmWidth), -0.5*upperArmLength, 0.0);
	*/

	//position arm down with respect to the wheel
	glTranslatef(0.0,-(0.5*wheelInternalRadius),0.0);
	// Position arm with respect to the robot
	glTranslatef(-(0.5*robotBodyWidth + 0.5 * upperArmWidth), 0, 0.0);
	
	// build arm
	glPushMatrix();
	glScalef(upperArmWidth, upperArmLength, upperArmWidth);
	glutSolidCube(1.0);
	glPopMatrix();

	//  Gun
	/*
	glMaterialfv(GL_FRONT, GL_AMBIENT, gun_mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, gun_mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, gun_mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, gun_mat_shininess);

	glPushMatrix();
	// rotate gun
	glTranslatef(-(0.5*robotBodyWidth + 0.5*upperArmWidth), -(0.5*upperArmLength), 0.0);
	glRotatef(gunAngle, 1.0, 0.0, 0.0);
	glTranslatef((0.5*robotBodyWidth + 0.5*upperArmWidth), (0.5*upperArmLength ), 0.0);
	
	// Position gun with respect to parent arm 
	glTranslatef(0, -(0.5*upperArmLength + 0.5*gunLength), 0.0);

	// build gun
	glScalef(gunWidth, gunLength, gunDepth);
	glutSolidCube(1.0);
	glPopMatrix();
	*/
	glPopMatrix();


}

// Callback, called at initialization and whenever user resizes the window.
void reshape(int w, int h)
{
	// Set up viewport, projection, then change to modelview matrix mode - 
	// display function will then set up camera and do modeling transforms.
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLdouble)w / h, 0.2, 40.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
	gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

bool stop = false;

// Callback, handles input from the keyboard, non-arrow keys
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 't':

		break;
	case 'r':
		robotAngle += 2.0;
		break;
	case 'R':
		robotAngle -= 2.0;
		break;
	case 'a':
		if (headAngle < maxHeadAngleDown)
			headAngle += 2.0;
		break;
	case 'A':
		if (headAngle > maxHeadAngleUp)
			headAngle -= 2.0;
		break;
	case 'w':
		deltaPositionX = 2.0 * sin(robotAngle * M_PI / 180.0);
		deltaPositionZ = 2.0 * cos(robotAngle * M_PI / 180.0);
		movementPositionX += deltaPositionX;
		movementPositionZ += deltaPositionZ;
		wheelTurn += 20;
		break;
	case 's':
		deltaPositionX = 2.0 * sin(robotAngle * M_PI / 180.0);
		deltaPositionZ = 2.0 * cos(robotAngle * M_PI / 180.0);
		movementPositionX -= deltaPositionX;
		movementPositionZ -= deltaPositionZ;
		wheelTurn -= 20;
		break;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}


void animationHandler(int param)
{
	if (!stop)
	{
		shoulderAngle += 1.0;
		cubeAngle += 2.0;
		glutPostRedisplay();
		glutTimerFunc(10, animationHandler, 0);
	}
}



// Callback, handles input from the keyboard, function and arrow keys
void functionKeys(int key, int x, int y)
{
	// Help key
	if (key == GLUT_KEY_F1)
	{

	}
	// Do transformations with arrow keys
	//else if (...)   // GLUT_KEY_DOWN, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_LEFT
	//{
	//}

	glutPostRedisplay();   // Trigger a window redisplay
}


// Mouse button callback - use only if you want to 
void mouse(int button, int state, int x, int y)
{
	currentButton = button;

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
		{
			;

		}
		break;
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN)
		{
			;
		}
		break;
	default:
		break;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}


// Mouse motion callback - use only if you want to 
void mouseMotionHandler(int xMouse, int yMouse)
{
	if (currentButton == GLUT_LEFT_BUTTON)
	{
		;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}

