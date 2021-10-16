#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif

#include "glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "glut.h"


//	This is a sample OpenGL / GLUT program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with a glut menu
//
//	The left mouse button does rotation
//	The middle mouse button does scaling
//	The user interface allows:
//		1. The axes to be turned on and off
//		2. The color of the axes to be changed
//		3. Debugging to be turned on and off
//		4. Depth cueing to be turned on and off
//		5. The projection to be changed
//		6. The transformations to be reset
//		7. The program to quit
//
//	Author:			Joe Graphics

// NOTE: There are a lot of good reasons to use const variables instead
// of #define's.  However, Visual C++ does not allow a const variable
// to be used as an array size or as the case in a switch( ) statement.  So in
// the following, all constants are const variables except those which need to
// be array sizes or cases in switch( ) statements.  Those are #defines.

// title of these windows:

const char *WINDOWTITLE = { "MineCraft" };
const char *GLUITITLE   = { "User Interface Window" };

// what the glui package defines as true and false:

const int GLUITRUE  = { true  };
const int GLUIFALSE = { false };

// the special key:

#define ESCAPE		0x1b
#define CONTROL     0x11

// initial window size:

const int INIT_WINDOW_SIZE = { 1000 };

// size of the 3d box:

const float BOXSIZE = { 2.f };

// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFP = { 0.5 };
const float ANGTP = { 0.3 };
const float ANGFR = { 1. };
const float SCLFACT = { 0.005f };

// minimum allowable scale factor:

const float MINSCALE = { 0.05f };

// scroll wheel button values:

const int SCROLL_WHEEL_UP   = { 3 };
const int SCROLL_WHEEL_DOWN = { 4 };

// equivalent mouse movement when we click a the scroll wheel:

const float SCROLL_WHEEL_CLICK_FACTOR = { 5. };

// active mouse buttons (or them together):

const int LEFT   = { 4 };
const int MIDDLE = { 2 };
const int RIGHT  = { 1 };

#define MS_IN_THE_ANIMATION_CYCLE   3000

// which projection:

enum Projections
{
	ORTHO,
	PERSP
};

// which button:

enum ButtonVals
{
	RESET,
	QUIT
};

// window background color (rgba):

const GLfloat BACKCOLOR[ ] = { 0., 0., 0., 1. };

// fog parameters:

const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE     = { GL_LINEAR };
const GLfloat FOGDENSITY  = { 0.30f };
const GLfloat FOGSTART    = { 1.5 };
const GLfloat FOGEND      = { 4. };


// what options should we compile-in?
// in general, you don't need to worry about these
// i compile these in to show class examples of things going wrong

//#define DEMO_Z_FIGHTING
//#define DEMO_DEPTH_BUFFER

// should we turn the shadows on?

//#define ENABLE_SHADOWS



// non-constant global variables:

int		ActiveButton;			// current button that is down
int		DebugOn;				// != 0 means to print debugging info
int		DepthCueOn;				// != 0 means to use intensity depth cueing
int		DepthBufferOn;			// != 0 means to use the z-buffer
int		DepthFightingOn;		// != 0 means to force the creation of z-fighting
GLuint	BoxList;				// object display list
int		MainWindow;				// window id for main graphics window
float	Scale;					// scaling factor
int		ShadowsOn;				// != 0 means to turn shadows on
int		Xmouse, Ymouse;			// mouse values
GLuint  Tex0;
GLuint  Tex1;
GLuint  Tex2;
GLuint  GrassList;
int     WhichSkin;
int     WhichView;
float   Xco, Yco, Zco;
float   HorAngle, VerAngle;
float   Xrot;

// function prototypes:

void	Animate( );
void	Display( );
void DoSkinMenu(int);
void DoViewMenu(int);
void	DoDepthBufferMenu( int );
void	DoDepthFightingMenu( int );
void	DoDepthMenu( int );
void	DoDebugMenu( int );
void	DoMainMenu( int );
//void	DoProjectMenu( int );
void	DoShadowMenu();
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( );
void	InitGraphics( );
void	InitLists( );
void	InitMenus( );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void MousePassMotion(int, int);
void	Reset( );
void	Resize( int, int );
void	Visibility( int );

void			Axes( float );
unsigned char *	BmpToTexture( char *, int *, int * );
void			HsvRgb( float[3], float [3] );
int				ReadInt( FILE * );
short			ReadShort( FILE * );

void			Cross(float[3], float[3], float[3]);
float			Dot(float [3], float [3]);
float			Unit(float [3], float [3]);

// main program:

int
main( int argc, char *argv[ ] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)

	glutInit( &argc, argv );

	// setup all the graphics stuff:

	InitGraphics( );

	// create the display structures that will not change:

	InitLists( );

	// init all the global variables used by Display( ):
	// this will also post a redisplay

	Reset( );

	// setup all the user interface stuff:

	InitMenus( );

	// draw the scene once and wait for some interaction:
	// (this will never return)

	glutSetWindow( MainWindow );
	glutMainLoop( );

	// glutMainLoop( ) never returns
	// this line is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutMainLoop( ) do it

void
Animate( )
{
	// put animation stuff in here -- change some global variables
	// for Display( ) to find:

	// force a call to Display( ) next time it is convenient:

	glutSetWindow( MainWindow );

	int ms = glutGet(GLUT_ELAPSED_TIME);	// milliseconds
	ms %= MS_IN_THE_ANIMATION_CYCLE;
	glutPostRedisplay();
}

void DrawBody()
{
	glPushMatrix();
	glTranslatef(-0.2314, -1.62, -0.3471);
	glScalef(0.0578, 0.0578, 0.0578);
	// Right leg
	glBegin(GL_QUADS);
		glNormal3f(0., 0., 1.);//front
		glTexCoord2f(0.0625, 0.375);
		glVertex3f(0., 12., 4.);	
		glTexCoord2f(0.125, 0.375);
		glVertex3f(4., 12., 4.);	
		glTexCoord2f(0.125, 0.);
		glVertex3f(4., 0., 4.);
		glTexCoord2f(0.0625, 0.);
		glVertex3f(0., 0., 4.);

		glNormal3f(0., 0., -1.);//back
		glTexCoord2f(0.1875, 0.375);
		glVertex3f(4., 12., 0.);
		glTexCoord2f(0.25, 0.375);
		glVertex3f(0., 12., 0.);	
		glTexCoord2f(0.25, 0.);
		glVertex3f(0., 0., 0.);
		glTexCoord2f(0.1875, 0.);
		glVertex3f(4., 0., 0.);

		glNormal3f(0., 1., 0.);//top
		glTexCoord2f(0.0625, 0.5);
		glVertex3f(0., 12., 0.);
		glTexCoord2f(0.125, 0.5);
		glVertex3f(4., 12., 0.);
		glTexCoord2f(0.125, 0.375);
		glVertex3f(4., 12., 4.);
		glTexCoord2f(0.0625, 0.375);
		glVertex3f(0., 12., 4.);


		glNormal3f(0., -1., 0.);//bottom
		glTexCoord2f(0.125, 0.5);
		glVertex3f(0., 0., 4.);
		glTexCoord2f(0.1875, 0.5);
		glVertex3f(4., 0., 4.);
		glTexCoord2f(0.1875, 0.375);
		glVertex3f(4., 0., 0.);
		glTexCoord2f(0.125, 0.375);
		glVertex3f(0., 0., 0.);


		glNormal3f(1., 0., 0.);//left
		glTexCoord2f(0.125, 0.375);
		glVertex3f(4., 12., 4.);
		glTexCoord2f(0.1875, 0.375);
		glVertex3f(4., 12., 0.);
		glTexCoord2f(0.1875, 0.);
		glVertex3f(4., 0., 0.);
		glTexCoord2f(0.125, 0.);
		glVertex3f(4., 0., 4.);

		glNormal3f(-1., 0., 0.);//right
		glTexCoord2f(0., 0.375);
		glVertex3f(0., 12., 0.);	
		glTexCoord2f(0.0625, 0.375);
		glVertex3f(0., 12., 4.);
		glTexCoord2f(0.0625, 0.);
		glVertex3f(0., 0., 4.);
		glTexCoord2f(0., 0.);
		glVertex3f(0., 0., 0.);
	glEnd();

	// Left leg
	glBegin(GL_QUADS);
		glNormal3f(0., 0., 1.);//front
		glTexCoord2f(0.0625, 0.375);
		glVertex3f(4., 12., 4.);
		glTexCoord2f(0.125, 0.375);
		glVertex3f(8., 12., 4.);
		glTexCoord2f(0.125, 0.);
		glVertex3f(8., 0., 4.);
		glTexCoord2f(0.0625, 0.);
		glVertex3f(4., 0., 4.);

		glNormal3f(0., 0., -1.);//back
		glTexCoord2f(0.1875, 0.375);
		glVertex3f(8., 12., 0.);
		glTexCoord2f(0.25, 0.375);
		glVertex3f(4., 12., 0.);
		glTexCoord2f(0.25, 0.);
		glVertex3f(4., 0., 0.);
		glTexCoord2f(0.1875, 0.);
		glVertex3f(8., 0., 0.);

		glNormal3f(0., 1., 0.);//top
		glTexCoord2f(0.0625, 0.5);
		glVertex3f(4., 12., 0.);
		glTexCoord2f(0.125, 0.5);
		glVertex3f(8., 12., 0.);
		glTexCoord2f(0.125, 0.375);
		glVertex3f(8., 12., 4.);
		glTexCoord2f(0.0625, 0.375);
		glVertex3f(4., 12., 4.);

		glNormal3f(0., -1., 0.);//bottom
		glTexCoord2f(0.125, 0.5);
		glVertex3f(4., 0., 4.);
		glTexCoord2f(0.1875, 0.5);
		glVertex3f(8., 0., 4.);
		glTexCoord2f(0.1875, 0.375);
		glVertex3f(8., 0., 0.);
		glTexCoord2f(0.125, 0.375);
		glVertex3f(4., 0., 0.);

		glNormal3f(1., 0., 0.);//left
		glTexCoord2f(0.125, 0.375);
		glVertex3f(8., 12., 4.);
		glTexCoord2f(0.1875, 0.375);
		glVertex3f(8., 12., 0.);
		glTexCoord2f(0.1875, 0.);
		glVertex3f(8., 0., 0.);
		glTexCoord2f(0.125, 0.);
		glVertex3f(8., 0., 4.);

		glNormal3f(-1., 0., 0.);//right
		glTexCoord2f(0., 0.375);
		glVertex3f(4., 12., 0.);
		glTexCoord2f(0.0625, 0.375);
		glVertex3f(4., 12., 4.);
		glTexCoord2f(0.0625, 0.);
		glVertex3f(4., 0., 4.);
		glTexCoord2f(0., 0.);
		glVertex3f(4., 0., 0.);
	glEnd();

	// Body
	glBegin(GL_QUADS);
		glNormal3f(0., 0., 1.);//front
		glTexCoord2f(0.3125, 0.375);
		glVertex3f(0., 24., 4.);
		glTexCoord2f(0.4375, 0.375);
		glVertex3f(8., 24., 4.);
		glTexCoord2f(0.4375, 0.);
		glVertex3f(8., 12., 4.);
		glTexCoord2f(0.3125, 0.);
		glVertex3f(0., 12., 4.);

		glNormal3f(0., 0., -1.);//back
		glTexCoord2f(0.5, 0.375);
		glVertex3f(8., 24., 0.);	
		glTexCoord2f(0.625, 0.375);
		glVertex3f(0., 24., 0.);
		glTexCoord2f(0.625, 0.);
		glVertex3f(0., 12., 0.);
		glTexCoord2f(0.5, 0.);
		glVertex3f(8., 12., 0.);

		glNormal3f(0., 1., 0.);//top
		glTexCoord2f(0.3125, 0.5);
		glVertex3f(0., 24., 0.);
		glTexCoord2f(0.4375, 0.5);
		glVertex3f(8., 24., 0.);
		glTexCoord2f(0.4375, 0.375);
		glVertex3f(8., 24., 4.);
		glTexCoord2f(0.3125, 0.375);
		glVertex3f(0., 24., 4.);

		glNormal3f(0., -1., 0.);//bottom
		glTexCoord2f(0.4375, 0.5);
		glVertex3f(0., 12., 4.);	
		glTexCoord2f(0.5625, 0.5);
		glVertex3f(8., 12., 4.);
		glTexCoord2f(0.5625, 0.375);
		glVertex3f(8., 12., 0.);
		glTexCoord2f(0.4375, 0.375);
		glVertex3f(0., 12., 0.);

		glNormal3f(1., 0., 0.);//left
		glTexCoord2f(0.4375, 0.375);
		glVertex3f(8., 24., 4.);
		glTexCoord2f(0.5, 0.375);
		glVertex3f(8., 24., 0.);
		glTexCoord2f(0.5, 0.);
		glVertex3f(8., 12., 0.);
		glTexCoord2f(0.4375, 0.);
		glVertex3f(8., 12., 4.);

		glNormal3f(-1., 0., 0.);//right
		glTexCoord2f(0.25, 0.375);
		glVertex3f(0., 24., 0.);	
		glTexCoord2f(0.3125, 0.375);
		glVertex3f(0., 24., 4.);
		glTexCoord2f(0.3125, 0.);
		glVertex3f(0., 12., 4.);
		glTexCoord2f(0.25, 0.);
		glVertex3f(0., 12., 0.);
	glEnd();
	
	// Right arm
	glBegin(GL_QUADS);
		glNormal3f(0., 0., 1.);//front
		glTexCoord2f(0.6875, 0.);
		glVertex3f(-4., 12., 4.);
		glTexCoord2f(0.75, 0.);
		glVertex3f(0., 12., 4.);
		glTexCoord2f(0.75, 0.375);
		glVertex3f(0., 24., 4.);
		glTexCoord2f(0.6875, 0.375);
		glVertex3f(-4., 24., 4.);

		glNormal3f(0., 0., -1.);//back
		glTexCoord2f(0.875, 0.);
		glVertex3f(-4., 12., 0.);
		glTexCoord2f(0.8125, 0.);
		glVertex3f(0., 12., 0.);
		glTexCoord2f(0.8125, 0.375);
		glVertex3f(0., 24., 0.);
		glTexCoord2f(0.875, 0.375);
		glVertex3f(-4., 24., 0.);

		glNormal3f(0., 1., 0.);//top
		glTexCoord2f(0.6875, 0.5);
		glVertex3f(-4., 24., 0.);
		glTexCoord2f(0.75, 0.5);
		glVertex3f(0., 24., 0.);
		glTexCoord2f(0.75, 0.375);
		glVertex3f(0., 24., 4.);
		glTexCoord2f(0.6875, 0.375);
		glVertex3f(-4., 24., 4.);

		glNormal3f(0., -1., 0.);//bottom
		glTexCoord2f(0.75, 0.375);
		glVertex3f(-4., 12., 0.);
		glTexCoord2f(0.8125, 0.375);
		glVertex3f(0., 12., 0.);
		glTexCoord2f(0.8125, 0.5);
		glVertex3f(0., 12., 4.);
		glTexCoord2f(0.75, 0.5);
		glVertex3f(-4., 12., 4.);

		glNormal3f(1., 0., 0.);//left
		glTexCoord2f(0.8125, 0.);
		glVertex3f(0., 12., 0.);
		glTexCoord2f(0.75, 0.);
		glVertex3f(0., 12., 4.);
		glTexCoord2f(0.75, 0.375);
		glVertex3f(0., 24., 4.);
		glTexCoord2f(0.8125, 0.375);
		glVertex3f(0., 24., 0.);

		glNormal3f(-1., 0., 0.);//right
		glTexCoord2f(0.625, 0.);
		glVertex3f(-4., 12., 0.);
		glTexCoord2f(0.6875, 0.);
		glVertex3f(-4., 12., 4.);
		glTexCoord2f(0.6875, 0.375);
		glVertex3f(-4., 24., 4.);
		glTexCoord2f(0.625, 0.375);
		glVertex3f(-4., 24., 0.);
	glEnd();

	// Left arm
	glBegin(GL_QUADS);
		glNormal3f(0., 0., 1.);//front
		glTexCoord2f(0.6875, 0.);
		glVertex3f(8., 12., 4.);
		glTexCoord2f(0.75, 0.);
		glVertex3f(12., 12., 4.);
		glTexCoord2f(0.75, 0.375);
		glVertex3f(12., 24., 4.);
		glTexCoord2f(0.6875, 0.375);
		glVertex3f(8., 24., 4.);

		glNormal3f(0., 0., -1.);//back
		glTexCoord2f(0.875, 0.);
		glVertex3f(8., 12., 0.);
		glTexCoord2f(0.8125, 0.);
		glVertex3f(12., 12., 0.);
		glTexCoord2f(0.8125, 0.375);
		glVertex3f(12., 24., 0.);
		glTexCoord2f(0.875, 0.375);
		glVertex3f(8., 24., 0.);

		glNormal3f(0., 1., 0.);//top
		glTexCoord2f(0.6875, 0.5);
		glVertex3f(8., 24., 0.);
		glTexCoord2f(0.75, 0.5);
		glVertex3f(12., 24., 0.);
		glTexCoord2f(0.75, 0.375);
		glVertex3f(12., 24., 4.);
		glTexCoord2f(0.6875, 0.375);
		glVertex3f(8., 24., 4.);

		glNormal3f(0., -1., 0.);//bottom
		glTexCoord2f(0.75, 0.375);
		glVertex3f(8., 12., 0.);
		glTexCoord2f(0.8125, 0.375);
		glVertex3f(12., 12., 0.);
		glTexCoord2f(0.8125, 0.5);
		glVertex3f(12., 12., 4.);
		glTexCoord2f(0.75, 0.5);
		glVertex3f(8., 12., 4.);

		glNormal3f(1., 0., 0.);//left
		glTexCoord2f(0.8125, 0.);
		glVertex3f(12., 12., 0.);
		glTexCoord2f(0.75, 0.);
		glVertex3f(12., 12., 4.);
		glTexCoord2f(0.75, 0.375);
		glVertex3f(12., 24., 4.);
		glTexCoord2f(0.8125, 0.375);
		glVertex3f(12., 24., 0.);

		glNormal3f(-1., 0., 0.);//right
		glTexCoord2f(0.625, 0.);
		glVertex3f(8., 12., 0.);
		glTexCoord2f(0.6875, 0.);
		glVertex3f(8., 12., 4.);
		glTexCoord2f(0.6875, 0.375);
		glVertex3f(8., 24., 4.);
		glTexCoord2f(0.625, 0.375);
		glVertex3f(8., 24., 0.);
	glEnd();
	glPopMatrix();
}

void DrawHead()
{
	glPushMatrix();
	glTranslatef(0., -0.2314, -0.2314);
	glRotatef(VerAngle, 1., 0., 0.);
	glTranslatef(-0.2314, -1.3886, -0.1157);
	glScalef(0.0578, 0.0578, 0.0578);
	// Head
	glBegin(GL_QUADS);
		glNormal3f(0., 0., 1.);//front
		glTexCoord2f(0.125, 0.5);
		glVertex3f(0., 24., 6.);
		glTexCoord2f(0.25, 0.5);
		glVertex3f(8., 24., 6.);
		glTexCoord2f(0.25, 0.75);
		glVertex3f(8., 32., 6.);
		glTexCoord2f(0.125, 0.75);
		glVertex3f(0., 32., 6.);

		glNormal3f(0., 0., -1.);//back
		glTexCoord2f(0.375, 0.75);
		glVertex3f(8., 32., -2.);
		glTexCoord2f(0.5, 0.75);
		glVertex3f(0., 32., -2.);
		glTexCoord2f(0.5, 0.5);
		glVertex3f(0., 24., -2.);
		glTexCoord2f(0.375, 0.5);
		glVertex3f(8., 24., -2.);

		glNormal3f(0., 1., 0.);//top
		glTexCoord2f(0.125, 1.);
		glVertex3f(0., 32., -2);
		glTexCoord2f(0.25, 1.);
		glVertex3f(8., 32., -2.);
		glTexCoord2f(0.25, 0.75);
		glVertex3f(8., 32., 6.);
		glTexCoord2f(0.125, 0.75);
		glVertex3f(0., 32., 6.);

		glNormal3f(0., -1., 0.);//bottom
		glTexCoord2f(0.25, 1.);
		glVertex3f(0., 24., 6.);
		glTexCoord2f(0.375, 1.);
		glVertex3f(8., 24., 6.);
		glTexCoord2f(0.375, 0.75);
		glVertex3f(8., 24., -2.);
		glTexCoord2f(0.25, 0.75);
		glVertex3f(0., 24., -2.);

		glNormal3f(-1., 0., 0.);//right
		glTexCoord2f(0., 0.75);
		glVertex3f(0., 32., -2.);
		glTexCoord2f(0.125, 0.75);
		glVertex3f(0., 32., 6.);
		glTexCoord2f(0.125, 0.5);
		glVertex3f(0., 24., 6.);
		glTexCoord2f(0., 0.5);
		glVertex3f(0., 24., -2.);

		glNormal3f(1., 0., 0.);//left
		glTexCoord2f(0.25, 0.75);
		glVertex3f(8., 32., 6.);
		glTexCoord2f(0.375, 0.75);
		glVertex3f(8., 32., -2.);
		glTexCoord2f(0.375, 0.5);
		glVertex3f(8., 24., -2.);
		glTexCoord2f(0.25, 0.5);
		glVertex3f(8., 24., 6.);
	glEnd();
	glPopMatrix();
}

void DrawTorch()
{
	glPushMatrix();

	glScalef(0.05, 0.05, 0.05);
	glTranslatef(0., 10., 0.);
	glColor3f(1., 0., 0.);
	glutSolidSphere(1.5, 50, 50);

	for (int i = 0; i < 50; i++)
	{	
		glBegin(GL_QUADS);
		glColor3f(1., 1., 1.);
		glVertex3f(cos(i * M_PI / 25), 0., sin(i * M_PI / 25));
		glVertex3f(cos((i + 1) * M_PI / 25), 0., sin((i + 1) * M_PI / 25));
		glVertex3f(0.8 * cos((i + 1) * M_PI / 25), -10., 0.8 * sin((i + 1) * M_PI / 25));
		glVertex3f(0.8 * cos(i * M_PI / 25), -10., 0.8 * sin(i * M_PI / 25));
		glEnd();
	}

	glBegin(GL_POLYGON);
	glColor3f(1., 1., 1.);
	for (int i = 0; i < 50; i++)
	{
		glVertex3f(0.8 * cos(i * M_PI / 25), -10., 0.8 * sin(i * M_PI / 25));
	}
	glEnd();
	
	glPopMatrix();
}

// draw the complete scene:
void
Display( )
{
	if( DebugOn != 0 )
	{
		fprintf( stderr, "Display\n" );
	}


	// set which window we want to do the graphics into:

	glutSetWindow( MainWindow );


	// erase the background:

	glDrawBuffer( GL_BACK );	
	glClearColor(0.527, 0.805, 0.977, 0.);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );


	glEnable( GL_DEPTH_TEST );
#ifdef DEMO_DEPTH_BUFFER
	if( DepthBufferOn == 0 )
		glDisable( GL_DEPTH_TEST );
#endif


	// specify shading to be flat:

	glShadeModel( GL_FLAT );


	// set the viewport to a square centered in the window:

	GLsizei vx = glutGet( GLUT_WINDOW_WIDTH );
	GLsizei vy = glutGet( GLUT_WINDOW_HEIGHT );
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = ( vx - v ) / 2;
	GLint yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );


	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluPerspective( 90., 1.,	0.1, 1000. );


	// place the objects into the scene:

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	
	float eyex = Xco + (cos((45. - VerAngle) * M_PI / 180.) - cos(M_PI / 4)) * 0.33 * sin(HorAngle * M_PI / 180.);
	float eyey = Yco + (sin((45. - VerAngle) * M_PI / 180.)- sin(M_PI / 4)) * 0.33;
	float eyez = Zco + (cos((45. - VerAngle) * M_PI / 180.) - cos(M_PI / 4)) * 0.33 * cos(HorAngle * M_PI / 180.);
	if (WhichView == 0)
	{
		gluLookAt(eyex, eyey, eyez, eyex + 5. * sin(HorAngle * M_PI / 180.), eyey - 5. * sin(VerAngle * M_PI / 180.), eyez + 5. * cos(HorAngle * M_PI / 180.), 0., 1., 0.);
	}
	else if (WhichView == 1)
		gluLookAt(Xco - 3 * sin(HorAngle * M_PI / 180.), Yco + 1.5, Zco - 3 * cos(HorAngle * M_PI / 180.), Xco, Yco, Zco, 0., 1., 0.);
	else if (WhichView == 2)
	{
		gluLookAt(0., 2., 3., 0., 0., 0., 0., 1., 0.);
		glRotatef((GLfloat)Xrot, 0., 1., 0.);
		glTranslatef(-Xco, 1.62 - Yco, -Zco);
		if( Scale < MINSCALE )
			Scale = MINSCALE;
		glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );
	}

	// set the fog parameters:

	if( DepthCueOn != 0 )
	{
		glFogi( GL_FOG_MODE, FOGMODE );
		glFogfv( GL_FOG_COLOR, FOGCOLOR );
		glFogf( GL_FOG_DENSITY, FOGDENSITY );
		glFogf( GL_FOG_START, FOGSTART );
		glFogf( GL_FOG_END, FOGEND );
		glEnable( GL_FOG );
	}
	else
	{
		glDisable( GL_FOG );
	}

	// since we are using glScalef( ), be sure normals get unitized:

	glEnable( GL_NORMALIZE );

	// draw the current object:	

	// draw people
	glPushMatrix();

	glTranslatef(Xco, Yco, Zco);
	glRotatef(HorAngle, 0., 2., 0.);
	glEnable(GL_TEXTURE_2D);
	if (WhichSkin == 0)
		glBindTexture(GL_TEXTURE_2D, Tex0);
	else
		glBindTexture(GL_TEXTURE_2D, Tex1);
	DrawBody();
	DrawHead();
	
	glPopMatrix();
	
	// draw lawn
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, Tex2);
	for (int a = -20; a < 20; a++)
	{
		for (int b = -20; b < 20; b++)
		{
			glPushMatrix();
			glTranslatef((float)a, -1., (float)b);
			glCallList(GrassList);
			glPopMatrix();
		}
	}
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);

	// draw torch
	glPushMatrix();
	glTranslatef(5., 0., 5.);
	DrawTorch();
	glPopMatrix();

#ifdef DEMO_Z_FIGHTING
	if( DepthFightingOn != 0 )
	{
		glPushMatrix( );
			glRotatef( 90.,   0., 1., 0. );
			glCallList( BoxList );
		glPopMatrix( );
	}
#endif

	/*
	// draw some gratuitous text that just rotates on top of the scene:

	glDisable( GL_DEPTH_TEST );
	glColor3f( 0., 1., 1. );
	DoRasterString( 0., 1., 0., (char *)"Text That Moves" );
	*/

	// draw some gratuitous text that is fixed on the screen:
	//
	// the projection matrix is reset to define a scene whose
	// world coordinate system goes from 0-100 in each axis
	//
	// this is called "percent units", and is just a convenience
	//
	// the modelview matrix is reset to identity as we don't
	// want to transform these coordinates

	glDisable( GL_DEPTH_TEST );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	gluOrtho2D( 0., 100.,     0., 100. );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	glColor3f( 1., 1., 1. );
	char xco[20], yco[20], zco[20];
	sprintf(xco, "X = %f", Xco);
	sprintf(yco, "Y = %f", Yco);
	sprintf(zco, "Z = %f", Zco);
	DoRasterString(5., 5., 0., (char*) "Qinyuan Song");
	DoRasterString(75., 11., 0., (char*) "Character World Position");
	DoRasterString(80., 8., 0., (char*) xco);
	DoRasterString(80., 5., 0., (char*) yco);
	DoRasterString(80., 2., 0., (char*) zco);


	// swap the double-buffered framebuffers:

	glutSwapBuffers( );


	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush( );
}

void DoViewMenu(int id)
{
	WhichView = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void DoSkinMenu(int id)
{
	WhichSkin = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}

void
DoDebugMenu( int id )
{
	DebugOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthBufferMenu( int id )
{
	DepthBufferOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthFightingMenu( int id )
{
	DepthFightingOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoDepthMenu( int id )
{
	DepthCueOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// main menu callback:

void
DoMainMenu( int id )
{
	switch( id )
	{
		case RESET:
			Reset( );
			break;

		case QUIT:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:
			glutSetWindow( MainWindow );
			glFinish( );
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;

		default:
			fprintf( stderr, "Don't know what to do with Main Menu ID %d\n", id );
	}

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

void
DoShadowsMenu(int id)
{
	ShadowsOn = id;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// use glut to display a string of characters using a raster font:

void
DoRasterString( float x, float y, float z, char *s )
{
	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );

	char c;			// one character to print
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}


// use glut to display a string of characters using a stroke font:

void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
	glPushMatrix( );
		glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
		float sf = ht / ( 119.05f + 33.33f );
		glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
		char c;			// one character to print
		for( ; ( c = *s ) != '\0'; s++ )
		{
			glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
		}
	glPopMatrix( );
}


// return the number of seconds since the start of the program:

float
ElapsedSeconds( )
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet( GLUT_ELAPSED_TIME );

	// convert it to seconds:

	return (float)ms / 1000.f;
}


// initialize the glui window:

void
InitMenus( )
{
	glutSetWindow( MainWindow );

	int skinmenu = glutCreateMenu(DoSkinMenu);
	glutAddMenuEntry("Steve", 0);
	glutAddMenuEntry("Box", 1);

	int viewmenu = glutCreateMenu(DoViewMenu);
	glutAddMenuEntry("First-person Perspective", 0);
	glutAddMenuEntry("Third-person Perspective", 1);
	glutAddMenuEntry("Photo Mode", 2);

	int depthcuemenu = glutCreateMenu( DoDepthMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthbuffermenu = glutCreateMenu( DoDepthBufferMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int depthfightingmenu = glutCreateMenu( DoDepthFightingMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int debugmenu = glutCreateMenu( DoDebugMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

	int shadowsmenu = glutCreateMenu(DoShadowsMenu);
	glutAddMenuEntry("Off", 0);
	glutAddMenuEntry("On", 1);

	int mainmenu = glutCreateMenu( DoMainMenu );
	glutAddSubMenu("Skin", skinmenu);
	glutAddSubMenu("Perspective", viewmenu);


#ifdef DEMO_DEPTH_BUFFER
	glutAddSubMenu(   "Depth Buffer",  depthbuffermenu);
#endif

#ifdef DEMO_Z_FIGHTING
	glutAddSubMenu(   "Depth Fighting",depthfightingmenu);
#endif

	glutAddSubMenu(   "Depth Cue",     depthcuemenu);

#ifdef ENABLE_SHADOWS
	glutAddSubMenu(   "Shadows",       shadowsmenu);
#endif

	glutAddMenuEntry( "Reset",         RESET );
	glutAddSubMenu(   "Debug",         debugmenu);
	glutAddMenuEntry( "Quit",          QUIT );

// attach the pop-up menu to the right mouse button:

	glutAttachMenu( GLUT_RIGHT_BUTTON );
}



// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions

void
InitGraphics( )
{
	// request the display modes:
	// ask for red-green-blue-alpha color, double-buffering, and z-buffering:

	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	// set the initial window configuration:

	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );

	// open the window and set its title:

	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );

	// set the framebuffer clear values:

	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );

	// setup the callback functions:
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on

	glutSetWindow( MainWindow );
	glutDisplayFunc( Display );
	glutReshapeFunc( Resize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc( MousePassMotion );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL);
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( -1, NULL, 0 );
	glutIdleFunc( Animate );

	// init glew (a window must be open to do this):

#ifdef WIN32
	GLenum err = glewInit( );
	if( err != GLEW_OK )
	{
		fprintf( stderr, "glewInit Error\n" );
	}
	else
		fprintf( stderr, "GLEW initialized OK\n" );
	fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

	int width = 64;
	int height = 32;
	unsigned char* Skin1 = BmpToTexture("skin1.bmp", &width, &height);
	unsigned char* Skin2 = BmpToTexture("skin2.bmp", &width, &height);
	unsigned char* Grass = BmpToTexture("grasstex.bmp", &width, &height);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &Tex0);
	glGenTextures(1, &Tex1);
	glGenTextures(1, &Tex2);

	glBindTexture(GL_TEXTURE_2D, Tex0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, Skin1);

	glBindTexture(GL_TEXTURE_2D, Tex1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, Skin2);

	glBindTexture(GL_TEXTURE_2D, Tex2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, Grass);
}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists( )
{
	glutSetWindow(MainWindow);
	//create the grass
	GrassList = glGenLists(1);
	glNewList(GrassList, GL_COMPILE);
		glPushMatrix();
		glScalef(0.9999, 0.9999, 0.9999);
		glBegin(GL_QUADS);
		glNormal3f(0., 0., 1.);//front
		glTexCoord2f(0.25, 0.);
		glVertex3f(0., 0., 1.);
		glTexCoord2f(0.5, 0.);
		glVertex3f(1., 0., 1.);
		glTexCoord2f(0.5, 0.5);
		glVertex3f(1., 1., 1.);
		glTexCoord2f(0.25, 0.5);
		glVertex3f(0., 1., 1.);

		glNormal3f(0., 0., -1.);//back
		glTexCoord2f(1., 0.);
		glVertex3f(0., 0., 0.);
		glTexCoord2f(0.75, 0.);
		glVertex3f(1., 0., 0.);
		glTexCoord2f(0.75, 0.5);
		glVertex3f(1., 1., 0.);
		glTexCoord2f(1., 0.5);
		glVertex3f(0., 1., 0.);

		glNormal3f(0., 1., 0.);//top
		glTexCoord2f(0.25, 0.5);
		glVertex3f(0., 1., 1);
		glTexCoord2f(0.5, 0.5);
		glVertex3f(1., 1., 1.);
		glTexCoord2f(0.5, 1.);
		glVertex3f(1., 1., 0.);
		glTexCoord2f(0.25, 1.);
		glVertex3f(0., 1., 0.);

		glNormal3f(0., -1., 0.);//bottom
		glTexCoord2f(0.5, 1.);
		glVertex3f(0., 0., 1.);
		glTexCoord2f(0.75, 1.);
		glVertex3f(1., 0., 1.);
		glTexCoord2f(0.75, 0.5);
		glVertex3f(1., 0., 0.);
		glTexCoord2f(0.5, 0.5);
		glVertex3f(0., 0., 0.);

		glNormal3f(-1., 0., 0.);//right
		glTexCoord2f(0., 0.5);
		glVertex3f(0., 1., 0.);
		glTexCoord2f(0.25, 0.5);
		glVertex3f(0., 1., 1.);
		glTexCoord2f(0.25, 0.);
		glVertex3f(0., 0., 1.);
		glTexCoord2f(0., 0.);
		glVertex3f(0., 0., 0.);

		glNormal3f(1., 0., 0.);//left
		glTexCoord2f(0.5, 0.5);
		glVertex3f(1., 1., 1.);
		glTexCoord2f(0.75, 0.5);
		glVertex3f(1., 1., 0.);
		glTexCoord2f(0.75, 0.);
		glVertex3f(1., 0., 0.);
		glTexCoord2f(0.5, 0.);
		glVertex3f(1., 0., 1.);
		glEnd();
		glPopMatrix();
	glEndList();
}


// the keyboard callback:

void
Keyboard( unsigned char c, int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
	{
		case 'w':
		case 'W':
			if (WhichView != 2)
			{		
				Xco = Xco + 0.08 * sin(HorAngle * M_PI / 180.);
				Zco = Zco + 0.08 * cos(HorAngle * M_PI / 180.);
			}
			break;

		case 's':
		case 'S':
			if (WhichView != 2)
			{
				Xco = Xco - 0.08 * sin(HorAngle * M_PI / 180.);
				Zco = Zco - 0.08 * cos(HorAngle * M_PI / 180.);
			}
			break;

		case 'a':
		case 'A':
			if (WhichView != 2)
			{
				Xco = Xco + 0.08 * cos(HorAngle * M_PI / 180.);
				Zco = Zco - 0.08 * sin(HorAngle * M_PI / 180.);
			}	
			break;

		case 'd':
		case 'D':
			if (WhichView != 2)
			{
				Xco = Xco - 0.08 * cos(HorAngle * M_PI / 180.);
				Zco = Zco + 0.08 * sin(HorAngle * M_PI / 180.);
			}
			break;

		case ' ':
			if (WhichView != 2)
			{
				Yco = Yco + 0.05;
			}
			break;

		case 'x':
		case 'X':
			if (WhichView != 2)
			{
				Yco = Yco - 0.05;
				if (Yco <= 1.62)
					Yco = 1.62;
			}
			break;

		case 'q':
		case 'Q':
			HorAngle = HorAngle + 1.;
			break;

		case 'e':
		case 'E':
			HorAngle = HorAngle - 1.;
			break;

		case '0':
			WhichView = 0;
			break;

		case '1':
			WhichView = 1;
			break;

		case '2':
			WhichView = 2;
			break;
	
		case ESCAPE:
			DoMainMenu( QUIT );	// will not return here
			break;				// happy compiler
		
		default:
			fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}

	// force a call to Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

// called when the mouse button transitions down or up:

void
MouseButton( int button, int state, int x, int y )
{
	
	int b = 0;			// LEFT, MIDDLE, or RIGHT

	if( DebugOn != 0 )
		fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );

	
	// get the proper button bit mask:

	switch( button )
	{
		case GLUT_LEFT_BUTTON:
			b = LEFT;		break;

		case GLUT_MIDDLE_BUTTON:
			b = MIDDLE;		break;

		case GLUT_RIGHT_BUTTON:
			b = RIGHT;		break;

		case SCROLL_WHEEL_UP:
			Scale += SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;

		case SCROLL_WHEEL_DOWN:
			Scale -= SCLFACT * SCROLL_WHEEL_CLICK_FACTOR;
			// keep object from turning inside-out or disappearing:
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;


		default:
			b = 0;
			fprintf( stderr, "Unknown mouse button: %d\n", button );
	}

	// button down sets the bit, up clears the bit:

	if( state == GLUT_DOWN )
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}

	glutSetWindow(MainWindow);
	glutPostRedisplay();
	
}


// called when the mouse moves while a button is down:

void
MouseMotion( int x, int y )
{
	int dx = x - Xmouse;		// change in mouse coords
	int dy = y - Ymouse;
	
	if (WhichView == 1)
	{
		if ((ActiveButton & LEFT) != 0)
		{
			HorAngle += (ANGTP * dx);
			VerAngle += (- ANGTP * dy);
			if (HorAngle < 0.)
			{
				HorAngle = HorAngle + 360.;
			}
			else if (HorAngle > 360.)
			{
				HorAngle = HorAngle - 360.;
			}

			if (VerAngle < -45.)
			{
				VerAngle = -45.;
			}
			else if (VerAngle > 45)
			{
				VerAngle = 45.;
			}
		}
	}

	if (WhichView == 2)
	{
		if ((ActiveButton & LEFT) != 0)
		{
			Xrot += (ANGFR * dx);
		}

		if ((ActiveButton & MIDDLE) != 0)
		{
			Scale += SCLFACT * (float)(dx - dy);

			// keep object from turning inside-out or disappearing:

			if (Scale < MINSCALE)
				Scale = MINSCALE;
		}
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

void MousePassMotion(int x, int y)
{
	
	if (WhichView == 0)
	{
		int dx = x - INIT_WINDOW_SIZE / 2;		// change in mouse coords
		int dy = y - INIT_WINDOW_SIZE / 2;

		HorAngle += (- ANGFP * dx);
		VerAngle += ( ANGFP * dy);
		if (HorAngle < 0.)
		{
			HorAngle = HorAngle + 360.;
		}
		else if (HorAngle > 360.)
		{
			HorAngle = HorAngle - 360.;
		}

		if (VerAngle < -45.)
		{
			VerAngle = -45.;
		}
		else if (VerAngle > 45)
		{
			VerAngle = 45.;
		}
		
		glutWarpPointer(INIT_WINDOW_SIZE / 2, INIT_WINDOW_SIZE / 2);
	}
	
	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset( )
{
	ActiveButton = 0;
	DebugOn = 0;
	DepthBufferOn = 1;
	DepthFightingOn = 0;
	DepthCueOn = 0;
	Scale  = 1.0;
	ShadowsOn = 0;
	WhichSkin = 0;
	Xco = Zco = 0.;
	Yco = 1.62;
	WhichView = 1;
	VerAngle = 0.;
	HorAngle = 0.;
	Xrot = 0.;
}


// called when user resizes the window:

void
Resize( int width, int height )
{
	if( DebugOn != 0 )
		fprintf( stderr, "ReSize: %d, %d\n", width, height );

	// don't really need to do anything since window size is
	// checked each time in Display( ):

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// handle a change to the window's visibility:

void
Visibility ( int state )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );

	if( state == GLUT_VISIBLE )
	{
		glutSetWindow( MainWindow );
		glutPostRedisplay( );
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}



///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[ ] = {
		0.f, 1.f, 0.f, 1.f
	      };

static float xy[ ] = {
		-.5f, .5f, .5f, -.5f
	      };

static int xorder[ ] = {
		1, 2, -3, 4
		};

static float yx[ ] = {
		0.f, 0.f, -.5f, .5f
	      };

static float yy[ ] = {
		0.f, .6f, 1.f, 1.f
	      };

static int yorder[ ] = {
		1, 2, 3, -2, 4
		};

static float zx[ ] = {
		1.f, 0.f, 1.f, 0.f, .25f, .75f
	      };

static float zy[ ] = {
		.5f, .5f, -.5f, -.5f, 0.f, 0.f
	      };

static int zorder[ ] = {
		1, 2, 3, 4, -5, 6
		};

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)

void
Axes( float length )
{
	glBegin( GL_LINE_STRIP );
		glVertex3f( length, 0., 0. );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., length, 0. );
	glEnd( );
	glBegin( GL_LINE_STRIP );
		glVertex3f( 0., 0., 0. );
		glVertex3f( 0., 0., length );
	glEnd( );

	float fact = LENFRAC * length;
	float base = BASEFRAC * length;

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 4; i++ )
		{
			int j = xorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 5; i++ )
		{
			int j = yorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
		}
	glEnd( );

	glBegin( GL_LINE_STRIP );
		for( int i = 0; i < 6; i++ )
		{
			int j = zorder[i];
			if( j < 0 )
			{
				
				glEnd( );
				glBegin( GL_LINE_STRIP );
				j = -j;
			}
			j--;
			glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
		}
	glEnd( );

}

struct bmfh
{
	short bfType;
	int bfSize;
	short bfReserved1;
	short bfReserved2;
	int bfOffBits;
} FileHeader;

struct bmih
{
	int biSize;
	int biWidth;
	int biHeight;
	short biPlanes;
	short biBitCount;
	int biCompression;
	int biSizeImage;
	int biXPelsPerMeter;
	int biYPelsPerMeter;
	int biClrUsed;
	int biClrImportant;
} InfoHeader;

const int birgb = { 0 };

// read a BMP file into a Texture:

unsigned char *
BmpToTexture( char *filename, int *width, int *height )
{
	FILE *fp = fopen( filename, "rb" );
	if( fp == NULL )
	{
		fprintf( stderr, "Cannot open Bmp file '%s'\n", filename );
		return NULL;
	}

	FileHeader.bfType = ReadShort( fp );


	// if bfType is not 0x4d42, the file is not a bmp:

	if( FileHeader.bfType != 0x4d42 )
	{
		fprintf( stderr, "File '%s' is the wrong type of file: 0x%0x\n", filename, FileHeader.bfType );
		fclose( fp );
		return NULL;
	}

	FileHeader.bfSize = ReadInt( fp );
	FileHeader.bfReserved1 = ReadShort( fp );
	FileHeader.bfReserved2 = ReadShort( fp );
	FileHeader.bfOffBits = ReadInt( fp );

	InfoHeader.biSize = ReadInt( fp );
	InfoHeader.biWidth = ReadInt( fp );
	InfoHeader.biHeight = ReadInt( fp );

	int nums = InfoHeader.biWidth;
	int numt = InfoHeader.biHeight;

	InfoHeader.biPlanes = ReadShort( fp );
	InfoHeader.biBitCount = ReadShort( fp );
	InfoHeader.biCompression = ReadInt( fp );
	InfoHeader.biSizeImage = ReadInt( fp );
	InfoHeader.biXPelsPerMeter = ReadInt( fp );
	InfoHeader.biYPelsPerMeter = ReadInt( fp );
	InfoHeader.biClrUsed = ReadInt( fp );
	InfoHeader.biClrImportant = ReadInt( fp );

	fprintf( stderr, "Image size in file '%s' is: %d x %d\n", filename, nums, numt );

	unsigned char * texture = new unsigned char[ 3 * nums * numt ];
	if( texture == NULL )
	{
		fprintf( stderr, "Cannot allocate the texture array!\b" );
		return NULL;
	}

	// extra padding bytes:

	int numextra =  4*(( (3*InfoHeader.biWidth)+3)/4) - 3*InfoHeader.biWidth;

	// we do not support compression:

	if( InfoHeader.biCompression != birgb )
	{
		fprintf( stderr, "Image file '%s' has the wrong type of image compression: %d\n", filename, InfoHeader.biCompression );
		fclose( fp );
		return NULL;
	}

	rewind( fp );
	fseek( fp, 14+40, SEEK_SET );

	if( InfoHeader.biBitCount == 24 )
	{
		unsigned char *tp = texture;
		for( int t = 0; t < numt; t++ )
		{
			for( int s = 0; s < nums; s++, tp += 3 )
			{
				*(tp+2) = fgetc( fp );		// b
				*(tp+1) = fgetc( fp );		// g
				*(tp+0) = fgetc( fp );		// r
			}

			for( int e = 0; e < numextra; e++ )
			{
				fgetc( fp );
			}
		}
	}

	fclose( fp );

	*width = nums;
	*height = numt;
	return texture;
}

int
ReadInt( FILE *fp )
{
	unsigned char b3, b2, b1, b0;
	b0 = fgetc( fp );
	b1 = fgetc( fp );
	b2 = fgetc( fp );
	b3 = fgetc( fp );
	return ( b3 << 24 )  |  ( b2 << 16 )  |  ( b1 << 8 )  |  b0;
}

short
ReadShort( FILE *fp )
{
	unsigned char b1, b0;
	b0 = fgetc( fp );
	b1 = fgetc( fp );
	return ( b1 << 8 )  |  b0;
}


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//		glColor3fv( rgb );

void
HsvRgb( float hsv[3], float rgb[3] )
{
	// guarantee valid input:

	float h = hsv[0] / 60.f;
	while( h >= 6. )	h -= 6.;
	while( h <  0. ) 	h += 6.;

	float s = hsv[1];
	if( s < 0. )
		s = 0.;
	if( s > 1. )
		s = 1.;

	float v = hsv[2];
	if( v < 0. )
		v = 0.;
	if( v > 1. )
		v = 1.;

	// if sat==0, then is a gray:

	if( s == 0.0 )
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}

	// get an rgb from the hue itself:
	
	float i = (float)floor( h );
	float f = h - i;
	float p = v * ( 1.f - s );
	float q = v * ( 1.f - s*f );
	float t = v * ( 1.f - ( s * (1.f-f) ) );

	float r=0., g=0., b=0.;			// red, green, blue
	switch( (int) i )
	{
		case 0:
			r = v;	g = t;	b = p;
			break;
	
		case 1:
			r = q;	g = v;	b = p;
			break;
	
		case 2:
			r = p;	g = v;	b = t;
			break;
	
		case 3:
			r = p;	g = q;	b = v;
			break;
	
		case 4:
			r = t;	g = p;	b = v;
			break;
	
		case 5:
			r = v;	g = p;	b = q;
			break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];
	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];
	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}

float
Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

float
Unit(float vin[3], float vout[3])
{
	float dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		vout[0] = vin[0] / dist;
		vout[1] = vin[1] / dist;
		vout[2] = vin[2] / dist;
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}
	return dist;
}
