#include <stdio.h>
#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <math.h>

#define M_PI 3.1415927

static int slices = 16;
static int stacks = 16;
GLUquadricObj *IDquadric;

/* GLUT callback Handlers */
void draw_sphere(float radius, int slices, int stacks)
{
    int i, j;
    for(i = 0; i <= slices; i++)
    {
        double lat0 = M_PI * (-0.5 + (double) (i - 1) / slices);
        double z0  = sin(lat0);
        double zr0 =  cos(lat0);

        double lat1 = M_PI * (-0.5 + (double) i / slices);
        double z1 = sin(lat1);
        double zr1 = cos(lat1);

        glBegin(GL_QUAD_STRIP);
        for(j = 0; j <= stacks; j++)
        {
            double lng = 2 * M_PI * (double) (j - 1) / stacks;
            double x = cos(lng);
            double y = sin(lng);

            glNormal3f(x * zr0, y * zr0, z0);
            glVertex3f(x * zr0, y * zr0, z0);
            glNormal3f(x * zr1, y * zr1, z1);
            glVertex3f(x * zr1, y * zr1, z1);
        }
    glEnd();
    }
}


void draw_cylinder(GLfloat radius, GLfloat height, GLfloat R, GLfloat G, GLfloat B)
{
    GLfloat x              = 0.0;
    GLfloat y              = 0.0;
    GLfloat angle          = 0.0;
    GLfloat angle_stepsize = 0.1;

    /** Draw the tube */
    glColor3ub(R-40,G-40,B-40);
    glBegin(GL_QUAD_STRIP);
    angle = 0.0;
        while( angle < 2*M_PI ) {
            x = radius * cos(angle);
            y = radius * sin(angle);
            glVertex3f(x, y , height);
            glVertex3f(x, y , 0.0);
            angle = angle + angle_stepsize;
        }
        glVertex3f(radius, 0.0, height);
        glVertex3f(radius, 0.0, 0.0);
    glEnd();

    /** Draw the circle on top of cylinder */
    glColor3ub(R,G,B);
    glBegin(GL_POLYGON);
    angle = 0.0;
        while( angle < 2*M_PI ) {
            x = radius * cos(angle);
            y = radius * sin(angle);
            glVertex3f(x, y , height);
            angle = angle + angle_stepsize;
        }
        glVertex3f(radius, 0.0, height);
    glEnd();
}

static void resize(int width, int height)
{
    const float ar = (float) width / (float) height;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-ar, ar, -1.0, 1.0, 2.0, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity() ;
}

void keyboard(unsigned char key, int x, int y);
void display(void);

static void idle(void)
{
    glutPostRedisplay();
}

const GLfloat light_ambient[]  = { 1.0f, 0.4f, 0.5f, 1.0f };
const GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { 2.0f, 5.0f, 5.0f, 0.0f };

const GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 100.0f };

/* Program entry point */

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitWindowSize(800,400);
    glutInitWindowPosition(10,10);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

    glutCreateWindow("GLUT Shapes");

    glutReshapeFunc(resize);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(idle);

    glClearColor(1,1,1,1);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);

    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

    glutMainLoop();

    return EXIT_SUCCESS;
}

void keyboard(unsigned char key, int x, int y)
{
  switch (key)
  {
    case '\x1B':
      exit(EXIT_SUCCESS);
      break;
  }
}

void display()
{
    const double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    const double a = t*90.0;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3d(0,1,1);
//primitive sphere
    glPushMatrix();
        glTranslated(-1.2,1.2,-6);
        glRotated(85,1,0,0);
        draw_sphere(1,slices,stacks);
    glPopMatrix();
//GLUT Sphere
    glPushMatrix();
        glTranslated(-1.2,-1.2,-6);
        glRotated(85,1,0,0);
        glutSolidSphere(1,slices,stacks);
    glPopMatrix();
//primitive cylinder
    glPushMatrix();
        glTranslated(1.2,1.8,-6);
        glRotated(85,1,0,0);
        draw_cylinder(1,1.5,80,200,300);
    glPopMatrix();
//GLUT cylinder
    glPushMatrix();
        glTranslated(1.2,-0.8,-6);
        glRotated(85,1,0,0);
        IDquadric = gluNewQuadric();
        gluCylinder(IDquadric,1,1,1.5,slices,stacks);
    glPopMatrix();

    glutSwapBuffers();
}
