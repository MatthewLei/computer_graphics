// Assignment 1: Matthew Lei, Xin Yang Chen
//
// A simple 2D OpenGL program

#define GL3_PROTOTYPES

// Include the vector and matrix utilities from the textbook, as well as some
// macro definitions.
#include "../include/Angel.h"

#ifdef __APPLE__
#  include <OpenGL/gl3.h>
#endif

#include <stdio.h>

//number of points for water object
const int waterPoints = 18;
const int squarePoints = 12;
const int pointPoints = 4;

GLuint vao1;
GLuint vao2;
GLuint vao3;


void init()
{
    // Create and link the shaders into a program
    GLuint shaders = InitShader("vertshader.glsl", "fragshader.glsl");

    //bind the shader
    glUseProgram(shaders);

    // water object
    //generally drawn from top-left to bottom-right
    vec2 water[] = {
        vec2(-0.5, .90),
        vec2(-0.7, 0.60),
        vec2(-0.3, 0.60),

        vec2(-0.7, 0.60),
        vec2(-0.3, 0.60),
        vec2(-0.7, 0.32),

        vec2(-0.3, 0.60),
        vec2(-0.7, 0.32),
        vec2(-0.3, 0.32),

        vec2(-0.7, 0.32),
        vec2(-0.3, 0.32),
        vec2(-0.6, 0.20),
    
        vec2(-0.3, 0.32),
        vec2(-0.6, 0.20),
        vec2(-0.4, 0.20),
        
        vec2(-0.65, 0.60),
        vec2(-0.55, 0.75),
        vec2(-0.65, 0.40),
    };
    
    //water object color (per vertex color)
    vec4 waterColors[] = {
        vec4(0.0, 0.75, 1.0, 1.0),
        vec4(0.0, 0.75, 1.0, 1.0),
        vec4(0.0, 0.0, 1.0, 1.0),
        
        vec4(0.0, 0.75, 1.0, 1.0),
        vec4(0.0, 0.0, 1.0, 1.0),
        vec4(0.0, 0.0, 1.0, 1.0),
        
        vec4(0.0, 0.0, 1.0, 1.0),
        vec4(0.0, 0.0, 1.0, 1.0),
        vec4(0.0, 0.0, 0.0, 1.0),
        
        vec4(0.0, 0.0, 1.0, 1.0),
        vec4(0.0, 0.0, 0.0, 1.0),
        vec4(0.0, 0.0, 0.0, 1.0),
        
        vec4(0.0, 0.0, 0.0, 1.0),
        vec4(0.0, 0.0, 0.0, 1.0),
        vec4(0.0, 0.0, 0.0, 1.0),
        
        vec4(0.80, 0.80, 1.0, 1.0),
        vec4(0.80, 0.80, 1.0, 1.0),
        vec4(0.25, 0.25, 1.0, 1.0),
        
    };

    //square line object
    vec2 square[] = {
        vec2(0.25, 0.25),
        vec2(0.75, 0.25),
        
        vec2(0.75, 0.25),
        vec2(0.75, 0.75),
        
        vec2(0.75, 0.75),
        vec2(0.25, 0.75),
        
        vec2(0.25, 0.75),
        vec2(0.25, 0.25),
        
        vec2(0.25, 0.25),
        vec2(0.75, 0.75),
        
        vec2(0.75, 0.25),
        vec2(0.25, 0.75),
    };
    
    //point object
    vec2 point[] = {
        vec2(0.375, 0.5),
        vec2(0.5, 0.375),
        vec2(0.625, 0.5),
        vec2(0.5, 0.625),
    };

    GLuint vPositionLoc = glGetAttribLocation(shaders, "vPosition");
    GLuint vColorLoc = glGetAttribLocation(shaders, "vColor");
    
    //create VAO1 (water object)
    glGenVertexArrays(1, &vao1);
    //bind VAO to context
    glBindVertexArray(vao1);
    
    //create VBO and bind water object to buffer[0]
    GLuint buffer[2];
    glGenBuffers(2,buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(water), water, GL_STATIC_DRAW);
    //binding color to buffer[1]
    glBindBuffer(GL_ARRAY_BUFFER, buffer[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(waterColors), waterColors, GL_STATIC_DRAW);
    
    // Get/enable position and color attribute locations from the vertex shader
    glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
    glEnableVertexAttribArray(vPositionLoc);
    glVertexAttribPointer(vPositionLoc, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    glBindBuffer(GL_ARRAY_BUFFER, buffer[1]);
    glEnableVertexAttribArray(vColorLoc);
    glVertexAttribPointer(vColorLoc, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    
    //setup for the line object
    //bind vao2 and line object to VBO (buffer2)
    glGenVertexArrays(1, &vao2);
    glBindVertexArray(vao2);
    GLuint buffer2;
    glGenBuffers(1, &buffer2);
    glBindBuffer(GL_ARRAY_BUFFER, buffer2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW);
    //set attributes and position
    glEnableVertexAttribArray(vPositionLoc);
    glVertexAttribPointer(vPositionLoc, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    
    //setup for point object
    //bind vao3 and point object to VBO (buffer3)
    glGenVertexArrays(1, &vao3);
    glBindVertexArray(vao3);
    GLuint buffer3;
    glGenBuffers(1, &buffer3);
    glBindBuffer(GL_ARRAY_BUFFER, buffer3);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point), point, GL_STATIC_DRAW);
    //set attributes and position
    glEnableVertexAttribArray(vPositionLoc);
    glVertexAttribPointer(vPositionLoc, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    
    //background is black
    glClearColor(1.0, 1.0, 1.0, 1);
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27:
        // Quit when ESC is pressed
        exit(EXIT_SUCCESS);
        break;
    }
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    //draw water object
    glBindVertexArray(vao1);
    glDrawArrays(GL_TRIANGLES, 0, waterPoints);
    //draw line object
    glBindVertexArray(vao2);
    glDrawArrays(GL_LINES, 0, squarePoints);
    //draw point object
    glBindVertexArray(vao3);
    glDrawArrays(GL_POINTS, 0, squarePoints);

    glFlush();
    glutSwapBuffers();
}


//------------------------------------------------------------------------------
// This program draws a red rectangle on a white background, but it's still
// missing the machinery to move to 3D.
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
#ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE);
#else
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitContextVersion (3, 2);
    glutInitContextFlags (GLUT_FORWARD_COMPATIBLE);
#endif
    glutInitWindowSize(720, 720);
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
    glutDisplayFunc(display);
    glutMainLoop();

    return(0);
}
