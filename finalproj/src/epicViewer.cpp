#define GL3_PROTOTYPES

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

//prototyping for bad code ordering
void myReshape(int w, int h);

//#define SQUAREVV

//#ifdef SQUAREVV
int ww=1280, wh  = 720;
float vl = -1.0, vr = 1.0, vb = -1.0, vt = 1.0;
float startAR = (vr-vl)/(vt-vb);
//#endif
//screen stuff
int cur_width = ww;
int cur_height = wh;


using namespace std;
typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

#define NO_PROMPT

#ifdef NO_PROMPT
std::string projType = "orthographic";  //perspective or orthographic
vec4 eye(0, 0, 0, 1);
//float eyex = 0;
//float eyey = 0;
//float eyez = 0;

//dont change these
vec3 at;
float cos_atx = (3.0 * M_PI) / 2;
float cos_aty = (3.0 * M_PI) / 2;
float cos_atz = M_PI;
float atx;  //atx = cos(cos_atx)
float aty;  //aty = cos(cos_aty)
float atz;  //atz = cos(cos_atz)
vec3 left_vec, right_vec;  //vectors left and right of at (ignore y)

vec4 up(0, 1, 0, 1);
//float upx = 0;
//float upy = 1;
//float upz = 0;
float yfov = 65;
float aspRatio = 1;
float near = 1;
float far = 100;
//ortho stuff
float o_left = -0.2666;  // (4/15)
float o_right = 0.2666;
float o_bottom = -.15;
float o_top = .15;
#endif

const bool VERBOSE = true;
std::vector < struct object > objs;

//mouse stuff
int init_x_pos = 0;
int init_y_pos = 0;
float prev_x = 0.0f;
float prev_y = 0.0f;

GLuint program;
GLuint model_view;  // model-view matrix uniform shader variable location
GLuint projection; // projection matrix uniform shader variable location
mat4 view, mv;
mat4 matrixIdentity;
mat4 cur_proj;       //orthographic, perspective data

//vec3 look_unit = vec3(0, 0, -1);
vec3 x_axis = vec3(1, 0, 0);
vec3 y_axis = vec3(0, 1, 0);
vec3 z_axis = vec3(0, 0, 1);

//sensitivities/camera stuff
float pace_rate = 2.0f;
float cam_sensitivity = 0.002f;
float zoomed = 0.0;
float zoom_in_rate = 0.95;
float zoom_out_rate = 1.05;

//rendering stuff
int num_shades = 2;
int toon_mode = 0;


struct object {
    int unique_id;
    vec4 color;
    char *fileName;
    GLuint vao;
    std::vector < vec4 > out_vertices;
    std::vector < vec4 > out_normals;
    int obj_num_vert;
    mat4 obj_model;
    float x_trans, y_trans, z_trans;
    float x_trans_manip, y_trans_manip, z_trans_manip;
    float x_rotate, y_rotate, z_rotate;
    float x_scale, y_scale, z_scale;
};

void calc_side_vec(float update = 0)
{
    //calculate right_vec
    right_vec.x = cos(cos_atx + (M_PI / 2) + (update * cam_sensitivity));
    right_vec.z = cos(cos_atz + (M_PI / 2) + (update * cam_sensitivity));
    //calculate left_vec
    left_vec.x = cos(cos_atx - (M_PI / 2) + (update * cam_sensitivity));
    left_vec.z = cos(cos_atz - (M_PI / 2) + (update * cam_sensitivity));
}

void
init()
{
    if (VERBOSE) {
        printf("Init() running...\n");
    }
    
    // Load shaders and use the resulting shader program
    program = InitShader( "toonPhongVshader.glsl", "toonPhongFshader.glsl" );
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
        
        mat4 enlarge;
        mat4 obj_trans;
        if (strcmp(objs.at(i).fileName, "floor.obj.txt") == 0) {
            enlarge = Scale(4, 4, 8);
            objs.at(i).obj_model = enlarge * objs.at(i).obj_model;
            obj_trans = Translate(0, -20, 0);
            objs.at(i).obj_model = obj_trans * objs.at(i).obj_model;
        } else if (strcmp(objs.at(i).fileName, "cow_statue.obj.txt") == 0){
            enlarge = Scale(150, 150, 150);
            objs.at(i).obj_model = enlarge * objs.at(i).obj_model;
            obj_trans = Translate(0, 67, 0);
            objs.at(i).obj_model = obj_trans * objs.at(i).obj_model;
            
        } else if (strcmp(objs.at(i).fileName, "cow.obj.txt") == 0){
            obj_trans = Translate(0, -19, 0);
            objs.at(i).obj_model = obj_trans * objs.at(i).obj_model;
            
        }
        //spawn far away
        mat4 spawn = Translate(0, 0, -1200);
        objs.at(i).obj_model = spawn * objs.at(i).obj_model;
        
    }
    
    // Initialize shader lighting parameters
    // RAM: No need to change these...we'll learn about the details when we
    // cover Illumination and Shading
    point4 light_position( 99, 300, 99, 1.0 );
//    point4 light_position( 1.5, 1.5, 2.0, 1.0 );
    color4 light_ambient( 0.2, 0.2, 0.2, 1.0 );
    color4 light_diffuse( 0.80, 0.80, 0.80, 1.0 );
    color4 light_specular( 1.0, 1.0, 1.0, 1.0 );
    
    color4 material_ambient( 1.0, 0.0, 1.0, 1.0 );
    color4 material_diffuse( 1.0, 0.8, 0.0, 1.0 );
    color4 material_specular( 1.0, 0.8, 0.0, 1.0 );
    float  material_shininess = 100.0;
    
    color4 ambient_product = light_ambient * material_ambient;
    color4 diffuse_product = light_diffuse * material_diffuse;
    color4 specular_product = light_specular * material_specular;
    
    cout << "ambient_product " << ambient_product << endl;
    cout << "specular_product " << specular_product << endl;
    cout << "diffuse_product " << diffuse_product << endl;
    
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
    
//    cur_proj = Perspective(yfov, aspRatio, near, far);
//    cur_proj = Ortho(o_left, o_right, o_bottom, o_top, near, far);
//    cout << "cur_proj from myReshape: " << cur_proj << endl;

    atx = cos(cos_atx);
    aty = cos(cos_aty);
    atz = cos(cos_atz);
    at.x = atx;
    at.y = aty;
    at.z = atz;
//    at.w = 1.0;
    
    calc_side_vec();
    
//    vec4 at(atx, aty, atz, 1.0);
    vec4 tmp_at(at, 1.0);
    view = LookAt( eye, tmp_at, up );
    //    matrixIdentity = identity();
    mv = matrixIdentity;
    
    glUniformMatrix4fv( model_view, 1, GL_TRUE, view );
//    glUniformMatrix4fv( projection, 1, GL_TRUE, cur_proj );
    glUniform1f( glGetUniformLocation(program, "num_shades"), num_shades);
    
    
    glEnable( GL_DEPTH_TEST );
    glClearColor( 0.0, 0.0, 0.0, 1.0 );
}

//----------------------------------------------------------------------------
void sync_object_mv(int i)
{
    mv = matrixIdentity;
    mv = mv * view;
    mv = mv * objs.at(i).obj_model;
    glUniformMatrix4fv( model_view, 1, GL_TRUE, mv );
}

void
display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    for (int i = 0; i < objs.size(); i++) {
        glBindVertexArray( objs.at(i).vao );
        sync_object_mv(i);
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        sync_object_mv(i);
        glDrawArrays( GL_TRIANGLES, 0, objs.at(i).obj_num_vert );
    }
    glutSwapBuffers();
}


//----------------------------------------------------------------------------

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
}

void myRotate(int obj, float x, float y, float z)
{
    objs.at(obj).x_rotate += x;
    objs.at(obj).y_rotate += y;
    objs.at(obj).z_rotate += z;
    //    printf("rotatation: %f, %f, %f\n", objs.at(obj).x_rotate, objs.at(obj).y_rotate, objs.at(obj).z_rotate);

    //initialize new rotation matrix to be applied
    mat4 r;
    r = r * RotateX(x);
    r = r * RotateY(y);
    r = r * RotateZ(z);
    
    objs.at(obj).obj_model = r * objs.at(obj).obj_model;
    
}

void myMouse(int button, int state, int x, int y)
{
//    cout << "atx: " << atx << ", aty: " << aty << ", atz: " << atz << endl;
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
//        GLubyte color[4];
//        int rgb_clicked = 0;
//        glReadPixels(x, cur_height - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, (void *)color);
        init_x_pos = x;
        init_y_pos = y;
        prev_x = 0;
        prev_y = 0;
        
        calc_side_vec();
        
//        printf("initial clicked x: %d, y: %d\n", init_x_pos, init_y_pos);
    } else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        init_x_pos = 0;
        init_y_pos = 0;
        prev_x = 0;
        prev_y = 0;
    }
}


void drag(int x, int y)
{
//    fprintf(stderr, "cos_atx: %.2f, cos_aty: %.2f, cos_atz: %.2f\n", cos_atx, cos_aty, cos_atz);
//    cout << "atx: " << atx << ", aty: " << aty << ", atz: " << atz << endl;
    //get view*projection matrix
    mat4 viewproj = cur_proj * view;
    //calculate the vector drawn by mouse [intial pos - current pos]
    vec3 drawn_vec(x - init_x_pos, init_y_pos - y, 0);  //reverse y for display grid
    
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    //calculate dragged distance since last pull sample
    float x_distance, y_distance;
    float tmp_x, tmp_y;  //temporary holders for incrementing
    x_distance = dot(drawn_vec, x_axis);
    y_distance = dot(drawn_vec, y_axis);
    tmp_x = x_distance;
    tmp_y = y_distance;
    x_distance = x_distance - prev_x;
    y_distance = y_distance - prev_y;
    
    //change view model
    //    mat4 look_lr, look_ud;
    //    look_lr = RotateY(x_distance); //look left/right
    //    look_ud = RotateX(-y_distance); //look up/down
    //    view = look_lr * view;
    //    view = look_ud * view;
    
    //manage z-plane camera movement
    cos_atz += x_distance * cam_sensitivity;
    cos_atx += x_distance * cam_sensitivity;
    
    //manage y-plane camera movement
    if (y_distance > 0) {
        if (cos_aty <= (11 * M_PI / 6)) {
            cos_aty += y_distance * cam_sensitivity;
            cos_atz += y_distance * cam_sensitivity;
        }
    } else if (y_distance < 0) {
        if (cos_aty >= (7 * M_PI / 6)) {
            cos_aty += y_distance * cam_sensitivity;
            cos_atz += y_distance * cam_sensitivity;
        }
    }
    
    //manage overflow
    if (cos_atx > 2 * M_PI) {
        cos_atx -= (2 * M_PI);
    } else if (cos_atx < -2 * M_PI) {
        cos_atx += (2 * M_PI);
    }
    if (cos_atz > 2 * M_PI) {
        cos_atz -= (2 * M_PI);
    } else if (cos_atz < -2 * M_PI) {
        cos_atz += (2 * M_PI);
    }
    
    //update at vec
    atx = cos(cos_atx);
    aty = cos(cos_aty);
    atz = cos(cos_atz);

    //also update left and right vec
    calc_side_vec(x_distance);
    
    at.x = atx;
    at.y = aty;
    at.z = atz;
    vec4 tmp_at(at, 1.0);
    view = LookAt( eye, at, up );
    prev_x = tmp_x;
    prev_y = tmp_y;

    for (int i = 0; i < objs.size(); i++) {
        glBindVertexArray(objs.at(i).vao);
        sync_object_mv(i);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawArrays(GL_TRIANGLES, 0, objs.at(i).obj_num_vert);
        
    }
    glutSwapBuffers();
}

//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
    float walk_x;
    float walk_z;
    mat4 trans;
    switch( key ) {
        case 'a': //left
            walk_x = pace_rate * dot(left_vec, x_axis);
            walk_z = pace_rate * dot(left_vec, z_axis);
            break;
        case 'w': //forward
            walk_x = pace_rate * dot(at, x_axis);
            walk_z = pace_rate * dot(at, z_axis);
            break;
        case 'd': //right
            walk_x = pace_rate * dot(right_vec, x_axis);
            walk_z = pace_rate * dot(right_vec, z_axis);
            break;
        case 's':  //back
            walk_x = pace_rate * dot(-at, x_axis);
            walk_z = pace_rate * dot(-at, z_axis);
//            trans = Translate(0, 0, -.1);
//            view = view * trans;
            break;
        case 033:  // Escape key
        case 'q': case 'Q':
            exit( EXIT_SUCCESS );
            break;
        case '1':
            printf("Phong shader mode\n");
            toon_mode = 0;
            glUniform1i(glGetUniformLocation( program, "toon_mode"), 0);
            glutPostRedisplay();
            break;
        case '2':
            printf("Toon shader mode\n");
            toon_mode = 1;
            glUniform1i(glGetUniformLocation( program, "num_shades"), num_shades);
            glUniform1i(glGetUniformLocation( program, "toon_mode"), 1);
            glutPostRedisplay();
            break;
        case '-': //zoom out
        case '=': //zoom in
            if (key == '-') {
                if (zoomed > 0) { //out
                    o_left *= zoom_out_rate;
                    o_right *= zoom_out_rate;
                    o_top *= zoom_out_rate;
                    o_bottom *= zoom_out_rate;
                    zoomed -= 1;
                }
            } else {
                if (zoomed < 20) {  //in
                    o_left *= zoom_in_rate;
                    o_right *= zoom_in_rate;
                    o_top *= zoom_in_rate;
                    o_bottom *= zoom_in_rate;
                    zoomed += 1;
                }
            }
            //redraw scene
            cur_proj = Ortho(o_bottom * (GLfloat) cur_width / (GLfloat) cur_height, o_top *
                         (GLfloat) cur_width / (GLfloat) cur_height, o_bottom, o_top, near, far);
            glUniformMatrix4fv( projection, 1, GL_FALSE, &cur_proj[0][0] );
            break;
    }
    //redisplay
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    for (int i = 0; i < objs.size(); i++) {
        glBindVertexArray( objs.at(i).vao );
        myTranslate(i, -walk_x, 0, -walk_z);
//        cout << objs.at(i).obj_model << endl;
        sync_object_mv(i);
        glDrawArrays(GL_TRIANGLES, 0, objs.at(i).obj_num_vert);
    }
    glutSwapBuffers();
}

//----------------------------------------------------------------------------

//window reshape function
void myReshape(int w, int h)
{
    cur_width = w;
    cur_height = h;
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
            printf("reshape: left: %f , right: %f\n , top: %f, bottom: %f, near: %f, far: %f\n", o_bottom * (GLfloat) w / (GLfloat) h, o_top * (GLfloat) w / (GLfloat) h, o_top, o_bottom, near, far);
            proj = Ortho(o_bottom * (GLfloat) w / (GLfloat) h, o_top *
                         (GLfloat) w / (GLfloat) h, o_bottom, o_top, near, far);
            
        }
    } else { //perspective FIX THIS TO USE FRUSTUM
        cout << "width: " << w << endl;
        cout << "height: " << h << endl;
        if( ar < startAR) { // (w <= h ){ //taller
            GLfloat tmpLeft = -(near * tan(yfov / 2 * (M_PI/ 180))) * (GLfloat) h / (GLfloat) w;
            GLfloat tmpRight = (near * tan(yfov / 2 * (M_PI/ 180))) * (GLfloat) h / (GLfloat) w;
            GLfloat tmpBottom = tmpLeft * (GLfloat) h / (GLfloat) w;
            GLfloat tmpTop = tmpRight * (GLfloat) h / (GLfloat) w;
            
            cout << "tmpLeft: " << tmpLeft << ", tmpRight: " << tmpRight << ", tmpBottom: " << tmpBottom << ", tmpTop: " << tmpTop << endl;
            proj = Frustum(tmpLeft, tmpRight, tmpBottom, tmpTop, near, far);
        }
        else //wider
        {
            GLfloat tmpBottom = -(near * tan(yfov / 2 * (M_PI/ 180)));
            GLfloat tmpTop = (near * tan(yfov / 2 * (M_PI/ 180)));
            GLfloat tmpLeft = tmpBottom * (GLfloat) w / (GLfloat) h;
            GLfloat tmpRight = tmpTop * (GLfloat) w / (GLfloat) h;
            cout << "tmpLeft: " << tmpLeft << ", tmpRight: " << tmpRight << ", tmpBottom: " << tmpBottom << ", tmpTop: " << tmpTop << endl;
            proj = Frustum(tmpLeft, tmpRight, tmpBottom, tmpTop, near, far);
            
        }
    }
    
    GLuint projectionLoc = glGetUniformLocation(program, "Projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &proj[0][0]);
    cout << "proj from myReshape: " << proj << endl;
    cur_proj = proj;
    
}

//----------------------------------------------------------------------------

void loadObjects(int argc, char **argv)
{
    if (argc <= 1) {
        cout << "Run with at least one obj file:" << endl;
        cout << "./prog [file1.obj.txt] [file2.obj.txt] ..." << endl;
        exit(EXIT_SUCCESS);
    }
    
    //theres some leftover code from prev project
    int r, g, b;
    for(int i = 1; i < argc; i++) {
        struct object tmp_obj;
        tmp_obj.fileName = argv[i];
        tmp_obj.unique_id = i;
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
    
}

//----------------------------------------------------------------------------

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
    
    glutKeyboardFunc(keyboard);
    glutReshapeFunc(myReshape);
    glutDisplayFunc(display);
    glutMouseFunc(myMouse);
    glutMotionFunc(drag);
    
    glutMainLoop();
    
    return(0);
}
