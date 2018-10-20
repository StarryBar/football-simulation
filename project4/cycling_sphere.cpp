/************************************************************
 * Handout: rotate-cube-new.cpp (A Sample Code for Shader-Based OpenGL ---
                                 for OpenGL version 3.1 and later)
 * Originally from Ed Angel's textbook "Interactive Computer Graphics" 6th Ed
              sample code "example3.cpp" of Chapter 4.
 * Moodified by Yi-Jen Chiang to include the use of a general rotation function
   Rotate(angle, x, y, z), where the vector (x, y, z) can have length != 1.0,
   and also to include the use of the function NormalMatrix(mv) to return the
   normal matrix (mat3) of a given model-view matrix mv (mat4).

   (The functions Rotate() and NormalMatrix() are added to the file "mat-yjc-new.h"
   by Yi-Jen Chiang, where a new and correct transpose function "transpose1()" and
   other related functions such as inverse(m) for the inverse of 3x3 matrix m are
   also added; see the file "mat-yjc-new.h".)

 * Extensively modified by Yi-Jen Chiang for the program structure and user
   interactions. See the function keyboard() for the keyboard actions.
   Also extensively re-structured by Yi-Jen Chiang to create and use the new
   function drawObj() so that it is easier to draw multiple objects. Now a floor
   and a rotating cube are drawn.

** Perspective view of a color cube using LookAt() and Perspective()

** Colors are assigned to each vertex and then the rasterizer interpolates
   those colors across the triangles.
**************************************************************/

#include "Angel-yjc.h"
#include <stdio.h>
#include <string>
#include <fstream>
#define PI 3.14159265358979323846
using namespace std;


typedef Angel::vec3  color3;
typedef Angel::vec3  point3;
typedef Angel::vec4	 point4;
typedef Angel::vec4	 color4;

GLuint Angel::InitShader(const char* vShaderFile, const char* fShaderFile);


//----------data structure---------------------
class triangle{
public:
	point3 pnt[3];
	color3 clr[3];
	point3 nml[3];
	triangle(){};
	~triangle() {};
};


//--------------functions------------------
char* fname = new char[20];
triangle* file_in(char* nfile);
void pumpkin();
void shadow_init();
void categorize(triangle* tri, point3* point, color3* color);
void calculate_normals(point3* normals, point3* points, int length, int flag);
void swap_normals(point3* normals);//used for flat shading
void particles_init();//used for particles initialization
void SetUp_Lighting_Uniform_Vars();
void drawObj(GLuint buffer, int num_vertices);
bool drawObjLock = 0;//control flow about adding features to the drawObj function
void image_set_up(void);
point3 c_x(point3 a, point3 b);
void demo_menu(int id);
void shadow_menu(int id);
void lighting_menu(int id);
void shading_menu(int id);
void lightsource_menu(int id);
void fogeffect_menu(int id);
void blending_menu(int id);
void firework_menu(int id);
void gmap_menu(int id);
void smap_menu(int id);
//--------------buffer------------------
triangle* tri;
int num_primitive;
point3* pumpkin_pnt;	//used for(x,y,z)
point3* pumpkin_normals;
point3* pumpkin_normals_f;//used for flat shading
color3* pumpkin_clr;
color3* shadow_clr;



GLuint program;       /* shader program object id */
GLuint programfire;
GLuint pumpkin_buffer;/* vertex buffer object id for sphere */
GLuint cube_buffer;   /* vertex buffer object id for cube */
GLuint floor_buffer;  /* vertex buffer object id for floor */
GLuint axis_buffer;
GLuint shadow_buffer;

const int floor_NumVertices = 6; //(1 face)*(2 triangles/face)*(3 vertices/triangle)//
point3 floor_points[floor_NumVertices]; // positions for all vertices
color3 floor_colors[floor_NumVertices]; // colors for all vertices
point3 floor_normals[floor_NumVertices];

const int axis_Num = 9;
point3 axis_points[axis_Num];
color3 axis_colors[axis_Num];


//----------------------parameters------------------------
mat4 ident();
GLfloat radius = 1;
GLfloat dist=0.0;
int flag=1; //1:a->b;  2:b->c;  3:c->a
mat4 acmlt_R = ident();
point3 y = vec3(0.0, 1.0, 0.0);			//find the rotation axis
point3 a=vec3(3.0, 1.0, 5.0);
point3 b=vec3(-1.0, 1.0, -4.0);
point3 c=vec3(3.5, 1.0, -2.5);
point3 ab = normalize(b - a);
point3 bc = normalize(c - b);
point3 ca = normalize(a - c);
GLfloat ab_ = length(b - a);
GLfloat bc_ = length(c - b);
GLfloat ca_ = length(a - c);
point3 Rab = c_x(y, ab);		
point3 Rbc = c_x(y, bc);
point3 Rca = c_x(y, ca);

int jpg = 0, pjpg = 0;
// Projection transformation parameters
GLfloat  fovy = 45.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.5, zFar =100.0;//3

GLfloat angle = 0.0; // rotation angle in degrees
vec4 init_eye(7.0, 3.0, -10.0, 1.0); // initial viewer position
vec4 eye = init_eye;               // current viewer position

//---HW3- Shader Lighting Parameters -----
color4 background_ambient(1.0, 1.0, 1.0, 1.0);
//directional light
color4 light_ambient(0.0, 0.0, 0.0, 1.0);
color4 light_diffuse(0.8, 0.8, 0.8, 1.0);//
color4 light_specular(0.2, 0.2, 0.2, 1.0);//
float const_att = 1.0;
float linear_att = 0;
float quad_att = 0;
point4 light_direction(0.1, 0.0, -0.1, 0.0);////parallel light
point4 POSlight_position(-14.0, 12.0, -3.0, 1.0);//////positional light
point4 spotlight_at(-6.0, 0.0, -4.5, 1.0);////positional light --> at

color4 POSlight_ambient(0.0, 0.0, 0.0, 1.0);
color4 POSlight_diffuse(1.0, 1.0, 1.0, 1.0);
color4 POSlight_specular(1.0, 1.0, 1.0, 1.0);
float POSconst_att = 2.0;
float POSlinear_att = 0.01;
float POSquad_att = 0.001;
float spot_exp = 15.0;
float spot_cutoff = 20 / 180 * PI;



												 //object coefficients
color4 ground_ambient(0.2, 0.2, 0.2, 1.0);
color4 ground_diffuse(0.0, 1.0, 0.0, 1.0);
color4 ground_specular(0.0, 0.0, 0.0, 1.0);
color4 GDambient_product = light_ambient * ground_ambient;
color4 GDdiffuse_product = light_diffuse * ground_diffuse;
color4 GDspecular_product = light_specular * ground_specular;
color4 GDglobal_product = background_ambient*ground_ambient;
color4 POS_GDambient_product = POSlight_ambient * ground_ambient;
color4 POS_GDdiffuse_product = POSlight_diffuse * ground_diffuse;
color4 POS_GDspecular_product = POSlight_specular * ground_specular;


color4 sphere_ambient(0.2, 0.2, 0.2, 1.0);
color4 sphere_diffuse(1.0, 0.84, 0.0, 1.0);
color4 sphere_specular(1.0, 0.84, 0.0, 1.0);
float  sphere_shininess = 125.0;
color4 SPambient_product = light_ambient * sphere_ambient;
color4 SPdiffuse_product = light_diffuse * sphere_diffuse;
color4 SPspecular_product = light_specular * sphere_specular;
color4 SPglobal_product = background_ambient*sphere_ambient;
color4 POS_SPambient_product = POSlight_ambient * sphere_ambient;
color4 POS_SPdiffuse_product = POSlight_diffuse * sphere_diffuse;
color4 POS_SPspecular_product = POSlight_specular * sphere_specular;


//------------HW3 light shadow parameters---------------------------
vec4 light= POSlight_position;
mat4 shadow(									//row as column
	light.y,	0,		 0,			0,
	-light.x,	0,		 -light.z,	-1, 
	0,			0,		 light.y,	0, 
	0,			0,		 0,			light.y);

//------------HW4 fog parameters-------------------------------------
float fog_start = 0.0;
float fog_end = 18.0;
float fog_density = 0.09;
point4 fog_color = point4(0.7, 0.7, 0.7, 0.5);
//-------------texture mapping parameters----------------------------
#define ImageWidth  64
#define ImageHeight 64
GLubyte Image[ImageHeight][ImageWidth][4];

#define	stripeImageWidth 32
GLubyte stripeImage[4 * stripeImageWidth];
vec2 texture_coord[floor_NumVertices] = {
	vec2(1.5,1.25),
	vec2(1.5,0.0),
	vec2(0.0,1.25),
	vec2(0.0,1.25),
	vec2(1.5,0.0),
	vec2(0.0,0.0)
};

//------------HW4 firework parameters--------------------------------
const int particle_Num = 300;
point3 particle_pos=vec3(0.0,0.1,0.0);	//used for(x,y,z)
color3 particle_clr[particle_Num];
point3 particle_spd[particle_Num];
GLuint particle_buffer;
float firework_time = 0.0;
float firework_start = 0.0;
float fmaxTime=9000.0;
float Tstep = 0.0;
//-------------tags--------------------------------
//reserved tag
bool groundEnable = 0;//you can never change it by user,only tag for programmer
//---------------------------------------
bool LatticeEnable = 0;
int LatticeType = 0;//0:for upright;1:for tilted
int textureType = 0;//0:for contour lines;1:for checkboard for sphere
int smapFrameType = 0;//0:for sphere mapping in world frame,1:for sphere mapping in eyeframe.
int smapType = 0;//0:for vertical mapping for sphere;
bool smapEnable = 0;//0:no sphere mapping;1:no ground mapping
bool gmapEnable_cpu = 0; //  0:no ground mapping;1:ground mapping
bool fireEnable = 0;//0:no firework;1:activate firework
bool blendEnable = 0;//0:no shadow blending;1:shadow blending
int fogtype = 0;//0:no fog;1:linear;2:exp;3:exp square
bool shadowEnable = 1;
bool lightEnable = 0;
bool shadingEnable = 1; GLuint noRenderTag;//shadingEnable control the CPU shading;noRenderTag control the GPU shading
vec3 lighttype =vec3( 1, 0, 0 ); //[index]0:directional light;1:spot light;2:point source;[value]0:off;1:on
bool animationLocked = 1;//1:animation locked; 0:animation activated,Toggled by key'b'or'B'
bool animationFlag = 0; // 1: animation; 0: non-animation. Toggled by right mouse button.

int cubeFlag = 0;   // 1: solid cube; 0: wireframe cube. Toggled by key 'c' or 'C'
int floorFlag = 1;  // 1: solid floor; 0: wireframe floor. Toggled by key 'f' or 'F'


//const int cube_NumVertices = 36; //(6 faces)*(2 triangles/face)*(3 vertices/triangle)
#if 0
point3 cube_points[cube_NumVertices]; // positions for all vertices
color3 cube_colors[cube_NumVertices]; // colors for all vertices
point3 cube_points[100]; 
color3 cube_colors[100];
#endif


// Vertices of a unit cube centered at origin, sides aligned with axes
point3 vertices[11] = {
    point3(  5.0,  0.0,  8.0),
    point3(  5.0,  0.0, -4.0),
	point3( -5.0,  0.0,  8.0),
    point3( -5.0,  0.0, -4.0),//floor
    point3(  0.0,  0.0,  0.0),//4
    point3(  1.0,  0.0,  0.0),
    point3(  10.0,  0.0,  0.0),//x-axis
	point3(	 0.0,  1.0,  0.0),
	point3(  0.0,  10.0,  0.0),//y-axis/
	point3(  0.0,  0.0,  1.0),
	point3(  0.0,  0.0,  10.0) //z-axis
};
// RGBA colors
color3 vertex_colors[11] = {
	color3( 0.0, 0.0, 0.0),  //black = =
    color3( 1.0, 0.0, 0.0),  // red
    color3( 1.0, 1.0, 0.0),  // yellow
    color3( 0.0, 1.0, 0.0),  // green
    color3( 0.0, 0.0, 1.0),  // blue
    color3( 1.0, 0.0, 1.0),  // magenta
    color3( 1.0, 1.0, 1.0),  // white
    color3( 0.0, 1.0, 1.0),   // cyan
	color3( 1.0, 0.5, 0.2),  // orange  
	color3( 1.0, 0.84,0.0),	//yellow required
	color3(0.25, 0.25,0.25)	//shadow color	//the fourth parameter'0.64' is defined in vertex shader
};
//----------------------------------------------------------------------------
/*int Index = 0; // YJC: This must be a global variable since quad() is called
               //      multiple times and Index should then go up to 36 for
               //      the 36 vertices and colors

// quad(): generate two triangles for each face and assign colors to the vertices
void quad( int a, int b, int c, int d )//////!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
{
    cube_colors[Index] = vertex_colors[a]; cube_points[Index] = vertices[a]; Index++;
    cube_colors[Index] = vertex_colors[b]; cube_points[Index] = vertices[b]; Index++;
    cube_colors[Index] = vertex_colors[c]; cube_points[Index] = vertices[c]; Index++;

    cube_colors[Index] = vertex_colors[c]; cube_points[Index] = vertices[c]; Index++;
    cube_colors[Index] = vertex_colors[d]; cube_points[Index] = vertices[d]; Index++;
    cube_colors[Index] = vertex_colors[a]; cube_points[Index] = vertices[a]; Index++;
}
//----------------------------------------------------------------------------
// generate 12 triangles: 36 vertices and 36 colors
void colorcube()
{
    quad( 1, 0, 3, 2 );
    quad( 2, 3, 7, 6 );
    quad( 3, 0, 4, 7 );
    quad( 6, 5, 1, 2 );
    quad( 4, 5, 6, 7 );
    quad( 5, 4, 0, 1 );
}*/
//-------------------------------
// generate 2 triangles: 6 vertices and 6 colors
void floor()
{
    floor_colors[0] = vertex_colors[3]; floor_points[0] = vertices[0];
    floor_colors[1] = vertex_colors[3]; floor_points[1] = vertices[2];
    floor_colors[2] = vertex_colors[3]; floor_points[2] = vertices[1];

    floor_colors[3] = vertex_colors[3]; floor_points[3] = vertices[1];
    floor_colors[4] = vertex_colors[3]; floor_points[4] = vertices[2];
    floor_colors[5] = vertex_colors[3]; floor_points[5] = vertices[3];


}
void axis()
{
	axis_colors[0] = vertex_colors[1]; axis_points[0] = vertices[4];
	axis_colors[1] = vertex_colors[1]; axis_points[1] = vertices[5];
	axis_colors[2] = vertex_colors[1]; axis_points[2] = vertices[6];

	axis_colors[3] = vertex_colors[5]; axis_points[3] = vertices[4];
	axis_colors[4] = vertex_colors[5]; axis_points[4] = vertices[7];
	axis_colors[5] = vertex_colors[5]; axis_points[5] = vertices[8];

	axis_colors[6] = vertex_colors[4]; axis_points[6] = vertices[4];
	axis_colors[7] = vertex_colors[4]; axis_points[7] = vertices[9];
	axis_colors[8] = vertex_colors[4]; axis_points[8] = vertices[10];
}
//----------------------------------------------------------------------------
// OpenGL initialization
void pumpkin()
{
	int i;
	for (i = 0; i < num_primitive; i++)
	{
		tri[i].clr[0] = vertex_colors[9];
		tri[i].clr[1] = vertex_colors[9];
		tri[i].clr[2] = vertex_colors[9];
	}
		/*
		tri[jpg].clr[0] = vertex_colors[2];
		tri[jpg].clr[1] = vertex_colors[2];
		tri[jpg].clr[2] = vertex_colors[2];
		tri[pjpg].clr[0] = vertex_colors[8];
		tri[pjpg].clr[1] = vertex_colors[0];
		tri[pjpg].clr[2] = vertex_colors[0];
		pjpg = jpg;
*/
}
void shadow_init()
{
	int i;
	for (i = 0; i < num_primitive*3; i++)
	{
		shadow_clr[i] = vertex_colors[10];
	}
}
void calculate_normals(point3* normals, point3* points, int length, int flag)//flag=1 calculate the normal of ground,flag=2 calculate the smooth normal of sphere,flag=3 calculate the flat normal of sphere
{
	if (flag == 1)
		for (int i = 0; i < length; i++)
			normals[i] = vec3(0.0, 1.0, 0.0);
	if (flag == 2)
		for (int i = 0; i < length; i++)
			normals[i] = points[i];
	if (flag == 3)
		for (int i = 0; i < length / 3; i++)
		{
			normals[3 * i] = normalize(c_x(points[3 * i + 1] - points[3 * i], points[3 * i + 2] - points[3 * i + 1]));
			normals[3 * i + 1] = normalize(c_x(points[3 * i + 1] - points[3 * i], points[3 * i + 2] - points[3 * i + 1]));
			normals[3 * i + 2] = normalize(c_x(points[3 * i + 1] - points[3 * i], points[3 * i + 2] - points[3 * i + 1]));
		}
}
void swap_normals(point3* normals)//used for flat shading
{
	glBindBuffer(GL_ARRAY_BUFFER, pumpkin_buffer);//switch to manuplate the buffer
	glBufferSubData(GL_ARRAY_BUFFER,
		sizeof(point3) * num_primitive * 3 + sizeof(color3) * num_primitive * 3,
		sizeof(point3) * num_primitive * 3,
		normals);
}
void particles_init()
{
	for(int i=0;i<particle_Num;i++)
	{
		particle_clr[i].x = rand() % 256 / 256.0;
		particle_clr[i].y = rand() % 256 / 256.0;
		particle_clr[i].z = rand() % 256 / 256.0;
		particle_spd[i].x = 2.0*(rand() % 256 / 256.0 - 0.5);//m/s
		particle_spd[i].y = 1.2*2.0*(rand() % 256 / 256.0);//m/s
		particle_spd[i].z = 2.0*(rand() % 256 / 256.0 - 0.5);//m/s
	}
}
//------------------------------checkboard generation--------------------------------------
void image_set_up(void)
{
	int i, j, c;

	/* --- Generate checkerboard image to the image array ---*/
	for (i = 0; i < ImageHeight; i++)
		for (j = 0; j < ImageWidth; j++)
		{
			c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0));

			if (c == 1) /* white*/ 
			{
				c = 255;
				Image[i][j][0] = (GLubyte)c;
				Image[i][j][1] = (GLubyte)c;
				Image[i][j][2] = (GLubyte)c;
			}
			else   /* green*/ 
			{
				Image[i][j][0] = (GLubyte)0;
				Image[i][j][1] = (GLubyte)150;
				Image[i][j][2] = (GLubyte)0;
			}

			Image[i][j][3] = (GLubyte)255;
		}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/*--- Generate 1D stripe image to array stripeImage[] ---*/
	for (j = 0; j < stripeImageWidth; j++) {
		/* When j <= 4, the color is (255, 0, 0),   i.e., red stripe/line.
		When j > 4,  the color is (255, 255, 0), i.e., yellow remaining texture*/
		
		stripeImage[4 * j] = (GLubyte)255;
		stripeImage[4 * j + 1] = (GLubyte)((j>4) ? 255 : 0);
		stripeImage[4 * j + 2] = (GLubyte)0;
		stripeImage[4 * j + 3] = (GLubyte)255;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	/*----------- End 1D stripe image ----------------*/

	/*--- texture mapping set-up is to be done in
	init() (set up texture objects),
	display() (activate the texture object to be used, etc.)
	and in shaders.
	---*/

} /* end function */
//-----------------------------init function--------------------------------------------
//-----------------------------------------------------------------------------------------
void init()
{
	image_set_up();
	GLuint texName;
	glGenTextures(1, &texName);
	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ImageWidth, ImageHeight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, Image);

	GLuint tex1d;
	glGenTextures(1, &tex1d);
	glBindTexture(GL_TEXTURE_1D, tex1d);
	//glActiveTexture(GL_TEXTURE1);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, stripeImageWidth,0, GL_RGBA, GL_UNSIGNED_BYTE, stripeImage);

	//colorcube();------------------
	pumpkin_pnt = new point3[num_primitive * 3];
	pumpkin_clr = new color3[num_primitive * 3];
	pumpkin();//init color of sphere
	floor();//init position&color of ground
	categorize(tri, pumpkin_pnt, pumpkin_clr);

	
	shadow_clr = new color3[num_primitive * 3];
	pumpkin_normals = new point3[num_primitive * 3];
	pumpkin_normals_f = new point3[num_primitive * 3];
	calculate_normals(floor_normals, vertices, floor_NumVertices, 1);
	calculate_normals(pumpkin_normals, pumpkin_pnt, num_primitive * 3, 2);
	calculate_normals(pumpkin_normals_f, pumpkin_pnt, num_primitive * 3, 3);
#if 0 //YJC: The following is not needed
    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
#endif
#if 0
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_points) + sizeof(cube_colors),
		 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cube_points), cube_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(cube_points), sizeof(cube_colors),
                    cube_colors);
#endif
#if 1

	// Create and initialize a vertex buffer object for cube, to be used in display()
	glGenBuffers(1, &pumpkin_buffer);			//put CPU memory slots into GPU MEMORY,and name it
	glBindBuffer(GL_ARRAY_BUFFER, pumpkin_buffer);//switch to manuplate the buffer
	glBufferData(GL_ARRAY_BUFFER,
	sizeof(point3)*num_primitive * 3 + sizeof(color3)*num_primitive * 3 + sizeof(point3)*num_primitive * 3 ,
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0,
		sizeof(point3) * num_primitive * 3, pumpkin_pnt);
	glBufferSubData(GL_ARRAY_BUFFER,
		sizeof(point3) * num_primitive * 3,
		sizeof(color3) * num_primitive * 3,
		pumpkin_clr);
	glBufferSubData(GL_ARRAY_BUFFER,
		sizeof(point3) * num_primitive * 3 + sizeof(color3) * num_primitive * 3,
		sizeof(point3) * num_primitive * 3,
		pumpkin_normals);
#endif

	////!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// Create and initialize a vertex buffer object for floor, to be used in display()
	glGenBuffers(1, &floor_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors) + sizeof(floor_normals)+sizeof(texture_coord),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points), floor_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points), sizeof(floor_colors),
		floor_colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors), sizeof(floor_normals),
		floor_normals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors)+sizeof(floor_normals),sizeof(texture_coord),
		texture_coord);

	axis();
	// Create and initialize a vertex buffer object for axis, to be used in display()
	glGenBuffers(1, &axis_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, axis_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axis_points) + sizeof(axis_colors),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(axis_points), axis_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(axis_points), sizeof(axis_colors),
		axis_colors);

	shadow_init();
	glGenBuffers(1, &shadow_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, shadow_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point3)*num_primitive * 3 + sizeof(color3)*num_primitive * 3,
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0,
		sizeof(point3) * num_primitive * 3, pumpkin_pnt);
	glBufferSubData(GL_ARRAY_BUFFER,
		sizeof(point3) * num_primitive * 3,
		sizeof(color3) * num_primitive * 3,
		shadow_clr);

 // Load shaders and create a shader program (to be used in display())
    program = InitShader("vshader42.glsl", "fshader42.glsl");

	//particles:------------------
	particles_init();

	glGenBuffers(1, &particle_buffer);//assign the slots name to var:ptcl_buffer
	glBindBuffer(GL_ARRAY_BUFFER, particle_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(particle_clr) + sizeof(particle_spd), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(particle_clr), particle_clr);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(particle_clr), sizeof(particle_spd), particle_spd);

	
	programfire = InitShader("vshaderfire.glsl", "fshaderfire.glsl");
	
    glEnable( GL_DEPTH_TEST );
    glClearColor(0.529, 0.807, 0.92, 0.0);
    glLineWidth(2.0);
}
//----------------------------------------------------------------------------
// drawObj(buffer, num_vertices):
//   draw the object that is associated with the vertex buffer object "buffer"
//   and has "num_vertices" vertices.
//
void drawObj(GLuint buffer, int num_vertices)//CPU send method of tranversing throughout the GPU memory slots,then GPU perform computation&drawing
{
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
	
	//draw firework
	if (drawObjLock)//controlled by fireEnable=1
	{
		glBindBuffer(GL_ARRAY_BUFFER, particle_buffer);
		GLuint particle_clrtag = glGetAttribLocation(programfire, "particle_clr");
		glEnableVertexAttribArray(particle_clrtag);
		glVertexAttribPointer(particle_clrtag, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
									//every time read 3 coordinate of a single array
		GLuint particle_spdtag = glGetAttribLocation(programfire, "particle_spd");
		glEnableVertexAttribArray(particle_spdtag);
		glVertexAttribPointer(particle_spdtag, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(particle_clr)));

		glDrawArrays(GL_POINTS, 0, num_vertices);
		
		glDisableVertexAttribArray(particle_clrtag);
		glDisableVertexAttribArray(particle_spdtag);
	
	}
	else
		/*----- Set up vertex attribute arrays for each vertex attribute -----*/
	{
		GLuint vPosition = glGetAttribLocation(program, "vPosition");//why is it always GLuint not GLfloat???
		glEnableVertexAttribArray(vPosition);
		glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(0));

		GLuint vColor = glGetAttribLocation(program, "vColor");
		glEnableVertexAttribArray(vColor);
		glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(sizeof(point3) * num_vertices));
		
		GLuint vNormal = glGetAttribLocation(program, "vNormal");
			glEnableVertexAttribArray(vNormal);
			glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0,
				BUFFER_OFFSET(sizeof(point3) * num_vertices + sizeof(color3) * num_vertices));

		GLuint vTexCoord;
		if (groundEnable)//ground control flow
		{
			vTexCoord = glGetAttribLocation(program, "vTexCoord");
			glEnableVertexAttribArray(vTexCoord);
			glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
				BUFFER_OFFSET(sizeof(point3) * num_vertices + sizeof(color3) * num_vertices + sizeof(point3)*num_vertices));
		}// the offset is the (total) size of the previous vertex attribute array(s)
		
	  /* Draw a sequence of geometric objs (triangles) from the vertex buffer
		 (using the attributes specified in each enabled vertex attribute array) */
		glDrawArrays(GL_TRIANGLES, 0, num_vertices);

		/*--- Disable each vertex attribute array being enabled ---*/
		glDisableVertexAttribArray(vPosition);//palse GPU reading
		glDisableVertexAttribArray(vColor);
		glDisableVertexAttribArray(vNormal);
		if(groundEnable)  
			glDisableVertexAttribArray(vTexCoord);//ground control flow*/
	}

}
//----------------------------------------------------------------------------
void SetUp_Lighting_Uniform_Vars()
{

	// The Light direction in Eye Frame
	glUniform4fv(glGetUniformLocation(program, "LightDirection"),
		1, light_direction);

	glUniform1f(glGetUniformLocation(program, "SpotExp"),
		spot_exp);
	glUniform1f(glGetUniformLocation(program, "SpotCutoff"),
		spot_cutoff);

	glUniform1f(glGetUniformLocation(program, "ConstAtt"),
		const_att);
	glUniform1f(glGetUniformLocation(program, "LinearAtt"),
		linear_att);
	glUniform1f(glGetUniformLocation(program, "QuadAtt"),
		quad_att);

	glUniform1f(glGetUniformLocation(program, "POSConstAtt"),
		POSconst_att);
	glUniform1f(glGetUniformLocation(program, "POSLinearAtt"),
		POSlinear_att);
	glUniform1f(glGetUniformLocation(program, "POSQuadAtt"),
		POSquad_att);

	glUniform1f(glGetUniformLocation(program, "Shininess"),
		sphere_shininess);

	//---------FOG parameters------------
	
	glUniform1i(glGetUniformLocation(program, "fogtype"),
		fogtype);
	glUniform1f(glGetUniformLocation(program, "fog_start"),
		fog_start);
	glUniform1f(glGetUniformLocation(program, "fog_end"),
		fog_end);
	glUniform1f(glGetUniformLocation(program, "fog_density"),
		fog_density);
	glUniform4fv(glGetUniformLocation(program, "fog_color"),
		1,fog_color);

}
void display( void )
{

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glUseProgram(program); // Use the shader program

	GLuint  model_view = glGetUniformLocation(program, "model_view" );// model-view matrix uniform shader variable location
	GLuint  projection = glGetUniformLocation(program, "projection" );// projection matrix uniform shader variable location

	//--parallel light
	GLuint AmbientProduct = glGetUniformLocation(program, "AmbientProduct");
	GLuint DiffuseProduct = glGetUniformLocation(program, "DiffuseProduct");
	GLuint SpecularProduct = glGetUniformLocation(program, "SpecularProduct");
	//--global light
	GLuint GlobalProduct = glGetUniformLocation(program, "GlobalProduct");
	//--positional light
	GLuint PosAmbientProduct = glGetUniformLocation(program, "PosAmbientProduct");
	GLuint PosDiffuseProduct = glGetUniformLocation(program, "PosDiffuseProduct");
	GLuint PosSpecularProduct = glGetUniformLocation(program, "PosSpecularProduct");
	
	GLfloat shadow4 = glGetUniformLocation(program, "shadowflag");//pass into the vshader,to determin the 4-th parameter.
	glUniform1f(shadow4, 1.0);
	
	noRenderTag = glGetUniformLocation(program,"noRenderTag");//pass noRenderTag

	GLuint stillObj = glGetUniformLocation(program, "stillObj");
	glUniform1i(stillObj, 0);
	
	GLuint groundEnable_cpu = glGetUniformLocation(program, "groundEnable");
	glUniform1i(groundEnable_cpu, groundEnable);
	
	GLuint gmapEnable = glGetUniformLocation(program, "gmapEnable");
	glUniform1i(gmapEnable, gmapEnable_cpu);

	glUniform3fv(glGetUniformLocation(program, "LightType"),
	1, lighttype);
	GLuint cubeEnable_cpu= glGetUniformLocation(program, "cubeEnable");

	GLuint smapType_cpu = glGetUniformLocation(program, "smapType");
	glUniform1i(smapType_cpu, smapType);
	
	GLuint smapFrameType_cpu = glGetUniformLocation(program, "smapFrameType");
	glUniform1i(smapFrameType_cpu, smapFrameType);

	GLuint smapTxturType_cpu = glGetUniformLocation(program, "textureType");
	glUniform1i(smapTxturType_cpu, textureType);

	GLuint LatticeType_cpu = glGetUniformLocation(program, "latticeType");
	glUniform1i(LatticeType_cpu, LatticeType);

	GLuint LatticeEnable_cpu = glGetUniformLocation(program, "latticeEnable");
	glUniform1i(LatticeEnable_cpu, LatticeEnable);
	
	GLuint ShadowEnable_gpu = glGetUniformLocation(program, "shadowEnable");
	glUniform1i(ShadowEnable_gpu, 0);
	//glUniform1i(glGetUniformLocation(program, "Gtexture_2D"), 0);///pass '0 ' into it;stand for GL_TEXTURE0,GROUND
	//glUniform1i(glGetUniformLocation(program, "Gtexture_1D"), 1);///pass '0 ' into it;stand for GL_TEXTURE1

	SetUp_Lighting_Uniform_Vars();//put all other uniforms there


/*---  Set up and pass on Projection matrix to the shader ---*/
    mat4  p = Perspective(fovy, aspect, zNear, zFar);
    glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

/*---  Set up and pass on Model-View matrix to the shader ---*/
    // eye is a global variable of vec4 set to init_eye and updated by keyboard()
    vec4    at(0.0, 0.0, 0.0, 1.0);
    vec4    up(0.0, 1.0, 0.0, 0.0);

	mat4  mv = LookAt(eye, at, up);
	mat4  mv_shadow;

	point4 POSlight_position_e = mv * POSlight_position;
	glUniform4fv(glGetUniformLocation(program, "PosLightPosition_e"),
		1, POSlight_position_e);
	point4 spotlight_at_e = mv * spotlight_at;
	glUniform4fv(glGetUniformLocation(program, "SpotlightAt"),
		1, spotlight_at_e);

/*----- Set Up the Model-View matrix for the cube -----*/
#if 0 // The following is to verify the correctness of the function NormalMatrix():
      // Commenting out Rotate() and un-commenting mat4WithUpperLeftMat3() 
      // gives the same result.
      mv = mv * Translate(0.0, 0.5, 0.0) * Scale (1.4, 1.4, 1.4) 
              * Rotate(angle, 0.0, 0.0, 2.0); 
           // * mat4WithUpperLeftMat3(NormalMatrix(Rotate(angle, 0.0, 0.0, 2.0), 1));
#endif
#if 1 // The following is to verify that Rotate() about (0,2,0) is RotateY():
      // Commenting out Rotate() and un-commenting RotateY()
      // gives the same result.
  //
  // The set-up below gives a new scene (scene 2), using Correct LookAt().
	  dist = angle  * 2 * PI * radius / 360;

	  switch (flag)
	  {
	  case 1:
		  mv = LookAt(eye, at, up) * Translate(a + ab*dist) * Rotate(angle, Rab.x, Rab.y, Rab.z) * acmlt_R;
		  mv_shadow = LookAt(eye, at, up) * shadow * Translate(a + ab*dist) * Rotate(angle, Rab.x, Rab.y, Rab.z) * acmlt_R;
		  if (dist >= ab_)
		  {
			  acmlt_R = Rotate(angle, Rab.x, Rab.y, Rab.z)*acmlt_R;
			  flag = 2;
			  angle = 0.0;
		  }
		  break;
	  case 2:
		  mv = LookAt(eye, at, up) * Translate(b + bc*dist) * Rotate(angle, Rbc.x, Rbc.y, Rbc.z) * acmlt_R;
		  mv_shadow = LookAt(eye, at, up) * shadow * Translate(b + bc*dist) * Rotate(angle, Rbc.x, Rbc.y, Rbc.z) * acmlt_R;
		  if (dist >= bc_) 
		  { 
			  acmlt_R = Rotate(angle, Rbc.x, Rbc.y, Rbc.z)*acmlt_R;
			  flag = 3; 
			  angle = 0.0;
		  }
		  break;
	  case 3:
		  mv = LookAt(eye, at, up) * Translate(c + ca*dist) * Rotate(angle, Rca.x, Rca.y, Rca.z) * acmlt_R;
		  mv_shadow = LookAt(eye, at, up) * shadow * Translate(c + ca*dist) * Rotate(angle, Rca.x, Rca.y, Rca.z) * acmlt_R;
		  if (dist >= ca_)
		  { 
			  acmlt_R = Rotate(angle, Rca.x, Rca.y, Rca.z)*acmlt_R;
			  flag = 1; 
			  angle = 0.0;
		  }
		  break;
	  }


	   // * RotateY(angle);
  //
  // The set-up below gives the original scene (scene 1), using Correct LookAt().
  //  mv = Translate(0.0, 0.5, 0.0) * mv * Scale (1.4, 1.4, 1.4) 
  //               * Rotate(angle, 0.0, 2.0, 0.0);
	        // * RotateY(angle); 
  //
  // The set-up below gives the original scene (scene 1), when using previously 
  //     Incorrect LookAt() (= Translate(1.0, 1.0, 0.0) * correct LookAt() )
  //  mv = Translate(-1.0, -0.5, 0.0) * mv * Scale (1.4, 1.4, 1.4) 
  //               * Rotate(angle, 0.0, 2.0, 0.0);
	        // * RotateY(angle);
  //
#endif
#if 0  // The following is to verify that Rotate() about (3,0,0) is RotateX():
       // Commenting out Rotate() and un-commenting RotateX()
       // gives the same result.
      mv = mv * Translate(0.0, 0.5, 0.0) * Scale (1.4, 1.4, 1.4)
                    * Rotate(angle, 3.0, 0.0, 0.0);
                 // * RotateX(angle);
#endif

	//sending normal matrix
	auto normalMatrix = glGetUniformLocation(program, "Normal_Matrix");//after binding,we cannot changing the value by assignment,but can by function
	glUniformMatrix3fv(normalMatrix, 1, GL_TRUE, NormalMatrix(mv, 1));//mat3 normalMatrix = NormalMatrix(mv, 1);


	//send product for sphere
	glUniform4fv(AmbientProduct,1, SPambient_product);
	glUniform4fv(DiffuseProduct, 1, SPdiffuse_product);
	glUniform4fv(SpecularProduct, 1, SPspecular_product);
	glUniform4fv(GlobalProduct, 1, SPglobal_product);
	glUniform4fv(PosAmbientProduct, 1, POS_SPambient_product);
	glUniform4fv(PosDiffuseProduct, 1, POS_SPdiffuse_product);
	glUniform4fv(PosSpecularProduct, 1, POS_SPspecular_product);



	glUniform1i(noRenderTag, !lightEnable);//read the lighting condition,to decide whether to render the object

	//sending general sphere matrix	
	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
    if (cubeFlag == 1) // Filled cube
       glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else              // Wireframe cube
       glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	GLuint cubeFlag_cpu = glGetUniformLocation(program, "cubeFilled");
	glUniform1i(cubeFlag_cpu,cubeFlag);//dynamic cube,satisfy the condition then calculate once for one subprogram.
	if (smapEnable) { glUniform1i(cubeEnable_cpu, 1); }//static cube.
    drawObj(pumpkin_buffer, num_primitive*3);  // draw the sphere
	if (smapEnable) { glUniform1i(cubeEnable_cpu, 0); }
	glUniform1i(cubeFlag_cpu, 1);
	

/*----- Set up the Mode-View matrix for the floor -----*/
// The set-up below gives a new scene (scene 2), using Correct LookAt() function


	//sending eyeframe lightsource
	//
	// The set-up below gives the original scene (scene 1), using Correct LookAt()
	//    mv = Translate(0.0, 0.0, 0.3) * LookAt(eye, at, up) * Scale (1.6, 1.5, 3.3);
	//
	// The set-up below gives the original scene (scene 1), when using previously 
	//       Incorrect LookAt() (= Translate(1.0, 1.0, 0.0) * correct LookAt() ) 
	//    mv = Translate(-1.0, -1.0, 0.3) * LookAt(eye, at, up) * Scale (1.6, 1.5, 3.3);
	
	//send product for floor
	glUniform4fv(AmbientProduct, 1, GDambient_product);
	glUniform4fv(DiffuseProduct, 1, GDdiffuse_product);
	glUniform4fv(SpecularProduct, 1, GDspecular_product);
	glUniform4fv(GlobalProduct, 1, GDglobal_product);
	glUniform4fv(PosAmbientProduct, 1, POS_GDambient_product);
	glUniform4fv(PosDiffuseProduct, 1, POS_GDdiffuse_product);
	glUniform4fv(PosSpecularProduct, 1, POS_GDspecular_product);

//--------------------making a decal-----------------------------------------
	if (blendEnable)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	glDepthMask(GL_FALSE);						//disable writing to z buffer------------------

	//draw the ground
	mv = LookAt(eye, at, up);
	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
	if (floorFlag == 1) // Filled floor
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else              // Wireframe floor
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glUniform1i(stillObj, 1);//control not moving
	groundEnable = 1; glUniform1i(groundEnable_cpu, groundEnable);//control  DIY ground buffer with drawObj.
	drawObj(floor_buffer, floor_NumVertices);  // draw the ground
	groundEnable = 0; glUniform1i(groundEnable_cpu, groundEnable);
	glUniform1i(stillObj, 0);

		if (!blendEnable) //recover writing to z buffer11111---------------------------------------
		{ 
			glDepthMask(GL_TRUE);
		}
	
	//draw the shadow
	if (shadowEnable && eye.y >= 0)
	{
		
		glUniformMatrix4fv(model_view, 1, GL_TRUE, mv_shadow); // GL_TRUE: matrix is row-major
		if (cubeFlag == 1) // Filled cube
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else              // Wireframe cube
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		
		glUniform1i(noRenderTag, 1);
		glUniform1i(ShadowEnable_gpu, 1);
		glUniform1f(shadow4, 0.64);					//control the 4th element of a color in vshader
		drawObj(shadow_buffer, num_primitive * 3);  // draw the shadow
		glUniform1f(shadow4, 1.0);
		glUniform1i(ShadowEnable_gpu, 0);
		glUniform1i(noRenderTag, !lightEnable);
	
	}

	if (blendEnable) //recover writing to z buffer22222---------------------------------------
	{
		glDepthMask(GL_TRUE); 
	}

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);	//disable writing to frame buffer------------------
	
	//draw the ground
	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
    if (floorFlag == 1) // Filled floor
       glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else              // Wireframe floor
       glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glUniform1i(stillObj, 1);
	groundEnable = 1; glUniform1i(groundEnable_cpu, groundEnable);//control  DIY ground buffer with drawObj.-texture mapping,not controllable by the user
    drawObj(floor_buffer, floor_NumVertices);  // draw the floor
	groundEnable = 0; glUniform1i(groundEnable_cpu, groundEnable);//control  DIY ground buffer with drawObj.
	glUniform1i(stillObj, 0);

	//draw the shadow
	if (blendEnable)
	{
		if (shadowEnable && eye.y >= 0)
		{
			glUniformMatrix4fv(model_view, 1, GL_TRUE, mv_shadow); // GL_TRUE: matrix is row-major
			if (cubeFlag == 1) // Filled cube
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			else              // Wireframe cube
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		
			glUniform1i(noRenderTag, 1);
			glUniform1i(ShadowEnable_gpu, 1);
			glUniform1f(shadow4, 0.64);					//control the 4th element of a color in vshader
			drawObj(shadow_buffer, num_primitive * 3);  // draw the shadow
			glUniform1f(shadow4, 1.0);
			glUniform1i(ShadowEnable_gpu, 0);
			glUniform1i(noRenderTag, !lightEnable);
			
		}
	glDisable(GL_BLEND);
	}

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);//recover writing to frame buffer----------------
//--------------------making a decal--------up----up-----up------------------------
	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // !!if skip this sentence,sth. interesting happens
	glUniform1i(noRenderTag, 1);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	drawObj(axis_buffer, axis_Num);  // draw the axis
	glUniform1i(noRenderTag, !lightEnable);

//-------------using another shader to draw firework
	if (fireEnable)
	{
		glUseProgram(programfire); // Use the shader program
		GLuint  model_viewF = glGetUniformLocation(programfire, "model_view");// model-view matrix uniform shader variable location
		GLuint  projectionF = glGetUniformLocation(programfire, "projection");// projection matrix uniform shader variable location
		p = Perspective(fovy, aspect, zNear, zFar);
		glUniformMatrix4fv(projectionF, 1, GL_TRUE, p);//used for binding uniform to GPU in advance.
		mv = LookAt(eye, at, up);
		glUniformMatrix4fv(model_viewF, 1, GL_TRUE, mv);
		//------firework parameters--------------
		glUniform3fv(glGetUniformLocation(programfire, "particle_pos"),1, particle_pos);
		if(firework_start==0) firework_start = glutGet(GLUT_ELAPSED_TIME);
		if (firework_time > fmaxTime)//perform similar as ''mod'' function
			firework_start = glutGet(GLUT_ELAPSED_TIME); //firework_time = 0;

		float t = glutGet(GLUT_ELAPSED_TIME);
		firework_time = t - firework_start;
		
		glUniform1f(glGetUniformLocation(programfire, "T"), firework_time);

		glPointSize(3.0);
		drawObjLock = 1;
		drawObj(particle_buffer, particle_Num);//used for allocate vertex attribute pointer& allow GPU reading sth.
		drawObjLock = 0;
	}
//===========draw axis================================



    glutSwapBuffers();
}
//---------------------------------------------------------------------------
void idle (void)
{
    //angle += 0.02;
    angle += 0.0625;    //YJC: change this value to adjust the cube rotation speed.
	//firework_time += 1.0;
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
    switch(key) {
	case 033: // Escape Key
	case 'q': case 'Q':
	    exit( EXIT_SUCCESS );
	    break;

        case 'X': eye[0] += 1.0; break;
	case 'x': eye[0] -= 1.0; break;
        case 'Y': eye[1] += 1.0; break;
	case 'y': eye[1] -= 1.0; break;
        case 'Z': eye[2] += 1.0; break;
	case 'z': eye[2] -= 1.0; break;

        case 'b': case 'B': // Toggle between animation and non-animation
			animationLocked = 0;// -animationFlag;
			glutIdleFunc(idle);
			animationFlag = 1;
            break;
	   
        case 'c': case 'C': // Toggle between filled and wireframe cube
	    cubeFlag = 1 -  cubeFlag;   
            break;

        case 'f': case 'F': // Toggle between filled and wireframe floor
	    floorFlag = 1 -  floorFlag; 
            break;

		case 'v': case 'V'://verticle 1D/2D texture
			smapType = 0;
			if (LatticeType == 1) LatticeType = 0;
			break;

		case 's': case 'S'://slanted 1D/2D texture
			smapType = 1;
			if (LatticeType == 0) LatticeType = 1;
			break;

		case 'o': case 'O':
			smapFrameType = 0;//world frame
			break;

		case 'e': case 'E':
			smapFrameType = 1;//eye frame
			break;

		case 'u': case 'U'://upright 1D/2D texture hole
			if(LatticeEnable)
			{
				LatticeType = 0;
				if (smapType == 1) smapType = 0;//align smapType with Latticetype
			}
			break;

		case 't': case 'T'://tilted 1D/2D texture hole
			if (LatticeEnable)
			{
				LatticeType = 1;
				if (smapType == 0) smapType = 1;
			}
			break;

		case 'l': case 'L'://tilted 1D/2D texture hole
			LatticeEnable = 1- LatticeEnable;
			break;
			
//		case 'i':jpg++;    init();	break;
//		case'I':jpg--;  init();	break;
//		case ' ':  // reset to initial viewer/eye position
//			 eye = init_eye;
//			 break;
    }
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void myMouse(int button, int state, int x, int y)
{
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
		if (!animationLocked) 
		{
			animationFlag = 1-animationFlag;
			if (animationFlag)	glutIdleFunc(idle);
			else				glutIdleFunc(NULL);
		}
	//if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	//{

	//}
	
}
//----------------------------------------------------------------------------
void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    aspect = (GLfloat) width  / (GLfloat) height;
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
int main(int argc, char **argv)
{ int err;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(512, 512);
    // glutInitContextVersion(3, 2);
    // glutInitContextProfile(GLUT_CORE_PROFILE);
    glutCreateWindow("cycling sphere");
	//interactive input!!
	cout << "show me the input file below:  (eg. draw8.txt)" << endl;
	cin >> fname;
	tri=file_in(fname);

  /* Call glewInit() and error checking */
  err = glewInit();
  if (GLEW_OK != err)
  { printf("Error: glewInit failed: %s\n", (char*) glewGetErrorString(err)); 
    exit(1);
  }
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(NULL);
    glutKeyboardFunc(keyboard);
	glutMouseFunc(myMouse);
	
	auto shadow_id = glutCreateMenu(shadow_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes", 2);
	auto lighting_id = glutCreateMenu(lighting_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes", 2);
	auto shading_id = glutCreateMenu(shading_menu);
	glutAddMenuEntry("Flat Shading", 1);
	glutAddMenuEntry("Smooth Shading", 2);
	auto lightsource_id = glutCreateMenu(lightsource_menu);
	glutAddMenuEntry("spot light", 1);
	glutAddMenuEntry("point source", 2);
	glutAddMenuEntry("directional light", 3);
	glutAddMenuEntry("no light", 4);
	auto fogeffect_id = glutCreateMenu(fogeffect_menu);
	glutAddMenuEntry("no fog", 1);
	glutAddMenuEntry("linear", 2);
	glutAddMenuEntry("exponential", 3);
	glutAddMenuEntry("exponential square", 4);
	auto blending_id = glutCreateMenu(blending_menu);
	glutAddMenuEntry("NO", 1);
	glutAddMenuEntry("YES", 2);
	auto firework_id = glutCreateMenu(firework_menu);
	glutAddMenuEntry("NO", 1);
	glutAddMenuEntry("YES", 2);
	auto gmap_id = glutCreateMenu(gmap_menu);
	glutAddMenuEntry("NO", 1);
	glutAddMenuEntry("YES", 2);
	auto smap_id = glutCreateMenu(smap_menu);
	glutAddMenuEntry("NO", 1);
	glutAddMenuEntry("YES-Contour Lines", 2);
	glutAddMenuEntry("YES-Checkboard", 3);

	glutCreateMenu(demo_menu); 
	glutAddMenuEntry("Quit", 1);
	glutAddMenuEntry("Default View Point", 2);
	glutAddSubMenu("Shadow", shadow_id);
	glutAddSubMenu("Enable Lighting",lighting_id);
	glutAddMenuEntry("Wire Frame Sphere",3);
	glutAddSubMenu("Shading",shading_id);
	glutAddSubMenu("Light Source", lightsource_id);
	glutAddSubMenu("Fog Options", fogeffect_id);
	glutAddSubMenu("Blending Shadow", blending_id);
	glutAddSubMenu("Firework", firework_id);
	glutAddSubMenu("Texture Mapped Ground", gmap_id);
	glutAddSubMenu("Texture Mapped Sphere", smap_id);
	glutAttachMenu(GLUT_LEFT_BUTTON);

    init();
    glutMainLoop();

	delete[] fname;
	delete[] tri;
	delete[] pumpkin_pnt;//dynamically allocated
	delete[] pumpkin_clr;
	delete[] shadow_clr;
	delete[] pumpkin_normals;
	delete[] pumpkin_normals_f;
    return 0;
	
}
//--------------------------MENU-----------------------------------------------------------
//------------------------------------------------------------------------------------------
void demo_menu(int id)
{
	switch (id)
	{
	case 1: exit(0); break;
	case 2: eye = init_eye; animationFlag = 1; glutIdleFunc(idle); break;
	case 3: cubeFlag = 0; break;	//wire frame sphere
	}
	
	if (!animationLocked&&animationFlag)
		glutIdleFunc(idle);
	glutPostRedisplay();
}
void shadow_menu(int id)
{
	switch (id)
	{
	case 1:shadowEnable = 0; break;
	case 2:shadowEnable = 1; break;
	}
	
	if (!animationLocked&&animationFlag)
		glutIdleFunc(idle);
	glutPostRedisplay();
}
void lighting_menu(int id)
{
	switch (id)
	{
	case 1:lightEnable = 0; break;
	case 2:lightEnable = 1; break;
	}
	
	if (!animationLocked&&animationFlag)
		glutIdleFunc(idle);
	glutPostRedisplay();
}
void shading_menu(int id)
{
	switch (id)
	{
	case 1:shadingEnable = 1; cubeFlag = 1; swap_normals(pumpkin_normals_f); break;
	case 2:shadingEnable = 1; cubeFlag = 1; swap_normals(pumpkin_normals); break;
	}
	
	if (!animationLocked&&animationFlag)
		glutIdleFunc(idle);
	glutPostRedisplay();
}
void lightsource_menu(int id)
{
	switch (id)
	{
	case 1: lighttype[1] = 1; lighttype[2] = 0; break;//spot
	case 2: lighttype[2] = 1; lighttype[1] = 0; break;//point
	case 3: lighttype[0] = 1; break;//directional 
	case 4: lighttype[0] = 0; lighttype[1] = 0; lighttype[2] = 0; break;//no position light
	}
	
	if (!animationLocked&&animationFlag)
		glutIdleFunc(idle);
	glutPostRedisplay();
}
void fogeffect_menu(int id)
{
	switch (id)
	{
	case 1: fogtype = 0; break;//no
	case 2: fogtype = 1; break;//linear
	case 3: fogtype = 2; break;//exp 
	case 4: fogtype = 3; break;//expsquare
	}

	if (!animationLocked&&animationFlag)
		glutIdleFunc(idle);
	glutPostRedisplay();
}
void blending_menu(int id)
{
	switch (id)
	{
	case 1: blendEnable = 0; break;
	case 2: blendEnable = 1; break;
	}

	if (!animationLocked&&animationFlag)
		glutIdleFunc(idle);
	glutPostRedisplay();
}
void firework_menu(int id)
{
	switch (id)
	{
	case 1: fireEnable = 0; break;
	case 2: fireEnable = 1;  break;
	}

	if (!animationLocked&&animationFlag)
		glutIdleFunc(idle);
	glutPostRedisplay();
}
void gmap_menu(int id) {
	switch (id)
	{
	case 1: gmapEnable_cpu = 0; break;
	case 2: gmapEnable_cpu = 1;  break;
	}

	if (!animationLocked&&animationFlag)
		glutIdleFunc(idle);
	glutPostRedisplay();
}
void smap_menu(int id) 
{
	switch (id)
	{
	case 1: smapEnable = 0; break;
	case 2: smapEnable = 1; textureType = 0; break;
	case 3: smapEnable = 1; textureType = 1; break;
	}

	if (!animationLocked&&animationFlag)
		glutIdleFunc(idle);
	glutPostRedisplay();
}
//--------------------------file IO----------------------------------
//---------------------------------------------------------------------
triangle* file_in(char* nfile)		//c++ file in function
{
	int i = 0;
	string s[9];
	ifstream fp(nfile);

	if (fp.fail()) { cout << "input error" << endl; exit(1); }
	
	fp >> num_primitive;
	fp.get();
	fp.get();
	triangle* tri = new triangle[num_primitive];
		while (!fp.eof()) 
	{
		fp.get();
		fp >> s[0] >> s[1] >> s[2] >> s[3] >> s[4] >> s[5] >> s[6] >> s[7] >> s[8];
		tri[i].pnt[0].x = atof(s[0].c_str());		tri[i].pnt[0].y = atof(s[1].c_str());		tri[i].pnt[0].z = atof(s[2].c_str());
		tri[i].pnt[1].x = atof(s[3].c_str());		tri[i].pnt[1].y = atof(s[4].c_str());		tri[i].pnt[1].z = atof(s[5].c_str());
		tri[i].pnt[2].x = atof(s[6].c_str());		tri[i].pnt[2].y = atof(s[7].c_str());		tri[i].pnt[2].z = atof(s[8].c_str());
		i++;
		fp.get();
	}
	fp.close();
	return tri;
}
void categorize(triangle* tri, point3* point, color3* color)	//categorize the structure into two buffer 
{
	int i;
	for (i = 0; i < num_primitive; i++)
	{
		point[3 * i] = tri[i].pnt[0];
		point[3 * i + 1] = tri[i].pnt[1]  ;
		point[3 * i + 2] = tri[i].pnt[2] ;
		color[3 * i] = tri[i].clr[0];
		color[3 * i + 1] = tri[i].clr[1];
		color[3 * i + 2] = tri[i].clr[2];
	}
}

//--------------------self designed funcition--------------------------------------
//------------------------------------------------------------------------------------
point3 c_x(point3 a, point3 b)	//the cross product
{
	return point3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}

mat4 ident()		//making a identical matrix
{
	mat4 c;
	for (int i = 0; i<4; i++) for (int j = 0; j<4; j++) c[i][j] = 0.0;
	for (int i = 0; i<4; i++) c[i][i] = 1.0;
	return c;
}
//-----------------------------------------------------------------------------------------

