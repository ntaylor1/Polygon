

#include <windows.h>
#include <GL/glut.h>
#include <cstdlib>
#include <vector>
#include <math.h>
#include <iostream>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif
static float pointSize = 8.0;
using namespace std;
// Set this to true to animate.
static bool spinningleft = true;
static bool spinningright = false;
static GLsizei width, height;
// This is the number of frames per second to render.
static const int FPS = 600;

// This global variable keeps track of the current orientation of the square.
// It is better to maintain the "state" of the animation globally rather
// than trying to successively accumulate transformations.  Over a period of
// time the approach of accumulating transformation matrices generally
// degrades due to rounding errors.
static GLfloat currentAngleOfRotation = 60.0;

class Point
{
public:
   Point(int x, int y)
   {
	  xVal = x; yVal = y;
   }
   Point(){};
   void setCoords(int x, int y)
   {
	  xVal = x; yVal = y;
   }
   void drawPoint(void); // Function to draw a point.
   int getX()
   {
       return xVal;
   }
   int getY()
   {
       return yVal;
   }

public:
   int xVal, yVal; // x and y co-ordinates of point.
   static float size; // Size of point.
};


float Point::size = pointSize; // Set point size.

// Function to draw a point.
void Point::drawPoint(void)
{
   glPointSize(size);
   glBegin(GL_POINTS);
      glVertex3f(xVal, yVal, 0.0);
   glEnd();
}

boolean flag = false;
boolean scale = false;
// Vector of points.
vector<Point> points;

Point p;

// Iterator to traverse a Point array.
vector<Point>::iterator pointsIterator;

// Currently clicked point.
Point currentPoint;

void setup(void)
{
   p = {0,0};
   glClearColor(1.0, 1.0, 1.0, 0.0);
}


//Transformation matrix code
// ----------------------------------------------------------------------------------------------

typedef GLfloat Matrix3x3 [3][3];

Matrix3x3 matComposite;

const GLdouble pi = 3.14159;


/* Construct the 3 by 3 identity matrix. */
void matrix3x3SetIdentity (Matrix3x3 matIdent3x3)
{
   GLint row, col;

   for (row = 0; row < 3; row++)
      for (col = 0; col < 3; col++)
         matIdent3x3 [row][col] = (row == col);
}

/* Premultiply matrix m1 times matrix m2, store result in m2. */
void matrix3x3PreMultiply (Matrix3x3 m1, Matrix3x3 m2)
{
   GLint row, col;
   Matrix3x3 matTemp;

   for (row = 0; row < 3; row++)
      for (col = 0; col < 3 ; col++)
         matTemp [row][col] = m1 [row][0] * m2 [0][col] + m1 [row][1] *
                            m2 [1][col] + m1 [row][2] * m2 [2][col];

   for (row = 0; row < 3; row++)
      for (col = 0; col < 3; col++)
         m2 [row][col] = matTemp [row][col];
}

void translate2D (GLfloat tx, GLfloat ty)
{
   Matrix3x3 matTransl;

   /*  Initialize translation matrix to identity.  */
   matrix3x3SetIdentity (matTransl);

   matTransl [0][2] = tx;
   matTransl [1][2] = ty;

   /*  Concatenate matTransl with the composite matrix.  */
   matrix3x3PreMultiply (matTransl, matComposite);
}

void rotate2D (Point pivotPt, GLfloat theta)
{
   Matrix3x3 matRot;

   /*  Initialize rotation matrix to identity.  */
   matrix3x3SetIdentity (matRot);

   matRot [0][0] = cos (theta);
   matRot [0][1] = -sin (theta);
   matRot [0][2] = pivotPt.xVal * (1 - cos (theta)) +
                        pivotPt.yVal * sin (theta);
   matRot [1][0] = sin (theta);
   matRot [1][1] = cos (theta);
   matRot [1][2] = pivotPt.yVal * (1 - cos (theta)) -
                        pivotPt.xVal * sin (theta);

   /*  Concatenate matRot with the composite matrix.  */
   matrix3x3PreMultiply (matRot, matComposite);
}

void scale2D (GLfloat sx, GLfloat sy, Point fixedPt)
{
   Matrix3x3 matScale;

   /*  Initialize scaling matrix to identity.  */
   matrix3x3SetIdentity (matScale);

   matScale [0][0] = sx;
   matScale [0][2] = (1 - sx) * fixedPt.xVal;
   matScale [1][1] = sy;
   matScale [1][2] = (1 - sy) * fixedPt.yVal;

   /*  Concatenate matScale with the composite matrix.  */
   matrix3x3PreMultiply (matScale, matComposite);
}

/* Using the composite matrix, calculate transformed coordinates. */
void transformVerts2D (GLint nVerts, Point * verts)
{
   GLint k;
   GLfloat temp;

   for (k = 0; k < nVerts; k++) {
      temp = matComposite [0][0] * verts [k].xVal + matComposite [0][1] *
             verts [k].yVal + matComposite [0][2];
      verts [k].yVal = matComposite [1][0] * verts [k].xVal + matComposite [1][1] *
                  verts [k].yVal + matComposite [1][2];
         verts [k].xVal = temp;
   }
}


// -----------------------------------------------------------------------------------

//draws the polygon
void drawPoly(Point *verts,int sides)
{
    GLint k;
    glBegin (GL_POLYGON);
       for (k = 0; k < sides; k++)
       {

           glVertex2i (verts [k].xVal, verts [k].yVal);
       }

    glEnd ( );
}





//window reshape function
void reshape(GLint w, GLint h) {
  glViewport(0, 0, w, h);
  GLfloat aspect = (GLfloat)w / (GLfloat)h;
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  width = w;
  height = h;

  glOrtho(0.0, (float)w, 0.0, (float)h, -1.0, 1.0);


}


void display() {

  GLint nVerts = 0;

  Point centroid;

  glClear(GL_COLOR_BUFFER_BIT);
  glColor3f(0.0, 0.0, 0.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  int counter = 0;

   //stars with points and draws them
   pointsIterator = points.begin();
   while(pointsIterator != points.end() )
   {
      counter++;
      if(flag == false)
      {
          pointsIterator->drawPoint();
      }

	  pointsIterator++;
   }



    //main loop to draw the polygon
    for(int x = 3; x < counter; x++)
   {

       if(points[x].getX() == points[x-1].getX())
       {
           flag = true; //user has specified all sides, input is finished
           nVerts = x;
           Point verts[nVerts];


           for(int z = 0; z < points.size() - 1;z++)
           {
               verts[z] = points.at(z);

           }


           GLint k, xSum = 0, ySum = 0;
           for (k = 0; k < nVerts;  k++) {
                xSum += verts [k].xVal;
                ySum += verts [k].yVal;
            }

           matrix3x3SetIdentity (matComposite);

           Point centroidPt;
           centroidPt.xVal = GLfloat (xSum) / GLfloat (nVerts);
           centroidPt.yVal = GLfloat (ySum) / GLfloat (nVerts);
           Point pivPt = centroidPt;
           GLdouble theta = currentAngleOfRotation;

           //continually rotate the polygon
           rotate2D (pivPt, theta);
           transformVerts2D (nVerts, verts);

           //p is initialized to 0, will be assigned a value when translation is needed
           if(p.xVal != 0)
           {
               rotate2D (pivPt, 360 - theta);
               translate2D(p.xVal - centroidPt.xVal,p.yVal - centroidPt.yVal);
               transformVerts2D (nVerts, verts);
           }

           //scale is initialized to false, becomes true when user uses shift key to scale
           if(scale == true)
           {

               scale2D(2,2,centroidPt);
               transformVerts2D (nVerts, verts);

           }




           //invokes the drawing function
           drawPoly(verts,nVerts);





       }
   }


  glFlush();
  glutSwapBuffers();
}

// Handles the timer by incrementing the angle of rotation and requesting the
// window to display again, provided the program is in the spinning state.
// Since the timer function is only called once, it sets the same function to
// be called again.
void timer(int v) {
  if (spinningleft) {
    currentAngleOfRotation += 0.04;
    if (currentAngleOfRotation > 360.0) {
      currentAngleOfRotation -= 360.0;
    }
    glutPostRedisplay();
  }

  if (spinningright) {
    currentAngleOfRotation -= 0.04;
    if (currentAngleOfRotation > 360.0) {
      currentAngleOfRotation -= 360.0;
    }
    glutPostRedisplay();
  }

  glutTimerFunc(1000/FPS, timer, v);
}

// Handles mouse events as follows: when the left button is pressed, generate
// new animation frames while the application is idle; when the right button
// is pressed, remove any idle-time callbacks, thus stopping the animation.
void mouse(int button, int state, int x, int y) {


  int counter = 0;
   // Store the clicked point in the currentPoint variable when left button is pressed.
   if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
      currentPoint = Point(x, height - y);

   // Store the currentPoint in the points vector when left button is released.
   if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
   {
       if(flag == false)
       {
           points.push_back(currentPoint);
       }

       if(flag == true)
       {
           currentPoint = Point(x, height - y);
           p = currentPoint;

       }


       //sets the rotation direction to the opposite when alt pressed
       int modifier = glutGetModifiers();
       if(modifier == GLUT_ACTIVE_ALT)
       {
           switch(spinningleft)
           {
               case true:
                   spinningleft = false;
                   spinningright = true;
                   break;
               case false:
                    spinningleft = true;
                    spinningright = false;
                    break;

           }

       }
       //scale when shift is pressed
       if(modifier == GLUT_ACTIVE_SHIFT)
       {
           scale = true;

       }




   }


   if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) exit(0);





   glutPostRedisplay();



}


void mouseMotion(int x, int y)
{
   // Update the location of the current point as the mouse moves with button pressed.
   if(flag == false)
    currentPoint.setCoords(x, height - y);
   glutPostRedisplay();
}





// Initializes GLUT, the display mode, and main window; registers callbacks;
// enters the main event loop.
int main(int argc, char** argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowPosition(80, 80);
  glutInitWindowSize(800, 500);
  glutCreateWindow("Spinning Square");
  setup();
  glutReshapeFunc(reshape);
  glutDisplayFunc(display);
  glutTimerFunc(100, timer, 0);

  glutMouseFunc(mouse);
  glutMotionFunc(mouseMotion);
  glutMainLoop();
}
