#include <GL/glew.h>
#include <GL/glfw3.h>
#include <math.h>

#define M_PI 3.14159265358979323846  /* pi */
#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 800

void drawDotCircle( GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLint numberOfSides );
void drawHollowCircle( GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLint numberOfSides );
void drawTriangleCircle( GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLint numberOfSides );

int main( void )
{
    GLFWwindow *window;

    // Initialize the library
    if ( !glfwInit( ) )
    {
        return -1;
    }

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow( SCREEN_WIDTH, SCREEN_HEIGHT, "Transform", NULL, NULL );

    if ( !window )
    {
        glfwTerminate( );
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent( window );

    glViewport( 0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT ); // specifies the part of the window to which OpenGL will draw (in pixels), convert from normalised to pixels
    glMatrixMode( GL_PROJECTION ); // projection matrix defines the properties of the camera that views the objects in the world coordinate frame. Here you typically set the zoom factor, aspect ratio and the near and far clipping planes
    glLoadIdentity( ); // replace the current matrix with the identity matrix and starts us a fresh because matrix transforms such as glOrpho and glRotate cumulate, basically puts us at (0, 0, 0)
    glOrtho( 0, SCREEN_WIDTH, 0, SCREEN_HEIGHT, 0, 1 ); // essentially set coordinate system
    glMatrixMode( GL_MODELVIEW ); // (default matrix mode) modelview matrix defines how your objects are transformed (meaning translation, rotation and scaling) in your world
    glLoadIdentity( ); // same as above comment

    GLfloat pointVertex[] = { SCREEN_WIDTH / 8, SCREEN_HEIGHT*3 / 4 };
    GLfloat lineVertices[] =
    {
        200, 350, 0,
        280, 550, 0
    };

    GLfloat triangleVertices[] =
    {
        350, 525, 0, // top left corner
        450, 375, 0, // bottom middle corner
        550, 525, 0  // top right corner
    };

    GLfloat rectangleVertices[] =
    {
        800, 450, 0.0, // top right corner
        725, 525, 0.0, // top left corner
        650, 450, 0.0, // bottom left corner
        725, 375, 0.0   // bottom right corner
    };

    glTranslatef(150,600,0);
    glRotatef(300,0,0,0.5);
    glScalef(0.5,0.5,0);

    // Loop until the user closes the window
    while ( !glfwWindowShouldClose( window ) )
    {
        glClear( GL_COLOR_BUFFER_BIT );

        // Render OpenGL here
        //Make a Dot(Point)
        glEnable( GL_POINT_SMOOTH ); // make the point circular
        glEnableClientState( GL_VERTEX_ARRAY ); // tell OpenGL that you're using a vertex array for fixed-function attribute
        glPointSize( 28 ); // must be added before glDrawArrays is called
        glVertexPointer( 2, GL_FLOAT, 0, pointVertex ); // point to the vertices to be used
        glDrawArrays( GL_POINTS, 0, 1 ); // draw the vertixes
        glDisableClientState( GL_VERTEX_ARRAY ); // tell OpenGL that you're finished using the vertex arrayattribute
        glDisable( GL_POINT_SMOOTH ); // stop the smoothing to make the points circular

        //Make a Line
        glLineWidth( 4 );
        glEnableClientState( GL_VERTEX_ARRAY );
        glVertexPointer( 3, GL_FLOAT, 0, lineVertices );
        glDrawArrays( GL_LINES, 0, 2 );
        glDisableClientState( GL_VERTEX_ARRAY );

        //Make a Triangle
        glEnableClientState( GL_VERTEX_ARRAY );
        glVertexPointer( 3, GL_FLOAT, 0, triangleVertices );
        glDrawArrays( GL_TRIANGLES, 0, 3 );
        glDisableClientState( GL_VERTEX_ARRAY );

        //Make a Rectangle
        glEnableClientState( GL_VERTEX_ARRAY ); // tell OpenGL that you're using a vertex array for fixed-function attribute
        glVertexPointer( 3, GL_FLOAT, 0, rectangleVertices ); // point to the vertices to be used
        glDrawArrays( GL_QUADS, 0, 4 ); // draw the vertixes
        glDisableClientState( GL_VERTEX_ARRAY ); // tell OpenGL that you're finished using the vertex arrayattribute

        drawDotCircle(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 4, 0, 80, 100);
        drawHollowCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 4, 0, 80, 100);
        drawTriangleCircle(SCREEN_WIDTH*3 / 4, SCREEN_HEIGHT / 4, 0, 80, 10);
        // Swap front and back buffers
        glfwSwapBuffers( window );

        // Poll for and process events
        glfwPollEvents( );
    }

    glfwTerminate( );

    return 0;
}

void drawDotCircle( GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLint numberOfSides)
{
    GLint numberOfVertices = numberOfSides + 1;

    GLfloat doublePi = 2.0f * M_PI;

    GLfloat circleVerticesX[numberOfVertices];
    GLfloat circleVerticesY[numberOfVertices];
    GLfloat circleVerticesZ[numberOfVertices];

    //circleVerticesX[0] = x;
    //circleVerticesY[0] = y;
    //circleVerticesZ[0] = z;


    for ( int i = 0; i < numberOfVertices; i++ )
    {
        circleVerticesX[i] = x + ( radius * cos( i * doublePi / numberOfSides ) );
        circleVerticesY[i] = y + ( radius * sin( i * doublePi / numberOfSides ) );
        circleVerticesZ[i] = z;
    }

    GLfloat allCircleVertices[numberOfVertices * 3];

    for ( int i = 0; i < numberOfVertices; i++ )
    {
        allCircleVertices[i * 3] = circleVerticesX[i];
        allCircleVertices[( i * 3 ) + 1] = circleVerticesY[i];
        allCircleVertices[( i * 3 ) + 2] = circleVerticesZ[i];
    }

        glEnable( GL_LINE_SMOOTH );
        glEnable( GL_LINE_STIPPLE );
        glPushAttrib( GL_LINE_BIT );
        glLineWidth( 2 );
        glLineStipple( 2, 0x00FF );
        glEnableClientState( GL_VERTEX_ARRAY );
        glVertexPointer( 3, GL_FLOAT, 0, allCircleVertices );
        glDrawArrays( GL_LINES, 0, numberOfVertices );
        glDisableClientState( GL_VERTEX_ARRAY );
        glPopAttrib( );
        glDisable( GL_LINE_STIPPLE );
        glDisable( GL_LINE_SMOOTH );
}

void drawHollowCircle( GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLint numberOfSides)
{
    GLint numberOfVertices = numberOfSides + 1;

    GLfloat doublePi = 2.0f * M_PI;

    GLfloat circleVerticesX[numberOfVertices];
    GLfloat circleVerticesY[numberOfVertices];
    GLfloat circleVerticesZ[numberOfVertices];

    //circleVerticesX[0] = x;
    //circleVerticesY[0] = y;
    //circleVerticesZ[0] = z;


    for ( int i = 0; i < numberOfVertices; i++ )
    {
        circleVerticesX[i] = x + ( radius * cos( i * doublePi / numberOfSides ) );
        circleVerticesY[i] = y + ( radius * sin( i * doublePi / numberOfSides ) );
        circleVerticesZ[i] = z;
    }

    GLfloat allCircleVertices[numberOfVertices * 3];

    for ( int i = 0; i < numberOfVertices; i++ )
    {
        allCircleVertices[i * 3] = circleVerticesX[i];
        allCircleVertices[( i * 3 ) + 1] = circleVerticesY[i];
        allCircleVertices[( i * 3 ) + 2] = circleVerticesZ[i];
    }

        glEnableClientState( GL_VERTEX_ARRAY );
        glVertexPointer( 3, GL_FLOAT, 0, allCircleVertices );
        glDrawArrays( GL_LINE_STRIP, 0, numberOfVertices );
        glDisableClientState( GL_VERTEX_ARRAY );
}

void drawTriangleCircle( GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLint numberOfSides)
{
    GLint numberOfVertices = numberOfSides + 1;

    GLfloat doublePi = 2.0f * M_PI;

    GLfloat circleVerticesX[numberOfVertices];
    GLfloat circleVerticesY[numberOfVertices];
    GLfloat circleVerticesZ[numberOfVertices];

    circleVerticesX[0] = x;
    circleVerticesY[0] = y;
    circleVerticesZ[0] = z;


    for ( int i = 0; i < numberOfVertices; i++ )
    {
        circleVerticesX[i] = x + ( radius * cos( i * doublePi / numberOfSides ) );
        circleVerticesY[i] = y + ( radius * sin( i * doublePi / numberOfSides ) );
        circleVerticesZ[i] = z;
    }

    GLfloat allCircleVertices[numberOfVertices * 3];

    for ( int i = 0; i < numberOfVertices; i++ )
    {
        allCircleVertices[i * 3] = circleVerticesX[i];
        allCircleVertices[( i * 3 ) + 1] = circleVerticesY[i];
        allCircleVertices[( i * 3 ) + 2] = circleVerticesZ[i];
    }

        glEnableClientState( GL_VERTEX_ARRAY );
        glVertexPointer( 3, GL_FLOAT, 0, allCircleVertices );
        glDrawArrays( GL_TRIANGLE_FAN, 0, numberOfVertices );
        glDisableClientState( GL_VERTEX_ARRAY );
}
