#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifndef F_PI
#define F_PI		((float)(M_PI))
#define F_2_PI		((float)(2.f*F_PI))
#define F_PI_2		((float)(F_PI/2.f))
#endif


#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif

#include "glew.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include "glut.h"



// Structure for storing planet information
struct planet
{
	char*					name;
	char*					file;
	float                   scale; // Size scale
	float                   AU;
	float					Angle;
	float					RotationsPerOrbit;
	float					AxialAngle;
	GLuint					texObject;
	GLuint					DLObject;
	GLuint					CircleDLObject;
};

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
//	Author:			Dalton Shults

// title of these windows:

const char *WINDOWTITLE = "OpenGL / GLUT Sample -- Dalton Shults";
const char *GLUITITLE   = "User Interface Window";

// what the glui package defines as true and false:

const int GLUITRUE  = true;
const int GLUIFALSE = false;

// the escape key:

const int ESCAPE = 0x1b;

// initial window size:

const int INIT_WINDOW_SIZE = 600;

// size of the 3d box to be drawn:

const float BOXSIZE = 2.f;

// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = 1.f;
const float SCLFACT = 0.005f;

// minimum allowable scale factor:

const float MINSCALE = 0.05f;

// scroll wheel button values:

const int SCROLL_WHEEL_UP   = 3;
const int SCROLL_WHEEL_DOWN = 4;

// equivalent mouse movement when we click the scroll wheel:

const float SCROLL_WHEEL_CLICK_FACTOR = 1.f;

// active mouse buttons (or them together):

const int LEFT   = 4;
const int MIDDLE = 2;
const int RIGHT  = 1;

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

// line width for the axes:

const GLfloat AXES_WIDTH   = 3.;

// the color numbers:
// this order must match the radio button order, which must match the order of the color names,
// 	which must match the order of the color RGB values

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA
};

char * ColorNames[ ] =
{
	(char *)"Red",
	(char*)"Yellow",
	(char*)"Green",
	(char*)"Cyan",
	(char*)"Blue",
	(char*)"Magenta"
};

// the color definitions:
// this order must match the menu order

const GLfloat Colors[ ][3] = 
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
};

// fog parameters:

const GLfloat	FOGCOLOR[4]				= { .0f, .0f, .0f, 1.f };
const GLenum	FOGMODE					= GL_LINEAR;
const GLfloat	FOGDENSITY				= 0.30f;
const GLfloat	FOGSTART				= 1.5f;
const GLfloat	FOGEND					= 4.f;
const float		PLANET_SIZE_SCALE		= 2.f;
const float		SUN_SIZE_SCALE			= .15f;
const float		ORBIT_SCALE				= 100.f;
float			ORBIT_SPEED_RATIO		= 1.f;     //0.00002;
const float		STARS_SCALE				= 7500.f;

//float SunDiameter = SUNS_SIZE * SUN_SIZE_SCALE;

// for lighting:

const float	WHITE[ ] = { 1.,1.,1.,1. };

// for animation:

const int MS_PER_CYCLE = 10000;		// 10000 milliseconds = 10 seconds


// non-constant global variables:

int		ActiveButton;			// current button that is down
GLuint	AxesList;				// list to hold the axes
int		AxesOn;					// != 0 means to draw the axes
GLuint	BoxList;				// object display list
int		DebugOn;				// != 0 means to print debugging info
int		DepthCueOn;				// != 0 means to use intensity depth cueing
int		DepthBufferOn;			// != 0 means to use the z-buffer
int		DepthFightingOn;		// != 0 means to force the creation of z-fighting
int		MainWindow;				// window id for main graphics window
int		NowColor;				// index into Colors[ ]
int		NowProjection;			// ORTHO or PERSP
int		Lines;
float	Scale;					// scaling factor
int		ShadowsOn;				// != 0 means to turn shadows on
float	Time;					// used for animation, this has a value between 0. and 1.
int		Xmouse, Ymouse;			// mouse values
float	Xrot, Yrot;				// rotation angles in degrees

// My variables

GLenum LightType;

// List Vars
GLuint SunDL;
GLuint LightSphereDL;
GLuint SphereDL;
GLuint MarsDL;
GLuint EarthDL;
GLuint VenusDL;
GLuint JupiterDL;
GLuint SaturnDL;
GLuint UranusDL;
GLuint NeptuneDL;
GLuint MercuryDL;

GLuint SolarDL;

// Tex Vars
GLuint SunTex;
GLuint MarsTex;
GLuint EarthTex;
GLuint VenusTex;
GLuint JupiterTex;
GLuint SaturnTex;
GLuint UranusTex;
GLuint NeptuneTex;
GLuint MercuryTex;

GLuint SolarTex;

// Mode variables
int		TextureMode;
int		LightingMode;
float	LightRadius;
bool	Frozen;
bool	ShowSphere;
bool	FollowEarth;

bool FollowVenus;
bool FollowMercury;
bool FollowMars;
bool FollowJupiter;
bool FollowSaturn;
bool FollowUranus;
bool FollowNeptune;

// Orbit periods
float mercuryOP; // = 0.244;
float venusOP; // = 0.611;
float earthOP; // = 1.;
float marsOP; // = 1.874;
float jupiterOP; // = 11.858;
float saturnOP; // = 29.652;
float uranusOP; // = 84.262;
float neptuneOP; // = 164.728;

// Rotation Angles
float mercuryAngle = 0.f;
float venusAngle = 0.f;
float earthAngle = 0.f;
float marsAngle = 0.f;
float jupiterAngle = 0.f;
float saturnAngle = 0.f;
float uranusAngle = 0.f;
float neptuneAngle = 0.f;

// Frames
float LastFrameTime = 0.0f;
float TimeDelta = 0.0f;

// Circle List vars
GLuint	earthCircleDL;
GLuint	mercuryCircleDL;
GLuint	venusCircleDL;
GLuint	marsCircleDL;
GLuint	jupiterCircleDL;
GLuint	saturnCircleDL;
GLuint	uranusCircleDL;
GLuint neptuneCircleDL;


// function prototypes:

void	Animate( );
void	Display( );
void	DoAxesMenu( int );
void	DoColorMenu( int );
void	DoDepthBufferMenu( int );
void	DoDepthFightingMenu( int );
void	DoDepthMenu( int );
void	DoDebugMenu( int );
void	DoMainMenu( int );
void	DoProjectMenu( int );
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( );
void	InitGraphics( );
void	InitLists( );
void	InitMenus( );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( );
void	Resize( int, int );
void	Visibility( int );

void	Axes( float );
void	HsvRgb( float[3], float [3] );
void	Cross(float[3], float[3], float[3]);
float	Dot(float [3], float [3]);
float	Unit(float [3], float [3]);
float	Unit(float [3]);


struct planet Entities[] =
{ //      name              file                       scale     au        angle    rot/orb     AxialAngle      texObject       DLObject
		{ "Sun",		"8k_sun.bmp",				109.f,		0.f,		0.0,	13.51f,		7.25f,			SunTex,			SunDL,		NULL},
		{ "Venus",      "8k_venus_surface.bmp",		0.95f,		0.72f,		0.0,	-0.93f,		2.64f,			VenusTex,		VenusDL,	venusCircleDL	},
		{ "Earth",      "8k_earth_daymap.bmp",		1.00f,		1.f,		0.0,	365.25f,	23.44f,			EarthTex,		EarthDL,	earthCircleDL	},
		{ "Mars",       "8k_mars.bmp",				0.53f,		1.52f,		0.0,	667.f,		25.19f,			MarsTex,		MarsDL,		marsCircleDL	},
		{ "Jupiter",    "8k_jupiter.bmp",			11.21f,		5.20f,		0.0,	10563.f,	3.13f,			JupiterTex,		JupiterDL,	jupiterCircleDL	},
		{ "Saturn",     "8k_saturn.bmp",			9.45f,		9.58f,		0.0,	23909.f,	26.73f,			SaturnTex,		SaturnDL,	saturnCircleDL	},
		{ "Uranus",     "2k_uranus.bmp",			4.01f,		19.22f,		0.0,	42621.f,	82.23f,			UranusTex,		UranusDL,	uranusCircleDL	},
		{ "Neptune",    "2k_neptune.bmp",			3.88f,		30.05f,		0.0,	89824.f,	28.32f,			NeptuneTex,		NeptuneDL,	neptuneCircleDL	},
		{ "Mercury",	"8k_mercury.bmp",			0.387f,		0.39f,		0.0,	1.50f,		0.03f,			MercuryTex,		MercuryDL,	mercuryCircleDL	}
};

const int NUMPLANETS = sizeof(Entities) / sizeof(struct planet);

// utility to create an array from 3 separate values:

float *
Array3( float a, float b, float c )
{
	static float array[4];

	array[0] = a;
	array[1] = b;
	array[2] = c;
	array[3] = 1.;
	return array;
}

// utility to create an array from a multiplier and an array:

float *
MulArray3( float factor, float array0[ ] )
{
	static float array[4];

	array[0] = factor * array0[0];
	array[1] = factor * array0[1];
	array[2] = factor * array0[2];
	array[3] = 1.;
	return array;
}


float *
MulArray3(float factor, float a, float b, float c )
{
	static float array[4];

	float* abc = Array3(a, b, c);
	array[0] = factor * abc[0];
	array[1] = factor * abc[1];
	array[2] = factor * abc[2];
	array[3] = 1.;
	return array;
}

// these are here for when you need them -- just uncomment the ones you need:

#include "setmaterial.cpp"
#include "setlight.cpp"
#include "osusphere.cpp"
//#include "osucone.cpp"
//#include "osutorus.cpp"
#include "bmptotexture.cpp"
//#include "loadobjfile.cpp"
//#include "keytime.cpp"
//#include "glslprogram.cpp"


void
initializeOP()
{
	venusOP = sqrt(pow(Entities[1].AU, 3));
	earthOP = sqrt(pow(Entities[2].AU, 3));
	marsOP = sqrt(pow(Entities[3].AU, 3));
	jupiterOP = sqrt(pow(Entities[4].AU, 3));
	saturnOP = sqrt(pow(Entities[5].AU, 3));
	uranusOP = sqrt(pow(Entities[6].AU, 3));
	neptuneOP = sqrt(pow(Entities[7].AU, 3));
	mercuryOP = sqrt(pow(Entities[8].AU, 3));
}
// main program:

int
main( int argc, char *argv[ ] )
{
	// turn on the glut package:
	// (do this before checking argc and argv since glutInit might
	// pull some command line arguments out)

	glutInit( &argc, argv );

	LastFrameTime = (float)glutGet(GLUT_ELAPSED_TIME);

	// setup all the graphics stuff:

	InitGraphics( );

	// create the display lists that **will not change**:

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

	// glutMainLoop( ) never actually returns
	// the following line is here to make the compiler happy:

	return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display( ) from here -- let glutPostRedisplay( ) do it

void
Animate( )
{
	// put animation stuff in here -- change some global variables for Display( ) to find:

	int ms = glutGet(GLUT_ELAPSED_TIME);
	//ms %= MS_PER_CYCLE;							// makes the value of ms between 0 and MS_PER_CYCLE-1
	Time = (float)ms / (float)MS_PER_CYCLE;		// makes the value of Time between 0. and slightly less than 1.

	// MY CODE
	earthAngle = fmod(Time / earthOP * ORBIT_SPEED_RATIO, 1.0f) * -F_2_PI;
	mercuryAngle = fmod(Time / mercuryOP * ORBIT_SPEED_RATIO, 1.0f) * -F_2_PI;
	venusAngle = fmod(Time / venusOP * ORBIT_SPEED_RATIO, 1.0f) * -F_2_PI;
	marsAngle = fmod(Time / marsOP * ORBIT_SPEED_RATIO, 1.0f) * -F_2_PI;
	jupiterAngle = fmod(Time / jupiterOP * ORBIT_SPEED_RATIO, 1.0f) * -F_2_PI;
	saturnAngle = fmod(Time / saturnOP * ORBIT_SPEED_RATIO, 1.0f) * -F_2_PI;
	uranusAngle = fmod(Time / uranusOP * ORBIT_SPEED_RATIO, 1.0f) * -F_2_PI;
	neptuneAngle = fmod(Time / neptuneOP * ORBIT_SPEED_RATIO, 1.0f) * -F_2_PI;

	Entities[1].Angle = venusAngle;
	Entities[2].Angle = earthAngle;
	Entities[3].Angle = marsAngle;
	Entities[4].Angle = jupiterAngle;
	Entities[5].Angle = saturnAngle;
	Entities[6].Angle = uranusAngle;
	Entities[7].Angle = neptuneAngle;
	Entities[8].Angle = mercuryAngle;

	// for example, if you wanted to spin an object in Display( ), you might call: glRotatef( 360.f*Time,   0., 1., 0. );

	// force a call to Display( ) next time it is convenient:

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// draw the complete scene:

void
Display( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting Display.\n");

	// set which window we want to do the graphics into:
	glutSetWindow( MainWindow );

	// erase the background:
	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable( GL_DEPTH_TEST );
#ifdef DEMO_DEPTH_BUFFER
	if( DepthBufferOn == 0 )
		glDisable( GL_DEPTH_TEST );
#endif


	// specify shading to be flat:

	glShadeModel( GL_FLAT );

	// set the viewport to be a square centered in the window:

	GLsizei vx = glutGet( GLUT_WINDOW_WIDTH );
	GLsizei vy = glutGet( GLUT_WINDOW_HEIGHT );
	GLsizei v = vx < vy ? vx : vy;			// minimum dimension
	GLint xl = ( vx - v ) / 2;
	GLint yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );


	// set the viewing volume:
	// remember that the Z clipping  values are given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D( ) IF YOU ARE DOING 2D !

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	if( NowProjection == ORTHO )
		glOrtho( -3000.f, 3000.f,     -3000.f, 3000.f,     .1f, 1000000.f );
	else
		gluPerspective( 70.f, 1.f,	.1f, 1000000.f );

	// place the objects into the scene:

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );

	// set the eye position, look-at position, and up-vector:
	if (FollowEarth)
	{
		float e_x = (Entities[2].AU * ORBIT_SCALE) * cos(Entities[2].Angle);
		float e_z = (Entities[2].AU * ORBIT_SCALE) * sin(Entities[2].Angle);
		Xrot, Yrot = 0;
		gluLookAt(e_x + 15, 15.f, e_z + 15.f, e_x, 0.f, e_z, 0.f, 1.f, 0.f);
	}
	else if (FollowVenus)
	{
		float v_x = (Entities[1].AU * ORBIT_SCALE) * cos(Entities[1].Angle);
		float v_z = (Entities[1].AU * ORBIT_SCALE) * sin(Entities[1].Angle);
		Xrot, Yrot = 0;
		gluLookAt(v_x + 15, 15.f, v_z + 15.f, v_x, 0.f, v_z, 0.f, 1.f, 0.f);
	}
	else if (FollowMars)
	{
		float m_x = (Entities[3].AU * ORBIT_SCALE) * cos(Entities[3].Angle);
		float m_z = (Entities[3].AU * ORBIT_SCALE) * sin(Entities[3].Angle);
		Xrot, Yrot = 0;
		gluLookAt(m_x + 25, 15.f, m_z + 15.f, m_x, 0.f, m_z, 0.f, 1.f, 0.f);
	}
	else if (FollowJupiter)
	{
		float j_x = (Entities[4].AU * ORBIT_SCALE) * cos(Entities[4].Angle);
		float j_z = (Entities[4].AU * ORBIT_SCALE) * sin(Entities[4].Angle);
		Xrot, Yrot = 0;
		gluLookAt(j_x + 75.f, 25.f, j_z + 100.f, j_x, 0.f, j_z, 0.f, 1.f, 0.f);
	}
	else if (FollowSaturn)
	{
		float s_x = (Entities[5].AU * ORBIT_SCALE) * cos(Entities[5].Angle);
		float s_z = (Entities[5].AU * ORBIT_SCALE) * sin(Entities[5].Angle);
		Xrot, Yrot = 0;
		gluLookAt(s_x + 75.f, 25.f, s_z + 100.f, s_x, 0.f, s_z, 0.f, 1.f, 0.f);
	}
	else if (FollowUranus)
	{
		float u_x = (Entities[6].AU * ORBIT_SCALE) * cos(Entities[6].Angle);
		float u_z = (Entities[6].AU * ORBIT_SCALE) * sin(Entities[6].Angle);
		Xrot, Yrot = 0;
		gluLookAt(u_x + 75.f, 25.f, u_z + 100.f, u_x, 0.f, u_z, 0.f, 1.f, 0.f);
	}
	else if (FollowNeptune)
	{
		float n_x = (Entities[7].AU * ORBIT_SCALE) * cos(Entities[7].Angle);
		float n_z = (Entities[7].AU * ORBIT_SCALE) * sin(Entities[7].Angle);
		Xrot, Yrot = 0;
		gluLookAt(n_x + 75.f, 25.f, n_z + 100.f, n_x, 0.f, n_z, 0.f, 1.f, 0.f);
	}
	else if (FollowMercury)
	{
		float m_x = (Entities[8].AU * ORBIT_SCALE) * cos(Entities[8].Angle);
		float m_z = (Entities[8].AU * ORBIT_SCALE) * sin(Entities[8].Angle);
		Xrot, Yrot = 0;
		gluLookAt(m_x + 7.5f, 25.f, m_z + 15.f, m_x, 0.f, m_z, 0.f, 1.f, 0.f);
	}
	else
	{
		gluLookAt(250.f, 1500.f, 5250.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f);
	}
	// rotate the scene:

	glRotatef( (GLfloat)Yrot, 0.f, 1.f, 0.f );
	glRotatef( (GLfloat)Xrot, 1.f, 0.f, 0.f );

	// uniformly scale the scene:

	if( Scale < MINSCALE )
		Scale = MINSCALE;
	glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );

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

	// possibly draw the axes:

	if( AxesOn != 0 )
	{
		glDisable(GL_LIGHTING);
		glColor3fv(&Colors[NowColor][0]);
		glCallList(AxesList);
		glEnable(GL_LIGHTING);
	}

	// since we are using glScalef( ), be sure the normals get unitized:


	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glCallList(Entities[0].DLObject);
	glDisable(GL_TEXTURE_2D);
	glEnable( GL_NORMALIZE );

	glEnable(GL_LIGHTING);
	SetPointLight(GL_LIGHT1, 0, 0, 0, 1.0, 1.0, 1.0);
	glDisable(GL_LIGHTING);
	
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glCallList(SolarDL);
	glEnable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	for (int i = 1; i < NUMPLANETS; i++)
	{	

		float x = (Entities[i].AU * ORBIT_SCALE) * cos(Entities[i].Angle);
		float z = (Entities[i].AU * ORBIT_SCALE) * sin(Entities[i].Angle);

		float Spin = -(Entities[i].RotationsPerOrbit * 360 * Entities[i].Angle * (180.f / F_PI));

		glEnable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, LightType);
			glPushMatrix();
				glTranslatef(x, 0.f, z);
				glRotatef(Entities[i].AxialAngle, 1.f, 0.f, 0.f);
				glRotatef(Spin, 0.f, 1.f, 0.f);
				glCallList(Entities[i].DLObject);
			glPopMatrix();
	    glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glEnable(GL_NORMALIZE);
	}
	if (Lines == 1)
	{
		for (int i = 1; i < NUMPLANETS; i++)
		{
			glPushMatrix();
			glColor3f(0.16f, 0.02f, 0.448f);
			glCallList(Entities[i].CircleDLObject);
			glPopMatrix();
		}
	}
	// draw the box object by calling up its display list:
	

	//glCallList( SunDL );

#ifdef DEMO_Z_FIGHTING
	if( DepthFightingOn != 0 )
	{
		glPushMatrix( );
			glRotatef( 90.f,   0.f, 1.f, 0.f );
			glCallList( BoxList );
		glPopMatrix( );
	}
#endif


	// draw some gratuitous text that just rotates on top of the scene:
	// i commented out the actual text-drawing calls -- put them back in if you have a use for them
	// a good use for thefirst one might be to have your name on the screen
	// a good use for the second one might be to have vertex numbers on the screen alongside each vertex

	glDisable( GL_DEPTH_TEST );
	glColor3f( 0.f, 1.f, 1.f );
	//DoRasterString( 0.f, 1.f, 0.f, (char *)"Text That Moves" );


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
	gluOrtho2D( 0.f, 100.f,     0.f, 100.f );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity( );
	glColor3f( 1.f, 1.f, 1.f );
	//DoRasterString( 5.f, 5.f, 0.f, (char *)"Text That Doesn't" );

	// swap the double-buffered framebuffers:

	glutSwapBuffers( );

	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush( ) here, not glFinish( ) !

	glFlush( );
}


void
DoAxesMenu( int id )
{
	AxesOn = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


void
DoColorMenu( int id )
{
	NowColor = id - RED;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
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
DoProjectMenu( int id )
{
	NowProjection = id;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
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
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitMenus.\n");

	glutSetWindow( MainWindow );

	int numColors = sizeof( Colors ) / ( 3*sizeof(float) );
	int colormenu = glutCreateMenu( DoColorMenu );
	for( int i = 0; i < numColors; i++ )
	{
		glutAddMenuEntry( ColorNames[i], i );
	}

	int axesmenu = glutCreateMenu( DoAxesMenu );
	glutAddMenuEntry( "Off",  0 );
	glutAddMenuEntry( "On",   1 );

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

	int projmenu = glutCreateMenu( DoProjectMenu );
	glutAddMenuEntry( "Orthographic",  ORTHO );
	glutAddMenuEntry( "Perspective",   PERSP );

	int mainmenu = glutCreateMenu( DoMainMenu );
	glutAddSubMenu(   "Axes",          axesmenu);
	glutAddSubMenu(   "Axis Colors",   colormenu);

#ifdef DEMO_DEPTH_BUFFER
	glutAddSubMenu(   "Depth Buffer",  depthbuffermenu);
#endif

#ifdef DEMO_Z_FIGHTING
	glutAddSubMenu(   "Depth Fighting",depthfightingmenu);
#endif

	glutAddSubMenu(   "Depth Cue",     depthcuemenu);
	glutAddSubMenu(   "Projection",    projmenu );
	glutAddMenuEntry( "Reset",         RESET );
	glutAddSubMenu(   "Debug",         debugmenu);
	glutAddMenuEntry( "Quit",          QUIT );

// attach the pop-up menu to the right mouse button:

	glutAttachMenu( GLUT_RIGHT_BUTTON );
}



// initialize the glut and OpenGL libraries:
//	also setup callback functions

void
InitGraphics( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitGraphics.\n");

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
	glutPassiveMotionFunc(MouseMotion);
	//glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( NULL );
	glutSpaceballRotateFunc( NULL );
	glutSpaceballButtonFunc( NULL );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( -1, NULL, 0 );

	// setup glut to call Animate( ) every time it has
	// 	nothing it needs to respond to (which is most of the time)
	// we don't need to do this for this program, and really should set the argument to NULL
	// but, this sets us up nicely for doing animation

	glutIdleFunc( Animate );

	// init the glew package (a window must be open to do this):

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

	// all other setups go here, such as GLSLProgram and KeyTime setups:
	//int sunWidth, sunHeight;
	//char* sunFile = (char*)"8k_sun.bmp";
	//unsigned char* sunTexture = BmpToTexture(sunFile, &sunWidth, &sunHeight);
	//if (sunTexture == NULL)
	//	fprintf(stderr, "Cannot open texture '%s'\n", sunFile);
	//else
	//	fprintf(stderr, "Opened '%s': width = %d ; height = %d\n", sunFile, sunWidth, sunHeight);

	//glGenTextures(1, &SunTex);
	//glBindTexture(GL_TEXTURE_2D, SunTex);
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexImage2D(GL_TEXTURE_2D, 0, 3, sunWidth, sunHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, sunTexture);
	// all other setups go here, such as GLSLProgram and KeyTime setups:
	for (int i = 0; i < NUMPLANETS; i++)
	{
		int width, height;
		char* file = (char*)Entities[i].file;

		unsigned char* texture = BmpToTexture(file, &width, &height);

		if (texture == NULL)
		{
			fprintf(stderr, "Cannot open texture '%s'\n", file);
		}
		else
		{
			fprintf(stderr, "Opened '%s': width = %d ; height = %d\n", file, width, height);
		}

		glGenTextures(1, &Entities[i].texObject);
		glBindTexture(GL_TEXTURE_2D, Entities[i].texObject);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);
	}

	int width, height;
	char* file = (char*)"8k_stars_milky_way.bmp";

	unsigned char* texture = BmpToTexture(file, &width, &height);

	if (texture == NULL)
	{
		fprintf(stderr, "Cannot open texture '%s'\n", file);
	}
	else
	{
		fprintf(stderr, "Opened '%s': width = %d ; height = %d\n", file, width, height);
	}
	glGenTextures(1, &SolarTex);
	glBindTexture(GL_TEXTURE_2D, SolarTex);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);

}


// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList( )

void
InitLists( )
{
	if (DebugOn != 0)
		fprintf(stderr, "Starting InitLists.\n");

	SphereDL = glGenLists(1);
	glNewList(SphereDL, GL_COMPILE);

		OsuSphere(1., 100, 100);
		
	glEndList();

	//Entities[0].DLObject = glGenLists(1);
	//glNewList(Entities[0].DLObject, GL_COMPILE);
	//	glBindTexture(GL_TEXTURE_2D, Entities[0].texObject);
	//	glPushMatrix();
	//		float scale_factor = SUNSCALE * PLANET_SIZE_SCALE;
	//		glScalef(scale_factor, scale_factor, scale_factor);
	//		glCallList(SphereDL);
	//	glPopMatrix();
	//glEndList();
	SolarDL = glGenLists(1);
	glNewList(SolarDL, GL_COMPILE);
		glBindTexture(GL_TEXTURE_2D, SolarTex);
		glPushMatrix();
			glScalef(STARS_SCALE, STARS_SCALE, STARS_SCALE);
			glCallList(SphereDL);
		glPopMatrix();
	glEndList();




		
	for (int i = 0; i < NUMPLANETS; i++)
	{
		Entities[i].DLObject = glGenLists(1);
		glNewList(Entities[i].DLObject, GL_COMPILE);
			glBindTexture(GL_TEXTURE_2D, Entities[i].texObject);
			fprintf(stderr, "\nOpening: '%s", Entities[i].file);
			fprintf(stderr, " I: '%i'\n", i);
				glPushMatrix();
				if (i == 0)
				{
					float scale = Entities[i].scale * SUN_SIZE_SCALE;
					SetMaterial( .5f, .5f, .5f, 180);
					glScalef(scale, scale, scale);
					glCallList(SphereDL);
				}
				else
				{
					float scale = Entities[i].scale * PLANET_SIZE_SCALE;
					SetMaterialDarkBack(.5f, .5f, .5f, 180);
					glScalef(scale, scale, scale);
					glCallList(SphereDL);
				}
				glPopMatrix();
		glEndList();
	}
	int slices = 720;
	GLfloat LineWidth = 0.5;

	for (int i = 1; i < NUMPLANETS; i++)
	{
		Entities[i].CircleDLObject = glGenLists(1);
		glNewList(Entities[i].CircleDLObject, GL_COMPILE);
		glPushMatrix();
		glLineWidth(LineWidth);
			glBegin(GL_LINE_LOOP);
			for (int j = 0; j <= slices; j++)
			{
				glVertex3f((((Entities[i].AU  * ORBIT_SCALE)) * cos(j * F_2_PI / slices)),
					0.f,
					(((Entities[i].AU ) * ORBIT_SCALE) * sin(j * F_2_PI / slices)));
			}
			glEnd();
		glPopMatrix();
		glEndList();
	}
	glLineWidth(1.0f);
	// create the axes:

	AxesList = glGenLists( 1 );
	glNewList( AxesList, GL_COMPILE );
		glLineWidth( AXES_WIDTH );
			Axes( 1.5 );
		glLineWidth( 1. );
	glEndList( );
}


// the keyboard callback:

//bool FollowVenus;
//bool FollowMercury;
//bool FollowMars;
//bool FollowJupiter;
//bool FollowSaturn;
//bool FollowUranus;
//bool FollowNeptune;

void
Keyboard( unsigned char c, int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );

	switch( c )
	{
		case 'o':
		case 'O':
			NowProjection = ORTHO;
			break;

		case 'p':
		case 'P':
			NowProjection = PERSP;
			break;

		case 'q':
		case 'Q':
		case ESCAPE:
			DoMainMenu( QUIT );	// will not return here
			break;				// happy compiler

		case 'l':
		case 'L':
			if (Lines == 1)
				Lines = 0;
			else
				Lines = 1;
			break;
		case 'e':
		case 'E':
			FollowEarth = !FollowEarth;
			FollowVenus = false;
			FollowMercury = false;
			FollowMars = false;
			FollowJupiter = false;
			FollowSaturn = false;
			FollowUranus = false;
			FollowNeptune = false;
			break;

		case 'v':
		case 'V':
			FollowEarth = false;
			FollowVenus = !FollowVenus;
			FollowMercury = false;
			FollowMars = false;
			FollowJupiter = false;
			FollowSaturn = false;
			FollowUranus = false;
			FollowNeptune = false;
			break;
		case 'm':
		case 'M':
			FollowEarth = false;
			FollowVenus = false;
			FollowMercury = !FollowMercury;
			FollowMars = false;
			FollowJupiter = false;
			FollowSaturn = false;
			FollowUranus = false;
			FollowNeptune = false;
			break;
		case 'w':
		case 'W':
			FollowEarth = false;
			FollowVenus = false;
			FollowMercury = false;
			FollowMars = !FollowMars;
			FollowJupiter = false;
			FollowSaturn = false;
			FollowUranus = false;
			FollowNeptune = false;
			break;

		case 'j':
		case 'J':
			FollowEarth = false;
			FollowVenus = false;
			FollowMercury = false;
			FollowMars = false;
			FollowJupiter = !FollowJupiter;
			FollowSaturn = false;
			FollowUranus = false;
			FollowNeptune = false;
			break;

		case 's':
		case 'S':
			FollowEarth = false;
			FollowVenus = false;
			FollowMercury = false;
			FollowMars = false;
			FollowJupiter = false;
			FollowSaturn = !FollowSaturn;
			FollowUranus = false;
			FollowNeptune = false;
			break;

		case 'u':
		case 'U':
			FollowEarth = false;
			FollowVenus = false;
			FollowMercury = false;
			FollowMars = false;
			FollowJupiter = false;
			FollowSaturn = false;
			FollowUranus = !FollowUranus;
			FollowNeptune = false;
			break;

		case 'n':
		case 'N':
			FollowEarth = false;
			FollowVenus = false;
			FollowMercury = false;
			FollowMars = false;
			FollowJupiter = false;
			FollowSaturn = false;
			FollowUranus = false;
			FollowNeptune = !FollowNeptune;
			break;

		case '0':
			if (LightType == GL_MODULATE)
				LightType = GL_REPLACE;
			else
				LightType = GL_MODULATE;
			break;

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

	if( ( ActiveButton & LEFT ) != 0 )
	{
		Xrot += ( ANGFACT*dy );
		Yrot += ( ANGFACT*dx );
	}

	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );

		// keep object from turning inside-out or disappearing:

		if( Scale < MINSCALE )
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene

void
Reset( )
{
	ActiveButton = 0;
	AxesOn = 1;
	DebugOn = 0;
	DepthBufferOn = 1;
	DepthFightingOn = 0;
	DepthCueOn = 0;
	Scale  = 1.0;
	ShadowsOn = 0;
	NowColor = YELLOW;
	NowProjection = PERSP;
	Xrot = Yrot = 0.;
	initializeOP();
	Lines = 1;
	FollowEarth = false;
	FollowMercury = false;
	FollowVenus = false;
	FollowMars = false;
	FollowJupiter = false;
	FollowSaturn = false;
	FollowNeptune = false;
	LightType = GL_MODULATE;
}


// called when user resizes the window:

void
Resize( int width, int height )
{
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

static float xx[ ] = { 0.f, 1.f, 0.f, 1.f };

static float xy[ ] = { -.5f, .5f, .5f, -.5f };

static int xorder[ ] = { 1, 2, -3, 4 };

static float yx[ ] = { 0.f, 0.f, -.5f, .5f };

static float yy[ ] = { 0.f, .6f, 1.f, 1.f };

static int yorder[ ] = { 1, 2, 3, -2, 4 };

static float zx[ ] = { 1.f, 0.f, 1.f, 0.f, .25f, .75f };

static float zy[ ] = { .5f, .5f, -.5f, -.5f, 0.f, 0.f };

static int zorder[ ] = { 1, 2, 3, 4, -5, 6 };

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


float
Unit( float v[3] )
{
	float dist = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	if (dist > 0.0)
	{
		dist = sqrtf(dist);
		v[0] /= dist;
		v[1] /= dist;
		v[2] /= dist;
	}
	return dist;
}
