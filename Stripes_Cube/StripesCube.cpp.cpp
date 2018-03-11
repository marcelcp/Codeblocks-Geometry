#pragma GCC diagnostic ignored "-Wwrite-strings"
#include <windows.h>
#include <stdio.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glut.h>
#include "tgaload.h"
#define MAX_NO_TEXTURES 1

#define CUBE_TEXTURE 0

GLuint texture_id[MAX_NO_TEXTURES];

float xrot;
float yrot;
float zrot;
float ratio;



#include <windows.h>
#include <GL\glu.h>
#include <stdio.h>
#include <mem.h>
#include "tgaload.h"

/* Extension Management */
PFNGLCOMPRESSEDTEXIMAGE2DARBPROC  glCompressedTexImage2DARB  = NULL;
PFNGLGETCOMPRESSEDTEXIMAGEARBPROC glGetCompressedTexImageARB = NULL;

/* Default support - lets be optimistic! */
bool tgaCompressedTexSupport = true;


void tgaGetExtensions ( void )
{
   glCompressedTexImage2DARB  = ( PFNGLCOMPRESSEDTEXIMAGE2DARBPROC  )
                   wglGetProcAddress ( "glCompressedTexImage2DARB"  );
   glGetCompressedTexImageARB = ( PFNGLGETCOMPRESSEDTEXIMAGEARBPROC )
                   wglGetProcAddress ( "glGetCompressedTexImageARB" );

   if ( glCompressedTexImage2DARB == NULL || glGetCompressedTexImageARB == NULL )
   	tgaCompressedTexSupport = false;
}


void tgaSetTexParams  ( unsigned int min_filter, unsigned int mag_filter, unsigned int application )
{
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter );

   glTexEnvf ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, application );
}


unsigned char *tgaAllocMem ( tgaHeader_t info )
{
   unsigned char  *block;

   block = (unsigned char*) malloc ( info.bytes );

   if ( block == NULL )
      return 0;

   memset ( block, 0x00, info.bytes );

   return block;
}

void tgaPutPacketTuples ( image_t *p, unsigned char *temp_colour, int &current_byte )
{
   if ( p->info.components == 3 )
   {
      p->data[current_byte]   = temp_colour[2];
      p->data[current_byte+1] = temp_colour[1];
      p->data[current_byte+2] = temp_colour[0];
      current_byte += 3;
   }

   if ( p->info.components == 4 )    // Because its BGR(A) not (A)BGR :(
   {
      p->data[current_byte]   = temp_colour[2];
      p->data[current_byte+1] = temp_colour[1];
      p->data[current_byte+2] = temp_colour[0];
      p->data[current_byte+3] = temp_colour[3];
      current_byte += 4;
   }
}


void tgaGetAPacket ( int &current_byte, image_t *p, FILE *file )
{
   unsigned char  packet_header;
   int            run_length;
   unsigned char  temp_colour[4] = { 0x00, 0x00, 0x00, 0x00 };

   fread        ( &packet_header, ( sizeof ( unsigned char )), 1, file );
   run_length = ( packet_header&0x7F ) + 1;

   if ( packet_header&0x80 )  // RLE packet
   {
      fread ( temp_colour, ( sizeof ( unsigned char )* p->info.components ), 1, file );

      if ( p->info.components == 1 )  // Special optimised case :)
      {
         memset ( p->data + current_byte, temp_colour[0], run_length );
         current_byte += run_length;
      } else
      for ( int i = 0; i < run_length; i++ )
         tgaPutPacketTuples ( p, temp_colour, current_byte );
   }

   if ( !( packet_header&0x80 ))  // RAW packet
   {
      for ( int i = 0; i < run_length; i++ )
      {
         fread ( temp_colour, ( sizeof ( unsigned char )* p->info.components ), 1, file );

         if ( p->info.components == 1 )
         {
            memset ( p->data + current_byte, temp_colour[0], run_length );
            current_byte += run_length;
         } else
            tgaPutPacketTuples ( p, temp_colour, current_byte );
      }
   }
}


void tgaGetPackets ( image_t *p, FILE *file )
{
  int current_byte = 0;

  while ( current_byte < p->info.bytes )
    tgaGetAPacket ( current_byte, p, file );
}


void tgaGetImageData ( image_t *p, FILE *file )
{
   unsigned char  temp;

   p->data = tgaAllocMem ( p->info );

   /* Easy unRLE image */
   if ( p->info.image_type == 1 || p->info.image_type == 2 || p->info.image_type == 3 )
   {
      fread ( p->data, sizeof (unsigned char), p->info.bytes, file );

      /* Image is stored as BGR(A), make it RGB(A)     */
      for (int i = 0; i < p->info.bytes; i += p->info.components )
      {
         temp = p->data[i];
         p->data[i] = p->data[i + 2];
         p->data[i + 2] = temp;
      }
   }

   /* RLE compressed image */
   if ( p->info.image_type == 9 || p->info.image_type == 10 )
      tgaGetPackets ( p, file );
}


void tgaUploadImage ( image_t *p, tgaFLAG mode )
{
   /*  Determine TGA_LOWQUALITY  internal format
       This directs OpenGL to upload the textures at half the bit
       precision - saving memory
    */
   GLenum internal_format = p->info.tgaColourType;

	if ( mode&TGA_LOW_QUALITY )
   {
      switch ( p->info.tgaColourType )
      {
         case GL_RGB       : internal_format = GL_RGB4; break;
         case GL_RGBA      : internal_format = GL_RGBA4; break;
         case GL_LUMINANCE : internal_format = GL_LUMINANCE4; break;
         case GL_ALPHA     : internal_format = GL_ALPHA4; break;
      }
   }

   /*  Let OpenGL decide what the best compressed format is each case. */
   if ( mode&TGA_COMPRESS && tgaCompressedTexSupport )
   {
      switch ( p->info.tgaColourType )
      {
         case GL_RGB       : internal_format = GL_COMPRESSED_RGB_ARB; break;
         case GL_RGBA      : internal_format = GL_COMPRESSED_RGBA_ARB; break;
         case GL_LUMINANCE : internal_format = GL_COMPRESSED_LUMINANCE_ARB; break;
         case GL_ALPHA     : internal_format = GL_COMPRESSED_ALPHA_ARB; break;
      }
   }

   /*  Pass OpenGL Texture Image */
   if ( !( mode&TGA_NO_MIPMAPS ))
      gluBuild2DMipmaps ( GL_TEXTURE_2D, internal_format, p->info.width,
                          p->info.height, p->info.tgaColourType, GL_UNSIGNED_BYTE, p->data );
   else
      glTexImage2D ( GL_TEXTURE_2D, 0, internal_format, p->info.width,
                     p->info.height, 0, p->info.tgaColourType, GL_UNSIGNED_BYTE, p->data );
}


void tgaFree ( image_t *p )
{
   if ( p->data != NULL )
      free ( p->data );
}


void tgaChecker ( image_t *p )
{
  unsigned char TGA_CHECKER[16384];
  unsigned char *pointer;

  // 8bit image
  p->info.image_type = 3;

  p->info.width  = 128;
  p->info.height = 128;

  p->info.pixel_depth = 8;

  // Set some stats
  p->info.components = 1;
  p->info.bytes      = p->info.width * p->info.height * p->info.components;

  pointer = TGA_CHECKER;

  for ( int j = 0; j < 128; j++ )
  {
    for ( int i = 0; i < 128; i++ )
    {
      if ((i ^ j) & 0x10 )
        pointer[0] = 0x00;
      else
        pointer[0] = 0xff;
      pointer ++;
    }
  }

  p->data = TGA_CHECKER;

  glTexImage2D ( GL_TEXTURE_2D, 0, GL_LUMINANCE4, p->info.width,
                 p->info.height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, p->data );

/*  Should we free?  I dunno.  The scope of TGA_CHECKER _should_ be local, so it
    probably gets destroyed automatically when the function completes... */
//  tgaFree ( p );
}


void tgaError ( char *error_string, char *file_name, FILE *file, image_t *p )
{
   printf  ( "%s - %s\n", error_string, file_name );
   tgaFree ( p );

   fclose ( file );

   tgaChecker ( p );
}


void tgaGetImageHeader ( FILE *file, tgaHeader_t *info )
{
   /*   Stupid byte alignment means that we have to fread each field
        individually.  I tried splitting tgaHeader into 3 structures, no matter
        how you arrange them, colour_map_entry_size comes out as 2 bytes instead
        1 as it should be.  Grrr.  Gotta love optimising compilers - theres a pragma
        for Borland, but I dunno the number for MSVC or GCC :(
    */
   fread ( &info->id_length,       ( sizeof (unsigned char )), 1, file );
   fread ( &info->colour_map_type, ( sizeof (unsigned char )), 1, file );
   fread ( &info->image_type,      ( sizeof (unsigned char )), 1, file );

   fread ( &info->colour_map_first_entry, ( sizeof (short int )), 1, file );
   fread ( &info->colour_map_length     , ( sizeof (short int )), 1, file );
   fread ( &info->colour_map_entry_size , ( sizeof (unsigned char )), 1, file );

   fread ( &info->x_origin , ( sizeof (short int )), 1, file );
   fread ( &info->y_origin , ( sizeof (short int )), 1, file );
   fread ( &info->width,     ( sizeof (short int )), 1, file );
   fread ( &info->height,    ( sizeof (short int )), 1, file );

   fread ( &info->pixel_depth,     ( sizeof (unsigned char )), 1, file );
   fread ( &info->image_descriptor,( sizeof (unsigned char )), 1, file );

   // Set some stats
   info->components = info->pixel_depth / 8;
   info->bytes      = info->width * info->height * info->components;
}

int tgaLoadTheImage ( char *file_name, image_t *p, tgaFLAG mode )
{
   FILE   *file;

   tgaGetExtensions ( );

   p->data = NULL;

   if (( file = fopen ( file_name, "rb" )) == NULL )
   {
   	tgaError ( "File not found", file_name, file, p );
      return 0;
   }

   tgaGetImageHeader ( file, &p->info );

   switch ( p->info.image_type )
   {
      case 1 :
         tgaError ( "8-bit colour no longer supported", file_name, file, p );
         return 0;

      case 2 :
         if ( p->info.pixel_depth == 24 )
            p->info.tgaColourType = GL_RGB;
         else if ( p->info.pixel_depth == 32 )
            p->info.tgaColourType = GL_RGBA;
			else
	      {
         	tgaError ( "Unsupported RGB format", file_name, file, p );
         	return 0;
         }
         break;

      case 3 :
         if ( mode&TGA_LUMINANCE )
            p->info.tgaColourType = GL_LUMINANCE;
         else if ( mode&TGA_ALPHA )
            p->info.tgaColourType = GL_ALPHA;
      	else
	      {
         	tgaError ( "Must be LUMINANCE or ALPHA greyscale", file_name, file, p );
				return 0;
         }
         break;

      case 9 :
         tgaError ( "8-bit colour no longer supported", file_name, file, p );
         return 0;

      case 10 :
         if ( p->info.pixel_depth == 24 )
            p->info.tgaColourType = GL_RGB;
         else if ( p->info.pixel_depth == 32 )
            p->info.tgaColourType = GL_RGBA;
         else
	      {
         	tgaError ( "Unsupported compressed RGB format", file_name, file, p );
         	return 0;
         }
   }

   tgaGetImageData ( p, file );

   fclose  ( file );

   return 1;
}

void tgaLoad ( char *file_name, image_t *p, tgaFLAG mode )
{
	if ( tgaLoadTheImage ( file_name, p, mode ))
   {
   	if  ( !( mode&TGA_NO_PASS ))
     	 tgaUploadImage  ( p, mode );

   	if ( mode&TGA_FREE )
     		tgaFree ( p );
   }

}

GLuint tgaLoadAndBind ( char *file_name, tgaFLAG mode )
{
   GLuint   texture_id;
   image_t  *p;

   glGenTextures ( 1, &texture_id );
   glBindTexture ( GL_TEXTURE_2D, texture_id );

   if ( tgaLoadTheImage ( file_name, p, mode ))
	{
   	tgaUploadImage  ( p, mode );
   	tgaFree       ( p );
   }

   return texture_id;
}

void init(void)
{
   glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
   glEnable ( GL_COLOR_MATERIAL );
   glColorMaterial ( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );

	glEnable ( GL_TEXTURE_2D );
   glPixelStorei ( GL_UNPACK_ALIGNMENT, 1 );
   glGenTextures (1, texture_id);

   image_t   temp_image;

   glBindTexture ( GL_TEXTURE_2D, texture_id[CUBE_TEXTURE] );
   tgaLoad  ( "Foto.tga", &temp_image, TGA_FREE | TGA_LOW_QUALITY );
   glEnable ( GL_CULL_FACE );

   //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void reshape( int w, int h )
{
	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if(h == 0)
		h = 1;

	ratio = 1.0f * w / h;
	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Set the viewport to be the entire window
    glViewport(0, 0, w, h);

	// Set the clipping volume
	gluPerspective(80,ratio,1,200);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, 30,
		      0,0,10,
			  0.0f,1.0f,0.0f);
}

void display( void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
   glLoadIdentity ( );
   glPushMatrix();
   glTranslatef ( 0.0, 0.0, -5.0 );
   glRotatef ( xrot, 1.0, 0.0, 0.0 );
   glRotatef ( yrot, 0.0, 1.0, 0.0 );
   glRotatef ( zrot, 0.0, 0.0, 1.0 );

   glBindTexture ( GL_TEXTURE_2D, texture_id[0] );

   glBegin ( GL_QUADS );
   // Front Face
   for(GLfloat i=-1.0; i<0.8; i+=0.4)
   {
       glColor3f(1.0,1.0,1.0);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, i,  1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, i,  1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  i+0.2,  1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  i+0.2,  1.0f);

   }
      for(GLfloat i=-0.8; i<1.0; i+=0.4)
   {
       glColor3f(1.0,0.0,0.0);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, i,  1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, i,  1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  i+0.2,  1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  i+0.2,  1.0f);

   }

		// Back Face
		for(GLfloat i=-1.0; i<0.8; i+=0.4)
   {
       glColor3f(1.0,1.0,1.0);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, i, -1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  i+0.2, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  i+0.2, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, i, -1.0f);
   }
   for(GLfloat i=-0.8; i<1.0; i+=0.4)
   {
       glColor3f(1.0,0.0,0.0);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, i, -1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  i+0.2, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  i+0.2, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, i, -1.0f);
   }
		// Top Face
    for(GLfloat i=-1.0; i<0.8; i+=0.4)
   {
       glColor3f(1.0,1.0,1.0);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(i,  1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(i,  1.0f,  1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( i+0.2,  1.0f,  1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( i+0.2,  1.0f, -1.0f);
   }
   for(GLfloat i=-0.8; i<1.0; i+=0.4)
   {
       glColor3f(1.0,0.0,0.0);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(i,  1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(i,  1.0f,  1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( i+0.2,  1.0f,  1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( i+0.2,  1.0f, -1.0f);
   }
		// Bottom Face
    for(GLfloat i=-1.0; i<0.8; i+=0.4)
   {
       glColor3f(1.0,0.0,0.0);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(i, -1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( i+0.2, -1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( i+0.2, -1.0f,  1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(i, -1.0f,  1.0f);
   }
   for(GLfloat i=-0.8; i<1.0; i+=0.4)
   {
       glColor3f(1.0,1.0,1.0);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(i, -1.0f, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( i+0.2, -1.0f, -1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( i+0.2, -1.0f,  1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(i, -1.0f,  1.0f);
   }
		// Right face
		for(GLfloat i=-1.0; i<0.8; i+=0.4)
   {
       glColor3f(1.0,0.0,0.0);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, i, -1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  i+0.2, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  i+0.2,  1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, i,  1.0f);
   }
   for(GLfloat i=-0.8; i<1.0; i+=0.4)
   {
       glColor3f(1.0,1.0,1.0);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, i, -1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  i+0.2, -1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  i+0.2,  1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, i,  1.0f);
   }
		// Left Face
    for(GLfloat i=-1.0; i<0.8; i+=0.4)
   {
       glColor3f(1.0,0.0,0.0);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, i, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, i,  1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  i+0.2,  1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  i+0.2, -1.0f);
   }
   for(GLfloat i=-0.8; i<1.0; i+=0.4)
   {
       glColor3f(1.0,1.0,1.0);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, i, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, i,  1.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  i+0.2,  1.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  i+0.2, -1.0f);
   }

	glEnd();
   glPopMatrix();
   xrot+=0.06f;
	yrot+=0.04f;
	zrot+=0.08f;
   glutSwapBuffers();
}

void keyboard ( unsigned char key, int x, int y )  // Create Keyboard Function
{
  switch ( key ) {
    case 27:        // When Escape Is Pressed...
      exit ( 0 );   // Exit The Program
      break;        // Ready For Next Case
    default:        // Now Wrap It Up
      break;
  }
}

void arrow_keys ( int a_keys, int x, int y )  // Create Special Function (required for arrow keys)
{
  switch ( a_keys ) {
    case GLUT_KEY_UP:     // When Up Arrow Is Pressed...
      glutFullScreen ( ); // Go Into Full Screen Mode
      break;
    case GLUT_KEY_DOWN:               // When Down Arrow Is Pressed...
      glutReshapeWindow ( 500, 500 ); // Go Into A 500 By 500 Window
      break;
    default:
      break;
  }
}

int main ( int argc, char** argv )   // Create Main Function For Bringing It All Together
{
  glutInit            ( &argc, argv ); // Erm Just Write It =)
  glutInitDisplayMode ( GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA ); // Display Mode
  glutInitWindowPosition (0,0);
  glutInitWindowSize  ( 500, 500 ); // If glutFullScreen wasn't called this is the window size
  glutCreateWindow    ( "Texture Mapping 00000019043 Marcel Cahya Prasetia" ); // Window Title (argv[0] for current directory as title)
  init ();
  glutFullScreen      ( );          // Put Into Full Screen
  glutDisplayFunc     ( display );  // Matching Earlier Functions To Their Counterparts
  glutReshapeFunc     ( reshape );
  glutKeyboardFunc    ( keyboard );
  glutSpecialFunc     ( arrow_keys );
  glutIdleFunc			 ( display );
  glutMainLoop        ( );          // Initialize The Main Loop
  return 0;
}
