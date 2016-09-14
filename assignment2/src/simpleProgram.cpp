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

#ifdef __APPLE__
#  include <OpenGL/gl3.h>
#endif

using namespace std;

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;
const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

std::string projType = "";
float eyex, eyey, eyez, atx, aty, atz, upx, upy, upz;
float yfov, aspRatio;
float near, far;
float o_left, o_right, o_bottom, o_top;
std::string PATH = "";
const bool VERBOSE = false;
GLuint vao;
int myNumVertices = 0;

GLuint  model_view;  // model-view matrix uniform shader variable location
GLuint  projection; // projection matrix uniform shader variable location


point4 points[NumVertices];
vec4   normals[NumVertices];

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
init(std::vector<vec4> out_vertices, std::vector<vec4> out_normals)
{
    if (((int)out_vertices.size() < 1) && ((int)out_normals.size() < 1)) {
        colorcube();
    }
    if (VERBOSE) {
        printf("Init() running...\n");
        fprintf(stderr, "Number of vertices: %d\n", myNumVertices);
    }
    
    // Create a vertex array object
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
    
    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, (out_vertices.size() * sizeof(vec4)) + (out_normals.size() * sizeof(vec4)),
                 &out_vertices[0], GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, out_vertices.size() * sizeof(vec4), &out_vertices[0] );
    glBufferSubData( GL_ARRAY_BUFFER, out_vertices.size() * sizeof(vec4), out_normals.size() * sizeof(vec4), &out_normals[0] );
    
    // Load shaders and use the resulting shader program
    GLuint program = InitShader( "vshader.glsl", "fshader.glsl" );
    glUseProgram( program );
    
    // set up vertex arrays
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(0) );
    
    GLuint vNormal = glGetAttribLocation( program, "vNormal" );
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 4, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(out_vertices.size() * sizeof(vec4)) );
    
    
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
    glBindVertexArray( vao );
    glDrawArrays( GL_TRIANGLES, 0, myNumVertices );
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
    }
}

//----------------------------------------------------------------------------

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

//function for prompting user to choose object
void chooseOjbect()
{
    int option; // user's entered option will be saved in this variable
    do //do-while loop starts here.that display menu again and again until user select to exit program
    {
        //Displaying Options for the menu
        cout << endl;
        cout << "1) load bunnyNS.obj " << endl;
        cout << "2) load sphere42NS.obj " << endl;
        cout << "3) load teapotNS.obj" << endl;
        cout << "4) Exit Program " << endl;
        //Prompting user to enter an option according to menu
        promptInteger("Please select an option by its associated number: ", &option);
        
        if(option == 1) // Checking if user selected option 1
        {
            PATH = "bunnyNS.obj.txt";
            if (VERBOSE) {
                cout << "Selected Bunny!";
                cout << "Note: bunny is smaller than unit cube\n" << endl;
            }
            cout << endl;
            break;
        }
        else if(option == 2) // Checking if user selected option 2
        {
            PATH = "sphere42NS.obj.txt";
            if (VERBOSE) {
                cout << "Selected Sphere!\n";
            }
            cout << endl;
            break;
        }
        else if(option == 3) // Checking if user selected option 3
        {
            PATH = "teapotNS.obj.txt";
            if (VERBOSE) {
                cout << "Selected Teapot!\n";
            }
            cout << endl;
            break;
        }
        else if(option == 4) // Checking if user selected option 4
        {
            cout << "Terminating Program" << endl;
            exit(EXIT_SUCCESS);
        }
        else
        {
            cout << "Invalid Option entered" << endl;
        }
    }
    while(option != 4);  //condition of do-while loop
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
    chooseOjbect();   //prompt user to choose object
    getSceneInput();  //get scene input from user
    
    glutInit(&argc, argv);
#ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DEPTH);
#else
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitContextVersion (3, 2);
    glutInitContextFlags (GLUT_FORWARD_COMPATIBLE);
#endif
    glutInitWindowSize(512, 512);
    glutInitWindowPosition(500, 300);
    glutCreateWindow("Simple Open GL Program");
    printf("%s\n%s\n", glGetString(GL_RENDERER), glGetString(GL_VERSION));
    
#ifndef __APPLE__
    glewExperimental = GL_TRUE;
    glewInit();
#endif
    
    //loader stuff
    std::vector <unsigned int> vertexIndices, normalIndices;
    std::vector <vec4> temp_vertices;
    std::vector <vec4> temp_normals;
    
    FILE * file = fopen(PATH.c_str(), "r");
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
            if (VERBOSE) { fprintf(stderr, "Captured vertex: %f %f %f\n", vertex.x, vertex.y, vertex.z); }
            temp_vertices.push_back(vertex);
            //        }else if ( strcmp( lineHeader, "vt" ) == 0 ){
            //            vec2 uv;
            //            fscanf(file, "%f %f\n", &uv.x, &uv.y );
            //            temp_uvs.push_back(uv);
        }else if ( strcmp( lineHeader, "#" ) == 0){
            fscanf(file, "%[^\n]\n", lineHeader);
            if (VERBOSE) { fprintf(stderr, "Captured comment: %s\n", lineHeader); }
        }else if ( strcmp( lineHeader, "vn" ) == 0 ){
            vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
            if (VERBOSE) { fprintf(stderr, "Captured normal: %f %f %f\n", normal.x, normal.y, normal.z); }
            temp_normals.push_back(normal);
        }else if ( strcmp( lineHeader, "f" ) == 0 ){
            std::string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[3], normalIndex[3];
            int matches = fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2] );
            if (VERBOSE) { fprintf(stderr, "Captured face: %d//%d %d//%d %d//%d\n", vertexIndex[0], normalIndex[0], vertexIndex[1], normalIndex[1], vertexIndex[2], normalIndex[2]); }
            if (matches != 6){
                fprintf(stderr, "Invalid matches (6), only found %d\n", matches);
                printf("File can't be read by our simple parser : ( Try exporting with other options\n");
                return false;
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
    if (VERBOSE) {
        printf("size of out_vertices: %d\n", (int)out_vertices.size());
        for (int i = 0; i < out_vertices.size(); i++) {
            fprintf(stderr, "final vec: %f %f %f %f\n", out_vertices[i].x, out_vertices[i].y, out_vertices[i].z, out_vertices[i].w);
        }
        printf("size of out_normals: %d\n", (int)out_normals.size());
        for (int i = 0; i < out_normals.size(); i++) {
            fprintf(stderr, "final norm: %f %f %f %f\n", out_normals[i].x, out_normals[i].y, out_normals[i].z, out_normals[i].w);
        }
    }
    
    if (VERBOSE) {
        fprintf(stderr, "temp_vertices.size(): %d\n", (int)temp_vertices.size());
        fprintf(stderr, "temp_normal.size(): %d\n", (int)temp_normals.size());
        fprintf(stderr, "vertexIndices.size(): %d\n", (int)vertexIndices.size());
        fprintf(stderr, "normalIndices.size(): %d\n", (int)normalIndices.size());
        fprintf(stderr, "out_vertices.size(): %d\n", (int)out_vertices.size());
        fprintf(stderr, "out_normals.size(): %d\n", (int)out_normals.size());
        
    }
    
    myNumVertices = out_vertices.size();
    init(out_vertices, out_normals);
    
    //NOTE:  callbacks must go after window is created!!!
    glutKeyboardFunc(keyboard);
    glutDisplayFunc(display);
    glutMainLoop();
    
    return(0);
}

