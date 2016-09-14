//Includes vec, mat, and other include files as well as macro defs
#define GL3_PROTOTYPES

// Include the vector and matrix utilities from the textbook, as well as some
// macro definitions.
#include "Angel.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string.h>
const double PI = 3.14159;

#ifdef __APPLE__
#  include <OpenGL/gl3.h>
#endif

#define SQUAREVV


#ifdef SQUAREVV
int ww=500, wh  = 500;
float vl = -1.0, vr = 1.0, vb = -1.0, vt = 1.0;
float startAR = (vr-vl)/(vt-vb);
#endif


#ifdef NONSQUAREVV
int ww=500, wh=250;
float vl = -10.0, vr=10.0, vt=5.0, vb=-5.0;
float startAR = (vr-vl)/(vt-vb);
#endif

#ifdef NONSYMVV
int ww = 500, wh = 250;
float vl=-5, vr = 15, vt=5.0, vb=-5.0;
#endif

using namespace std;

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;
const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)
GLuint program;

#define NO_PROMPT

#ifdef NO_PROMPT
std::string projType = "orthographic";
float eyex = -1.5;
float eyey = -1.5;
float eyez = -1.5;
float atx = 0;
float aty = 0;
float atz = 0;
float upx = 0;
float upy = 1;
float upz = 0;
float yfov = 65;
float aspRatio = 1;
float near = 1;
float far = 4;
float o_left = -.5;
float o_right = .5;
float o_bottom = -.5;
float o_top = .5;
#endif

#ifdef PROMPT
std::string projType = "";
float eyex, eyey, eyez, atx, aty, atz, upx, upy, upz;
float near, far;
float yfov, aspRatio;
float o_left, o_right, o_bottom, o_top;
#endif

const bool VERBOSE = true;
std::vector < struct object > objs;
int current_height = 0;
int manip_num_vert = 54;
int ref_floor_vert = 6;
const float LINE_WIDTH = 0.04f;

int clicked_object = 0;
std::string axis_to_drag = "";
std::string trans_mode = "t";   //default to translate mode
int init_x_pos = 0;
int init_y_pos = 0;
float prev_distance = 0.0f;

//sensitivity setting (should set relative to obj vertex dimensions)
const float t_sensitivity = 100;  //higher = slower
const float r_sensitivity = 1;  //[0 - 1) for slower rotation, [1 - 5...] for faster rotation
const float s_sensitivity = 100;  //higher = scales slower


//views
GLuint  model_view;  // model-view matrix uniform shader variable location
GLuint  projection; // projection matrix uniform shader variable location
mat4 view, mv;
mat4 matrixIdentity;
mat4 cur_proj;

point4 points[NumVertices];
vec4   normals[NumVertices];

struct object {
    int unique_id;
    vec4 color;
    char *fileName;
    GLuint vao;
    std::vector < vec4 > out_vertices;
    std::vector < vec4 > out_normals;
    int obj_num_vert;
    mat4 obj_model;
    mat4 manip_model;
    GLuint vao_manip;
    int wireframe_mode;
    float x_trans, y_trans, z_trans;
    float x_trans_manip, y_trans_manip, z_trans_manip;
    float x_rotate, y_rotate, z_rotate;
    float x_scale, y_scale, z_scale;
};

point4 ref_floor[] = {
    point4( 1, 0, 1, 1),
    point4( -1, 0, 1, 1),
    point4( -1, 0, -1, 1),
    
    point4( -1, 0, -1, 1),
    point4( 1, 0, 1, 1),
    point4( 1, 0, -1, 1),
};

point4 ref_floor_color[] = {
    point4( .5, .5, .5, 1),
    point4( .5, .5, .5, 1),
    point4( .5, .5, .5, 1),
    point4( .5, .5, .5, 1),
    point4( .5, .5, .5, 1),
    point4( .5, .5, .5, 1),
};

point4 manipulator[] = {
    //x-axis
    point4( 0.0, 0.0, 0.0, 1),
    point4( 1.0, 0.0, 0.0, 1),
    point4( 1.0, LINE_WIDTH, 0.0, 1),
    point4( 1.0, LINE_WIDTH, 0.0, 1),
    point4( 0.0, 0.0, 0.0, 1),
    point4( 0.0, LINE_WIDTH, 0.0, 1),
    point4( 0.0, LINE_WIDTH, 0.0, 1),
    point4( 1.0, LINE_WIDTH, 0.0, 1),
    point4( 1.0, 0.0, LINE_WIDTH, 1),
    point4( 1.0, 0.0, LINE_WIDTH, 1),
    point4( 0.0, LINE_WIDTH, 0.0, 1),
    point4( 0.0, 0.0, LINE_WIDTH, 1),
    point4( 0.0, 0.0, 0.0, 1),
    point4( 1.0, 0.0, 0.0, 1),
    point4( 1.0, 0.0, LINE_WIDTH, 1),
    point4( 1.0, 0.0, LINE_WIDTH, 1),
    point4( 0.0, 0.0, 0.0, 1),
    point4( 0.0, 0.0, LINE_WIDTH, 1),
    //y-axis
    point4( 0.0, 0.0, 0.0, 1),
    point4( 0.0, 1.0, 0.0, 1),
    point4( LINE_WIDTH, 1.0, 0.0, 1),
    point4( LINE_WIDTH, 1.0, 0.0, 1),
    point4( 0.0, 0.0, 0.0, 1),
    point4( LINE_WIDTH, 0.0, 0.0, 1),
    point4( LINE_WIDTH, 0.0, 0.0, 1),
    point4( LINE_WIDTH, 1.0, 0.0, 1),
    point4( 0.0, 1.0, LINE_WIDTH, 1),
    point4( 0.0, 1.0, LINE_WIDTH, 1),
    point4( LINE_WIDTH, 0.0, 0.0, 1),
    point4( 0.0, 0.0, LINE_WIDTH, 1),
    point4( 0.0, 0.0, 0.0, 1),
    point4( 0.0, 1.0, 0.0, 1),
    point4( 0.0, 1.0, LINE_WIDTH, 1),
    point4( 0.0, 1.0, LINE_WIDTH, 1),
    point4( 0.0, 0.0, 0.0, 1),
    point4( 0.0, 0.0, LINE_WIDTH, 1),
    //z-axis
    point4( 0.0, 0.0, 0.0, 1),
    point4( 0.0, 0.0, 1.0, 1),
    point4( 0.0, LINE_WIDTH, 1.0, 1),
    
    point4( 0.0, LINE_WIDTH, 1.0, 1),
    point4( 0.0, 0.0, 0.0, 1),
    point4( 0.0, LINE_WIDTH, 0.0, 1),
    
    point4( 0.0, LINE_WIDTH, 0.0, 1),
    point4( 0.0, LINE_WIDTH, 1.0, 1),
    point4( LINE_WIDTH, 0.0, 1.0, 1),
    
    point4( LINE_WIDTH, 0.0, 1.0, 1),
    point4( 0.0, LINE_WIDTH, 0.0, 1),
    point4( LINE_WIDTH, 0.0, 0.0, 1),
    
    point4( 0.0, 0.0, 0.0, 1),
    point4( 0.0, 0.0, 1.0, 1),
    point4( LINE_WIDTH, 0.0, 1.0, 1),
    
    point4( LINE_WIDTH, 0.0, 1.0, 1),
    point4( 0.0, 0.0, 0.0, 1),
    point4( LINE_WIDTH, 0.0, 0.0, 1),
};

point4 manipulator_color[] = {
    //x-axis
    point4( 1, 0, 0, 1),
    point4( 1, 0, 0, 1),
    point4( 1, 0, 0, 1),
    point4( 1, 0, 0, 1),
    point4( 1, 0, 0, 1),
    point4( 1, 0, 0, 1),
    point4( 1, 0, 0, 1),
    point4( 1, 0, 0, 1),
    point4( 1, 0, 0, 1),
    point4( 1, 0, 0, 1),
    point4( 1, 0, 0, 1),
    point4( 1, 0, 0, 1),
    point4( 1, 0, 0, 1),
    point4( 1, 0, 0, 1),
    point4( 1, 0, 0, 1),
    point4( 1, 0, 0, 1),
    point4( 1, 0, 0, 1),
    point4( 1, 0, 0, 1),
    //y-axis
    point4( 0, 1, 0, 1),
    point4( 0, 1, 0, 1),
    point4( 0, 1, 0, 1),
    point4( 0, 1, 0, 1),
    point4( 0, 1, 0, 1),
    point4( 0, 1, 0, 1),
    point4( 0, 1, 0, 1),
    point4( 0, 1, 0, 1),
    point4( 0, 1, 0, 1),
    point4( 0, 1, 0, 1),
    point4( 0, 1, 0, 1),
    point4( 0, 1, 0, 1),
    point4( 0, 1, 0, 1),
    point4( 0, 1, 0, 1),
    point4( 0, 1, 0, 1),
    point4( 0, 1, 0, 1),
    point4( 0, 1, 0, 1),
    point4( 0, 1, 0, 1),
    //z-axis
    point4( 0, 0, 1, 1),
    point4( 0, 0, 1, 1),
    point4( 0, 0, 1, 1),
    point4( 0, 0, 1, 1),
    point4( 0, 0, 1, 1),
    point4( 0, 0, 1, 1),
    point4( 0, 0, 1, 1),
    point4( 0, 0, 1, 1),
    point4( 0, 0, 1, 1),
    point4( 0, 0, 1, 1),
    point4( 0, 0, 1, 1),
    point4( 0, 0, 1, 1),
    point4( 0, 0, 1, 1),
    point4( 0, 0, 1, 1),
    point4( 0, 0, 1, 1),
    point4( 0, 0, 1, 1),
    point4( 0, 0, 1, 1),
    point4( 0, 0, 1, 1),
};

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[8] = {
    point4( -0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5, -0.5, -0.5, 1.0 ),
    point4( -0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5, -0.5, -0.5, 1.0 )
};


//----------------------------------------------------------------------------

// quad generates two triangles for each face and assigns colors
//    to the vertices.  Notice we keep the relative ordering when constructing the tris
int Index = 0;
void
quad( int a, int b, int c, int d )
{
    
    
    vec4 u = vertices[b] - vertices[a];
    vec4 v = vertices[c] - vertices[b];
    
    vec4 normal = normalize( cross(u, v) );
    normal[3] = 0.0;
    
    normals[Index] = normal; points[Index] = vertices[a]; Index++;
    normals[Index] = normal; points[Index] = vertices[b]; Index++;
    normals[Index] = normal; points[Index] = vertices[c]; Index++;
    normals[Index] = normal; points[Index] = vertices[a]; Index++;
    normals[Index] = normal; points[Index] = vertices[c]; Index++;
    normals[Index] = normal; points[Index] = vertices[d]; Index++;
}

//----------------------------------------------------------------------------

// generate 12 triangles: 36 vertices and 36 colors
void
colorcube()
{
    quad( 4, 5, 6, 7 );
    quad( 5, 4, 0, 1 );
    quad( 1, 0, 3, 2 );
    quad( 2, 3, 7, 6 );
    quad( 3, 0, 4, 7 );
    quad( 6, 5, 1, 2 );
}

//----------------------------------------------------------------------------

// OpenGL initialization
void
init()
{
    if (VERBOSE) {
        printf("Init() running...\n");
    }
    
    // Load shaders and use the resulting shader program
    program = InitShader( "vshader.glsl", "fshader.glsl" );
    glUseProgram( program );
    
    //setup for each object
    for (int i = 0; i < objs.size(); i++) {
        if (VERBOSE) {
            fprintf(stderr, "Number of vertices for %s: %d\n", objs.at(i).fileName, objs.at(i).obj_num_vert);
        }
        
        // Create a vertex array object
        glGenVertexArrays( 1, &objs.at(i).vao );
        glBindVertexArray( objs.at(i).vao );
        
        // Create and initialize a buffer object
        GLuint buffer;
        glGenBuffers( 1, &buffer );
        glBindBuffer( GL_ARRAY_BUFFER, buffer );
        glBufferData( GL_ARRAY_BUFFER, (objs.at(i).out_vertices.size() * sizeof(vec4)) + (objs.at(i).out_normals.size() * sizeof(vec4)), &objs.at(i).out_vertices[0], GL_STATIC_DRAW );
        glBufferSubData( GL_ARRAY_BUFFER, 0, objs.at(i).out_vertices.size() * sizeof(vec4), &objs.at(i).out_vertices[0] );
        glBufferSubData( GL_ARRAY_BUFFER, objs.at(i).out_vertices.size() * sizeof(vec4), objs.at(i).out_normals.size() * sizeof(vec4), &objs.at(i).out_normals[0] );
        
        // set up vertex arrays
        GLuint vPosition = glGetAttribLocation( program, "vPosition" );
        glEnableVertexAttribArray( vPosition );
        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
                              BUFFER_OFFSET(0) );
        
        GLuint vNormal = glGetAttribLocation( program, "vNormal" );
        glEnableVertexAttribArray( vNormal );
        glVertexAttribPointer( vNormal, 4, GL_FLOAT, GL_FALSE, 0,
                              BUFFER_OFFSET(objs.at(i).out_vertices.size() * sizeof(vec4)) );
        
        // set up its own manipulator
        glGenVertexArrays(1, &objs.at(i).vao_manip);
        glBindVertexArray( objs.at(i).vao_manip);
        
        GLuint buf_manip;
        glGenBuffers( 1, &buf_manip);
        glBindBuffer( GL_ARRAY_BUFFER, buf_manip);
        
        glBufferData( GL_ARRAY_BUFFER, sizeof(manipulator) + sizeof(manipulator_color), manipulator, GL_STATIC_DRAW );
        glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(manipulator), manipulator);
        glBufferSubData( GL_ARRAY_BUFFER, sizeof(manipulator), sizeof(manipulator_color), manipulator_color );

        vPosition = glGetAttribLocation( program, "vPosition" );
        glEnableVertexAttribArray( vPosition );
        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
                              BUFFER_OFFSET(0) );
        
        GLuint manip_color = glGetAttribLocation( program, "manip_color" );
        glEnableVertexAttribArray( manip_color );
        glVertexAttribPointer( manip_color, 4, GL_FLOAT, GL_FALSE, 0,
                              BUFFER_OFFSET(sizeof(manipulator)) );

    }
    
    // Initialize shader lighting parameters
    // RAM: No need to change these...we'll learn about the details when we
    // cover Illumination and Shading
    point4 light_position( 1.5, 1.5, 2.0, 1.0 );
    color4 light_ambient( 0.2, 0.2, 0.2, 1.0 );
    color4 light_diffuse( 1.0, 1.0, 1.0, 1.0 );
    color4 light_specular( 1.0, 1.0, 1.0, 1.0 );
    
    color4 material_ambient( 1.0, 0.0, 1.0, 1.0 );
    color4 material_diffuse( 1.0, 0.8, 0.0, 1.0 );
    color4 material_specular( 1.0, 0.8, 0.0, 1.0 );
    float  material_shininess = 100.0;
    
    color4 ambient_product = light_ambient * material_ambient;
    color4 diffuse_product = light_diffuse * material_diffuse;
    color4 specular_product = light_specular * material_specular;
    
    glUniform4fv( glGetUniformLocation(program, "AmbientProduct"),
                 1, ambient_product );
    glUniform4fv( glGetUniformLocation(program, "DiffuseProduct"),
                 1, diffuse_product );
    glUniform4fv( glGetUniformLocation(program, "SpecularProduct"),
                 1, specular_product );
    
    glUniform4fv( glGetUniformLocation(program, "LightPosition"),
                 1, light_position );
    
    glUniform1f( glGetUniformLocation(program, "Shininess"),
                material_shininess );
    
    model_view = glGetUniformLocation( program, "ModelView" );
    projection = glGetUniformLocation( program, "Projection" );
    
    if (projType == "perspective") {
        cur_proj = Perspective(yfov, aspRatio, near, far);
    } else if (projType == "orthographic") {
        cur_proj = Ortho(o_left, o_right, o_bottom, o_top, near, far);
    }
    point4  eye( eyex, eyey, eyez, 1);
    point4  at( atx, aty, atz, 1.0 );
    vec4    up( upx, upy, upz, 0.0 );
    
    view = LookAt( eye, at, up );
    matrixIdentity = identity();
    mv = matrixIdentity;
    
    glUniformMatrix4fv( model_view, 1, GL_TRUE, view );
    glUniformMatrix4fv( projection, 1, GL_TRUE, cur_proj );
    
    
    glEnable( GL_DEPTH_TEST );
    glClearColor( 1.0, 1.0, 1.0, 1.0 );
}

//----------------------------------------------------------------------------
//sync current obj_model with model_view in vertex shader
//i is the index of object to sync in the objs array
void sync_object_mv(int i)
{
    mv = matrixIdentity;
    mv = mv * view;
    mv = mv * objs.at(i).obj_model;
    glUniformMatrix4fv( model_view, 1, GL_TRUE, mv );
}

//i is the object index that holders the manipulator to be synced
void sync_manip_mv(int i)
{
    mv = matrixIdentity;
    mv = mv * view;
    mv = mv * objs.at(i).manip_model;
    glUniformMatrix4fv( model_view, 1, GL_TRUE, mv );
}

void
display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    for (int i = 0; i < objs.size(); i++) {
        glBindVertexArray( objs.at(i).vao );
        sync_object_mv(i);
        if (clicked_object == objs.at(i).unique_id && objs.at(i).wireframe_mode == 1) {
            //draw in wireframe mode
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glPolygonOffset(1, 2 );
            glDrawArrays(GL_TRIANGLES, 0, objs.at(i).obj_num_vert);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            
            //draw manipulator
            glUniform1i( glGetUniformLocation(program, "renderingManip"), 1);
            glBindVertexArray( objs.at(i).vao_manip );
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            sync_manip_mv(i);
            glDrawArrays( GL_TRIANGLES, 0, manip_num_vert);
            glUniform1i( glGetUniformLocation(program, "renderingManip"), 0);
            
        } else {
            //draw in regular mode
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            sync_object_mv(i);
            glDrawArrays( GL_TRIANGLES, 0, objs.at(i).obj_num_vert );
        }
    }
    glutSwapBuffers();
}

//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
    switch( key ) {
        case 033:  // Escape key
        case 'q': case 'Q':
            exit( EXIT_SUCCESS );
            break;
        case 'r':
            fprintf(stderr, "Rotation Mode\n");
            trans_mode = "r";
            break;
        case 't':
            fprintf(stderr, "Translation Mode\n");
            trans_mode = "t";
            break;
        case 's':
            fprintf(stderr, "Scale Mode\n");
            trans_mode = "s";
            break;
    }
}

//----------------------------------------------------------------------------
//mouse click actions

//correctly set new rotations
//translate obj to origin, apply rotation, translate back
void myRotate(int obj, float x, float y, float z)
{
    objs.at(obj).x_rotate += x;
    objs.at(obj).y_rotate += y;
    objs.at(obj).z_rotate += z;
//    printf("rotatation: %f, %f, %f\n", objs.at(obj).x_rotate, objs.at(obj).y_rotate, objs.at(obj).z_rotate);
    
    //matrix for re-translation
    mat4 re_translate;
    re_translate = re_translate * Translate(objs.at(obj).x_trans,
                                            objs.at(obj).y_trans,
                                            objs.at(obj).z_trans);
    //matrix for translating to origin
    mat4 un_translate;
    un_translate = un_translate * Translate(-objs.at(obj).x_trans,
                                            -objs.at(obj).y_trans,
                                            -objs.at(obj).z_trans);
    //initialize new rotation matrix to be applied
    mat4 r;
    r = r * RotateX(x);
    r = r * RotateY(y);
    r = r * RotateZ(z);
    
    //applying all matrices (ORDER MATTERS when being applied in-line)
    objs.at(obj).obj_model = un_translate * objs.at(obj).obj_model;
    objs.at(obj).obj_model = r * objs.at(obj).obj_model;
    objs.at(obj).obj_model = re_translate * objs.at(obj).obj_model;
}

void myTranslate(int obj, float x, float y, float z)
{
    objs.at(obj).x_trans += x;
    objs.at(obj).y_trans += y;
    objs.at(obj).z_trans += z;
//    printf("translate %f, %f, %f\n", objs.at(obj).x_trans, objs.at(obj).y_trans, objs.at(obj).z_trans);
    
    mat4 t;
    t = Translate(x, y, z);

    //ORDER MATTERS
    objs.at(obj).obj_model = t * objs.at(obj).obj_model;
    objs.at(obj).manip_model = t * objs.at(obj).manip_model;
}

void myScale(int obj, float x, float y, float z)
{
    //undo scaling matrix
    mat4 un_scale = Scale(1 / objs.at(obj).x_scale, 1 / objs.at(obj).y_scale, 1 / objs.at(obj).z_scale);
    
    objs.at(obj).x_scale += x;
    objs.at(obj).y_scale += y;
    objs.at(obj).z_scale += z;
//    printf("scale: %f, %f, %f\n", objs.at(obj).x_scale, objs.at(obj).y_scale, objs.at(obj).z_scale);
    
    //matrix for re-translation
    mat4 re_translate;
    re_translate = re_translate * Translate(objs.at(obj).x_trans,
                                            objs.at(obj).y_trans,
                                            objs.at(obj).z_trans);
    //matrix for translating to origin
    mat4 un_translate;
    un_translate = un_translate * Translate(-objs.at(obj).x_trans,
                                            -objs.at(obj).y_trans,
                                            -objs.at(obj).z_trans);
    //new scale matrix
    mat4 re_scale;
    re_scale = Scale(objs.at(obj).x_scale, objs.at(obj).y_scale, objs.at(obj).z_scale);
    
    //applying all matrices (ORDER MATTERS)
    objs.at(obj).obj_model = un_translate * objs.at(obj).obj_model;
    objs.at(obj).obj_model = un_scale * objs.at(obj).obj_model;
    objs.at(obj).obj_model = re_scale * objs.at(obj).obj_model;
    objs.at(obj).obj_model = re_translate * objs.at(obj).obj_model;

}

void drag(int x, int y)
{
    if (clicked_object != 0 && axis_to_drag != "") {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        int i;
        for (i = 0; i < objs.size(); i++) {
            //if you are the object being dragged...
            if (clicked_object == objs.at(i).unique_id && objs.at(i).wireframe_mode == 1) {
                glBindVertexArray(objs.at(i).vao);
                
                //get view*projection matrix
                mat4 viewproj = cur_proj * view;
                //calculate the unit vector drawn by mouse [intial pos - current pos]
                vec4 drawn_vec(x - init_x_pos, init_y_pos - y, 0, 1);  //reverse y for display grid
                float distance = 0;
                float tmp_holder = 0;  //tmp holder for: prev_distance = distance.
                if (axis_to_drag == "x") {
                    vec4 x_axis(1, 0, 0, 1);
                    //get x axis in screen coordinates
                    x_axis = viewproj * x_axis;
                    //find distance drawn relative to axis direction
                    distance = dot(drawn_vec, x_axis);
                    tmp_holder = distance;
                    distance = distance - prev_distance;

                    if (trans_mode == "t") {
                        distance = distance / t_sensitivity;
                        myTranslate(i, distance, 0, 0);
                    } else if (trans_mode == "r") {
                        distance = distance * r_sensitivity;
                        //manage rotation variable overflow
                        if (objs.at(i).x_rotate > 360) {
                            objs.at(i).x_rotate -= 360;
                        } else if (objs.at(i).x_rotate < -360) {
                            objs.at(i).x_rotate += 360;
                        }
                        myRotate(i, distance, 0, 0);

                    } else if (trans_mode == "s") {
                        distance = distance / s_sensitivity;
                        myScale(i, distance, 0, 0);
                    } else {
                        fprintf(stderr, "Error: Transformation mode not set. Press t, r, or s.\n");
                        return;
                    }
                } else if (axis_to_drag == "y") {
                    vec4 y_axis(0, 1, 0, 1);
                    //get y axis in screen coordinates
                    y_axis = viewproj * y_axis;
                    //find distance drawn relative to axis direction
                    distance = dot(drawn_vec, y_axis);
                    tmp_holder = distance;
                    distance = distance - prev_distance;
                    if (trans_mode == "t") {
                        distance = distance / t_sensitivity;
                        myTranslate(i, 0, distance, 0);
                    } else if (trans_mode == "r") {
                        distance = distance * r_sensitivity;
                        //manage rotation variable overflow
                        if (objs.at(i).y_rotate > 360) {
                            objs.at(i).y_rotate -= 360;
                        } else if (objs.at(i).y_rotate < -360) {
                            objs.at(i).y_rotate += 360;
                        }
                        myRotate(i, 0, distance, 0);
                        
                    } else if (trans_mode == "s") {
                        distance = distance / s_sensitivity;
                        myScale(i, 0, distance, 0);
                    } else {
                        fprintf(stderr, "Error: Transformation mode not set. Press t, r, or s.\n");
                        return;
                    }
                } else if (axis_to_drag == "z") {
                    vec4 z_axis(0, 0, 1, 1);
                    //get z axis in screen coordinates
                    z_axis = viewproj * z_axis;
                    //find distance drawn relative to axis direction
                    distance = dot(drawn_vec, z_axis);
                    tmp_holder = distance;
                    distance = distance - prev_distance;
                    if (trans_mode == "t") {
                        distance = distance / t_sensitivity;
                        myTranslate(i, 0, 0, distance);
                    } else if (trans_mode == "r") {
                        distance = distance * r_sensitivity;
                        //manage rotation variable overflow
                        if (objs.at(i).z_rotate > 360) {
                            objs.at(i).z_rotate -= 360;
                        } else if (objs.at(i).z_rotate < -360) {
                            objs.at(i).z_rotate += 360;
                        }
                        myRotate(i, 0, 0, distance);
                        
                    } else if (trans_mode == "s") {
                        distance = distance / s_sensitivity;
                        myScale(i, 0, 0, distance);
                    } else {
                        fprintf(stderr, "Error: Transformation mode not set. Press t, r, or s.\n");
                        return;
                    }
                } else {
                    fprintf(stderr, "Fatal Error: dragging manipulator without constant set\n");
                    exit(EXIT_FAILURE);
                }
                prev_distance = tmp_holder;

                sync_object_mv(i);

                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glPolygonOffset(1, 2 );
                glDrawArrays(GL_TRIANGLES, 0, objs.at(i).obj_num_vert);
                
                //draw manipulator
                glUniform1i( glGetUniformLocation(program, "renderingManip"), 1);
                glBindVertexArray( objs.at(i).vao_manip );
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                sync_manip_mv(i);
                glDrawArrays( GL_TRIANGLES, 0, manip_num_vert);
                glUniform1i( glGetUniformLocation(program, "renderingManip"), 0);
            } else {
                glBindVertexArray(objs.at(i).vao);
                sync_object_mv(i);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glDrawArrays(GL_TRIANGLES, 0, objs.at(i).obj_num_vert);
            }
        }
        glutSwapBuffers();
    }
}

void myMouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        //check if manipulator was clicked
        GLubyte color[4];
        int rgb_clicked = 0;
        glReadPixels(x, current_height - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, (void *)color);
        if (color[0] == 255 && color[1] == 0 && color[2] == 0) {
            rgb_clicked = 1;
        } else if (color[0] == 0 && color[1] == 255 && color[2] == 0) {
            rgb_clicked = 2;
        } else if (color[0] == 0 && color[1] == 0 && color[2] == 255) {
            rgb_clicked = 3;
        }
        
        //manipulator was clicked / enable dragging
        if (clicked_object != 0 && rgb_clicked != 0) {
            init_x_pos = x;
            init_y_pos = y;
            glDrawBuffer(GL_BACK);
            glReadBuffer(GL_BACK);
            if (rgb_clicked == 1) {
                if (VERBOSE) {
                    printf("red x-axis clicked\n");
                }
                axis_to_drag = "x";
            } else if (rgb_clicked == 2) {
                if (VERBOSE) {
                    printf("green y-axis clicked\n");
                }
                axis_to_drag = "y";
            } else {
                if (VERBOSE) {
                    printf("blue z-axis clicked\n");
                }
                axis_to_drag = "z";
            }
            return;
        }
        
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glDrawBuffer(GL_BACK);
        glReadBuffer(GL_BACK);
        
        //for each object, draw in ID mode
        for (int i = 0; i < objs.size(); i++) {
            glBindVertexArray(objs.at(i).vao);
            glUniform1i(glGetUniformLocation(program, "id_mode"), 1);
            GLuint objectColorId = glGetUniformLocation(program, "objectColor");
            glUniform4f(objectColorId, objs.at(i).color.x, objs.at(i).color.y, objs.at(i).color.z, objs.at(i).color.z);
            sync_object_mv(i);
            glDrawArrays( GL_TRIANGLES, 0, objs.at(i).obj_num_vert );
        }
        
        //capture the object/color clicked
        glReadPixels(x, current_height - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, (void *)color);
        if ((color[0] == 255) || (clicked_object == color[0])) {
            clicked_object = 0;
        } else {
            clicked_object = color[0];
        }
        
        //rerender original objects
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glUniform1i(glGetUniformLocation(program, "id_mode"), 0);
        for (int i = 0; i < objs.size(); i++) {
            glBindVertexArray(objs.at(i).vao);
            if (objs.at(i).unique_id == clicked_object) {
                //draw in wireframe mode
                sync_object_mv(i);
                objs.at(i).wireframe_mode = 1;
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glPolygonOffset(1, 2 );
                sync_object_mv(i);
                glDrawArrays( GL_TRIANGLES, 0, objs.at(i).obj_num_vert );
                
                //draw manipulator
                sync_manip_mv(i);
                glUniform1i( glGetUniformLocation(program, "renderingManip"), 1);
                glBindVertexArray( objs.at(i).vao_manip );
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glDrawArrays( GL_TRIANGLES, 0, manip_num_vert);
                glUniform1i( glGetUniformLocation(program, "renderingManip"), 0);
            } else {
                //draw regular mode
                objs.at(i).wireframe_mode = 0;
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                sync_object_mv(i);
                glDrawArrays( GL_TRIANGLES, 0, objs.at(i).obj_num_vert );
            }
        }
        
        //clean up
        glReadBuffer(GL_FRONT);
        glDrawBuffer(GL_FRONT);
        glutSwapBuffers();
    } else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        //disable all dragging
        axis_to_drag = "";
        init_x_pos = 0;
        init_y_pos = 0;
        prev_distance = 0;
        glDrawBuffer(GL_FRONT);
        glReadBuffer(GL_FRONT);
    }
}

//----------------------------------------------------------------------------
//prompts / user input

void promptInteger(const char prompt[], int *des)
{
    int tmp;
    while ( 1 ) {
        cout << prompt;
        if (!(cin >> tmp)) {
            cout << "Please enter numbers only\n" << endl;
            cin.clear();
            cin.ignore(10000, '\n');
        } else {
            *des = tmp;
            cin.clear();
            cin.ignore(10000, '\n');
            break;
        }
    }
    if (VERBOSE) {
        cout << "Capured: " << *des << endl;
    }
}

void promptFloat(const char prompt[], float *des)
{
    float tmp;
    while ( 1 ) {
        cout << prompt;
        if (!(cin >> tmp)) {
            cout << "Please enter numbers/decimals only\n" << endl;
            cin.clear();
            cin.ignore(10000, '\n');
        } else {
            *des = tmp;
            cin.clear();
            cin.ignore(10000, '\n');
            break;
        }
    }
    if (VERBOSE) {
        cout << "Capured: " << *des << endl;
    }
}

void getSceneInput()
{
    if (VERBOSE) {
        printf("Getting scene input\n");
    }
    int input = -1;
    printf("Enter the number associated with each option:\n");
    int done = 0;
    do {
        promptInteger("(1) Orthographic or (2) Perspective view: ", &input);
        if (input == 1){
            projType = "orthographic";
        } else if (input == 2){
            projType = "perspective";
        } else {
            printf("invalid option\n");
            continue;
        }
        if (projType == "orthographic") {   //orthographic
            while ( 1 ) {
                input = -1;
                if (VERBOSE) {
                    cout << "Picked Orthographic view.\n";
                }
                cout << endl;
                cout << "Enter number values to the follow prompts (floats):\n";
                promptFloat("left: ", &o_left);
                promptFloat("right: ", &o_right);
                promptFloat("bottom: ", &o_bottom);
                promptFloat("top: ", &o_top);
                promptFloat("near: ", &near);
                promptFloat("far: ", &far);
                printf("\nyou entered:\nleft: %.2f, right: %.2f, bottom: %.2f, top: %.2f, near: %.2f, far: %.2f, ", o_left, o_right, o_bottom, o_top, near, far);
                promptInteger("is this correct?\n (1)yes / (2) no: ", &input);
                if (input == 1) {
                    break;
                } else if (input == 2) {
                    continue;
                } else {
                    printf("invalid option\n");
                    continue;
                }
            }
        } else {                            //perspective
            while ( 1 ) {
                if (VERBOSE) {
                    cout << "Picked Perspective view.\n";
                }
                cout << endl;
                cout << "Enter number values to the follow prompts (floats):\n";
                promptFloat("field of view (degrees): ", &yfov);
                promptFloat("aspect ratio: ", &aspRatio);
                promptFloat("near: ", &near);
                promptFloat("far: ", &far);
                printf("\nyou entered:\nfield of view: %.2f, aspect ratio: %.2f, near: %.2f, far: %.2f\n", yfov, aspRatio, near, far);
                promptInteger("is this correct?\n(1) yes / (2) no: ", &input);
                if (input == 1) {
                    break;
                } else if (input == 2) {
                    continue;
                } else {
                    printf("invalid option\n");
                    continue;
                }
            }
        }
        while ( 1 ) {
            cout << endl;
            promptFloat("eye x: ", &eyex);
            promptFloat("eye y: ", &eyey);
            promptFloat("eye z: ", &eyez);
            promptFloat("at x: ", &atx);
            promptFloat("at y: ", &aty);
            promptFloat("at z: ", &atz);
            promptFloat("up x: ", &upx);
            promptFloat("up y: ", &upy);
            promptFloat("up z: ", &upz);
            printf("\nyou entered:\neyex: %.2f, eyey: %.2f, eyez: %.2f, atx: %.2f, aty: %.2f, atz: %.2f, upx: %.2f, upy: %.2f, upz: %.2f\n", eyex, eyey, eyez, atx, aty, atz, upx, upy, upz);
            promptInteger("is this correct?\n(1) yes / (2) no: ", &input);
            if (input == 1) {
                done = 1;
                break;
            } else if (input == 2) {
                continue;
            } else {
                printf("invalid option\n");
                continue;
            }

        }
    } while ( done == 0);
}

//--------------------------------------------------------------------------------


//window reshape function
void myReshape(int w, int h)
{
//    cout << "before reshape: " << glGetUniformLocation(program, "Projection") << endl;
    current_height = h;
    glViewport(0,0,w,h);
    float ar = w/h;
    mat4 proj;

    if (projType == "orthographic") { //orthographic
        if( ar < startAR) { // (w <= h ){ //taller
            proj = Ortho(o_left, o_right, o_left * (GLfloat) h / (GLfloat) w,
                    o_right * (GLfloat) h / (GLfloat) w, near, far);
            
        }
        else //wider
            
        {
            proj = Ortho(o_bottom * (GLfloat) w / (GLfloat) h, o_top *
                    (GLfloat) w / (GLfloat) h, o_bottom, o_top, near, far);
            
        }
    } else { //perspective FIX THIS TO USE FRUSTUM
        cout << "width: " << w << endl;
        cout << "height: " << h << endl;
        if( ar < startAR) { // (w <= h ){ //taller
            GLfloat tmpLeft = -(near * tan(yfov / 2 * (PI / 180))) * (GLfloat) h / (GLfloat) w;
            GLfloat tmpRight = (near * tan(yfov / 2 * (PI / 180))) * (GLfloat) h / (GLfloat) w;
            GLfloat tmpBottom = tmpLeft * (GLfloat) h / (GLfloat) w;
            GLfloat tmpTop = tmpRight * (GLfloat) h / (GLfloat) w;
            
            cout << "tmpLeft: " << tmpLeft << ", tmpRight: " << tmpRight << ", tmpBottom: " << tmpBottom << ", tmpTop: " << tmpTop << endl;
            proj = Frustum(tmpLeft, tmpRight, tmpBottom, tmpTop, near, far);
        }
        else //wider
        {
            GLfloat tmpBottom = -(near * tan(yfov / 2 * (PI / 180)));
            GLfloat tmpTop = (near * tan(yfov / 2 * (PI / 180)));
            GLfloat tmpLeft = tmpBottom * (GLfloat) w / (GLfloat) h;
            GLfloat tmpRight = tmpTop * (GLfloat) w / (GLfloat) h;
            cout << "tmpLeft: " << tmpLeft << ", tmpRight: " << tmpRight << ", tmpBottom: " << tmpBottom << ", tmpTop: " << tmpTop << endl;
            proj = Frustum(tmpLeft, tmpRight, tmpBottom, tmpTop, near, far);
            
        }
    }
    
    GLuint projectionLoc = glGetUniformLocation(program, "Projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &proj[0][0]);

}

//returns an array of struct object
void loadObjects(int argc, char **argv)
{
    if (argc <= 1) {
        cout << "Run with at least one obj file:" << endl;
        cout << "./prog [file1.obj.txt] [file2.obj.txt] ..." << endl;
        exit(EXIT_SUCCESS);
    }
    
    int r, g, b;
    for(int i = 1; i < argc; i++) {
        struct object tmp_obj;
        tmp_obj.fileName = argv[i];
        tmp_obj.unique_id = i;
        tmp_obj.wireframe_mode = 0;
        tmp_obj.x_trans = 0;
        tmp_obj.y_trans = 0;
        tmp_obj.z_trans = 0;
        tmp_obj.x_trans_manip = 0;
        tmp_obj.y_trans_manip = 0;
        tmp_obj.z_trans_manip = 0;
        tmp_obj.x_rotate = 0;
        tmp_obj.y_rotate = 0;
        tmp_obj.z_rotate = 0;
        tmp_obj.x_scale = 1;
        tmp_obj.y_scale = 1;
        tmp_obj.z_scale = 1;
        r = (i & 0x000000FF) >> 0;
        g = (i & 0x0000FF00) >> 8;
        b = (i & 0x00FF0000) >> 16;
        tmp_obj.color.x = r / 256.0f;
        tmp_obj.color.y = g / 256.0f;
        tmp_obj.color.z = b / 256.0f;
        tmp_obj.color.w = 1;
        
        //loader stuff
        std::vector <unsigned int> vertexIndices, normalIndices;
        std::vector <vec4> temp_vertices;
        std::vector <vec4> temp_normals;
        
        if (VERBOSE) {
            cout << "Loading " << tmp_obj.fileName << " ... ";
        }
        
        FILE * file = fopen(tmp_obj.fileName, "r");
        if (file == NULL) {
            printf("Could not open file");
            exit(EXIT_FAILURE);
        }
        
        while( 1 ){
            
            char lineHeader[128];
            // read the first word of the line
            int res = fscanf(file, "%s", lineHeader);
            if (res == EOF)
                break; // EOF = End Of File. Quit the loop.
            // else : parse lineHeader
            if ( strcmp( lineHeader, "v" ) == 0 ){
                vec3 vertex;
                fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
//                if (VERBOSE) { fprintf(stderr, "Captured vertex: %f %f %f\n", vertex.x, vertex.y, vertex.z); }
                temp_vertices.push_back(vertex);
                //        }else if ( strcmp( lineHeader, "vt" ) == 0 ){
                //            vec2 uv;
                //            fscanf(file, "%f %f\n", &uv.x, &uv.y );
                //            temp_uvs.push_back(uv);
            }else if ( strcmp( lineHeader, "#" ) == 0){
                fscanf(file, "%[^\n]\n", lineHeader);
//                if (VERBOSE) { fprintf(stderr, "Captured comment: %s\n", lineHeader); }
            }else if ( strcmp( lineHeader, "vn" ) == 0 ){
                vec3 normal;
                fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
//                if (VERBOSE) { fprintf(stderr, "Captured normal: %f %f %f\n", normal.x, normal.y, normal.z); }
                temp_normals.push_back(normal);
            }else if ( strcmp( lineHeader, "f" ) == 0 ){
                std::string vertex1, vertex2, vertex3;
                unsigned int vertexIndex[3], normalIndex[3];
                int matches = fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2] );
//                if (VERBOSE) { fprintf(stderr, "Captured face: %d//%d %d//%d %d//%d\n", vertexIndex[0], normalIndex[0], vertexIndex[1], normalIndex[1], vertexIndex[2], normalIndex[2]); }
                if (matches != 6){
                    fprintf(stderr, "Invalid matches (6), only found %d\n", matches);
                    printf("File can't be read by our simple parser : ( Try exporting with other options\n");
                    exit(EXIT_FAILURE);
                }
                vertexIndices.push_back(vertexIndex[0]);
                vertexIndices.push_back(vertexIndex[1]);
                vertexIndices.push_back(vertexIndex[2]);
                normalIndices.push_back(normalIndex[0]);
                normalIndices.push_back(normalIndex[1]);
                normalIndices.push_back(normalIndex[2]);
            }
        }
        
        std::vector < vec4 > out_vertices;
        std::vector < vec4 > out_normals;
        for (int i = 0; i < vertexIndices.size(); i++) {
            int vertexIndex = vertexIndices[i];
            vec4 vertex = temp_vertices[ vertexIndex - 1];
            vertex.w = 1.0;
            out_vertices.push_back(vertex);
        }
        for (int i = 0; i < normalIndices.size(); i++) {
            int normalIndex = normalIndices[i];
            vec4 normal = temp_normals[ normalIndex - 1];
            normal.w = 0.0;
            out_normals.push_back(normal);
        }
        
        tmp_obj.out_normals = out_normals;
        tmp_obj.out_vertices = out_vertices;
        tmp_obj.obj_num_vert = out_vertices.size();
        objs.push_back(tmp_obj);
        
        if (VERBOSE) {
            cout << "Success" << endl;
        }
    }
//    struct object floor;
//    floor.unique_id = -1;
//    floor.out_vertices = ref_floor;
    
}

/*
 *  simple.c
 *  This program draws a red rectangle on a white background.
 *
 * Still missing the machinery to move to 3D
 */

/* glut.h includes gl.h and glu.h*/

int main(int argc, char** argv)
{
    loadObjects(argc, argv);
    
#ifdef PROMPT
    getSceneInput();  //get scene input from user
#endif
    
    glutInit(&argc, argv);
#ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DEPTH);
#else
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitContextVersion (3, 2);
    glutInitContextFlags (GLUT_FORWARD_COMPATIBLE);
#endif
    glutInitWindowSize(ww, wh);
    glutInitWindowPosition(500, 300);
    glutCreateWindow("Simple Open GL Program");
    printf("%s\n%s\n", glGetString(GL_RENDERER), glGetString(GL_VERSION));
    
#ifndef __APPLE__
    glewExperimental = GL_TRUE;
    glewInit();
#endif
    
    init();
    
    //NOTE:  callbacks must go after window is created!!!
    glutMouseFunc(myMouse);
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(myReshape);
    glutDisplayFunc(display);
    glutMotionFunc(drag);

    glutMainLoop();
    
    return(0);
}

