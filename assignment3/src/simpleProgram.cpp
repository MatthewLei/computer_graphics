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
float eyex = -1.2;
float eyey = -.2;
float eyez = 1;
float atx = 0;
float aty = 0;
float atz = 0;
float upx = 0;
float upy = 1;
float upz = 0;
float yfov = 65;
float aspRatio = 1;
float near = .5;
float far = 4;
float o_left = -1;
float o_right = 1;
float o_bottom = -1;
float o_top = 1;
#endif

#ifdef PROMPT
float eyex, eyey, eyez, atx, aty, atz, upx, upy, upz;
float near, far;
float yfov, aspRatio;
float o_left, o_right, o_bottom, o_top;
#endif

const bool VERBOSE = true;
std::vector < struct object > objects;
int currentHeight = 0;
int clicked_object = 0;

GLuint  model_view;  // model-view matrix uniform shader variable location
GLuint  projection; // projection matrix uniform shader variable location


point4 points[NumVertices];
vec4   normals[NumVertices];

struct object {
    int unique_id;
    vec4 color;
    char *fileName;
    GLuint vao;
    std::vector < vec4 > out_vertices;
    std::vector < vec4 > out_normals;
    int objNumVertices;
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
    for (int i = 0; i < objects.size(); i++) {
        if (VERBOSE) {
            fprintf(stderr, "Number of vertices for %s: %d\n", objects.at(i).fileName, objects.at(i).objNumVertices);
        }
        
        // Create a vertex array object
        glGenVertexArrays( 1, &objects.at(i).vao );
        glBindVertexArray( objects.at(i).vao );
        
        // Create and initialize a buffer object
        GLuint buffer;
        glGenBuffers( 1, &buffer );
        glBindBuffer( GL_ARRAY_BUFFER, buffer );
        glBufferData( GL_ARRAY_BUFFER, (objects.at(i).out_vertices.size() * sizeof(vec4)) + (objects.at(i).out_normals.size() * sizeof(vec4)),
                     &objects.at(i).out_vertices[0], GL_STATIC_DRAW );
        glBufferSubData( GL_ARRAY_BUFFER, 0, objects.at(i).out_vertices.size() * sizeof(vec4), &objects.at(i).out_vertices[0] );
        glBufferSubData( GL_ARRAY_BUFFER, objects.at(i).out_vertices.size() * sizeof(vec4), objects.at(i).out_normals.size() * sizeof(vec4), &objects.at(i).out_normals[0] );
        
        // set up vertex arrays
        GLuint vPosition = glGetAttribLocation( program, "vPosition" );
        glEnableVertexAttribArray( vPosition );
        glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
                              BUFFER_OFFSET(0) );
        
        GLuint vNormal = glGetAttribLocation( program, "vNormal" );
        glEnableVertexAttribArray( vNormal );
        glVertexAttribPointer( vNormal, 4, GL_FLOAT, GL_FALSE, 0,
                              BUFFER_OFFSET(objects.at(i).out_vertices.size() * sizeof(vec4)) );
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
    
    
    mat4 p;
    if (projType == "perspective") {
        p = Perspective(yfov, aspRatio, near, far);
    } else if (projType == "orthographic") {
        p = Ortho(o_left, o_right, o_bottom, o_top, near, far);
    }
    point4  eye( eyex, eyey, eyez, 1);
    point4  at( atx, aty, atz, 1.0 );
    vec4    up( upx, upy, upz, 0.0 );
    
    mat4  mv = LookAt( eye, at, up );
    
    glUniformMatrix4fv( model_view, 1, GL_TRUE, mv );
    glUniformMatrix4fv( projection, 1, GL_TRUE, p );
    
    
    glEnable( GL_DEPTH_TEST );
    glClearColor( 1.0, 1.0, 1.0, 1.0 );
}

//----------------------------------------------------------------------------

void
display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    for (int i = 0; i < objects.size(); i++) {
        glBindVertexArray( objects.at(i).vao );
        if (objects.at(i).unique_id == clicked_object) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glPolygonOffset(1, 2 );
            glDrawArrays( GL_TRIANGLES, 0, objects.at(i).objNumVertices );
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glDrawArrays( GL_TRIANGLES, 0, objects.at(i).objNumVertices );
        }
        glutSwapBuffers();

    }
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
    }
}

//----------------------------------------------------------------------------

void myMouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glDrawBuffer(GL_BACK);
        glReadBuffer(GL_BACK);
        //for each object, draw in ID mode
        for (int i = 0; i < objects.size(); i++) {
            glBindVertexArray(objects.at(i).vao);
            
            //change buffer to render ID color
            int choiceId = glGetUniformLocation(program, "choice");
            glUniform1i(choiceId, 1);
            GLuint objectColorId = glGetUniformLocation(program, "objectColor");
            glUniform4f(objectColorId, objects.at(i).color.x, objects.at(i).color.y, objects.at(i).color.z, objects.at(i).color.z);
            glDrawArrays( GL_TRIANGLES, 0, objects.at(i).objNumVertices );
        }
        //capture the object/color clicked
        GLubyte color[4];
        glReadPixels(x, currentHeight - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, (void *)color);
//        printf("clicked color %d %d %d %d\n", color[0], color[1], color[2], color[3]);
        if ((color[0] == 255) || (clicked_object == color[0])) {
            clicked_object = 0;
        } else {
            clicked_object = color[0];
        }
//        printf("clicked object is now %d\n", clicked_object);
        //rerender original objects
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        int choiceId = glGetUniformLocation(program, "choice");
        glUniform1i(choiceId, 0);
        for (int i = 0; i < objects.size(); i++) {
            glBindVertexArray(objects.at(i).vao);
            if (objects.at(i).unique_id == clicked_object) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glPolygonOffset(1, 2 );
                glDrawArrays( GL_TRIANGLES, 0, objects.at(i).objNumVertices );
            } else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glDrawArrays( GL_TRIANGLES, 0, objects.at(i).objNumVertices );
            }
            
        }
        
        //clean up
        glReadBuffer(GL_FRONT);
        glDrawBuffer(GL_FRONT);
        glutSwapBuffers();
    }
}

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



//window reshape function
void myReshape(int w, int h)
{
//    cout << "before reshape: " << glGetUniformLocation(program, "Projection") << endl;
    currentHeight = h;
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
        r = (i & 0x000000FF) >> 0;
        g = (i & 0x0000FF00) >> 8;
        b = (i & 0x00FF0000) >> 16;
        tmp_obj.color.x = r / 256.0f;
        tmp_obj.color.y = g / 256.0f;
        tmp_obj.color.z = b / 256.0f;
        tmp_obj.color.w = 1;
        cout << "tmp_obj.color.x = " << tmp_obj.color.x << endl;
        cout << "tmp_obj.color.y = " << tmp_obj.color.y << endl;
        cout << "tmp_obj.color.z = " << tmp_obj.color.z << endl;
        
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
//        if (VERBOSE) {
//            printf("size of out_vertices: %d\n", (int)out_vertices.size());
//            for (int i = 0; i < out_vertices.size(); i++) {
//                fprintf(stderr, "final vec: %f %f %f %f\n", out_vertices[i].x, out_vertices[i].y, out_vertices[i].z, out_vertices[i].w);
//            }
//            printf("size of out_normals: %d\n", (int)out_normals.size());
//            for (int i = 0; i < out_normals.size(); i++) {
//                fprintf(stderr, "final norm: %f %f %f %f\n", out_normals[i].x, out_normals[i].y, out_normals[i].z, out_normals[i].w);
//            }
//        }
        
        tmp_obj.out_normals = out_normals;
        tmp_obj.out_vertices = out_vertices;
        tmp_obj.objNumVertices = out_vertices.size();
        objects.push_back(tmp_obj);
        
        if (VERBOSE) {
            cout << "Success" << endl;
        }
    }
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

    glutMainLoop();
    
    return(0);
}

