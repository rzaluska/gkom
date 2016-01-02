/*
*/

#ifdef _WIN32
#include <windows.h>
#include "glut.h"
#endif
#include <stdio.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <string.h>
#include "gravity.h"
#include "math3d.h"
#include "camera.h"
#include "gui.h"
#include "shader.h"
#include "object.h"
#include "skybox.h"
#include "SOIL/SOIL.h"

char keys[1024];
int width = 800;
int height = 600;
int fov = 60;
GLuint program;
GLint attribute_coord;
GLint attribute_color;
GLint mvp;
GLuint vbo_cube_vertices, vbo_cube_colors , ibo_cube_elements;
Matrix4f projection;
struct Camera * camera;
struct Skybox * skybox;
GLuint sp;
GLuint s_mvp;
GLuint s_model;
GLuint s_coord;
GLuint stc;
GLuint s_t;
struct Mesh * sphere;
int mode = 0;
GLuint texture;
struct Object * sun;

void init()
{
    sun = object_init();
    sun->program_id = compile_program("shaders/planet.vert", "shaders/planet.frag");
    sun->mesh = create_sphere_Mesh();
    sun->attribute_coord = glGetAttribLocation(sun->program_id, "coord");
    sun->uniform_mvp = glGetUniformLocation(sun->program_id, "mvp");
    sun->uniform_model = glGetUniformLocation(sun->program_id, "model");
    sun->uniform_texture = glGetUniformLocation(sun->program_id, "ourTexture");

    glGenTextures(1, &texture);
    sun->texture_id = texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    int width, height;
    unsigned char* image = SOIL_load_image("res/sun.jpg", &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);

    camera = default_Camera();
    glPointSize(30);
    glEnable(GL_DEPTH_TEST);
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
    skybox = init_Skybox();
    program = compile_program("shaders/shader.vert", "shaders/shader.frag");
    attribute_coord= glGetAttribLocation(program, "coord");

    attribute_color= glGetAttribLocation(program, "color");
    mvp = glGetUniformLocation(program, "mvp");

    GLfloat cube_vertices[] = {
        -1.0, -1.0,  1.0,
        1.0, -1.0,  1.0,
        1.0,  1.0,  1.0,
        -1.0,  1.0,  1.0,

        -1.0, -1.0, -1.0,
        1.0, -1.0, -1.0,
        1.0,  1.0, -1.0,
        -1.0,  1.0, -1.0,
    };
    glGenBuffers(1, &vbo_cube_vertices);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_vertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    GLfloat cube_colors[] = {
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0,
        1.0, 1.0, 1.0,

        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0,
        1.0, 1.0, 1.0,
    };
    glGenBuffers(1, &vbo_cube_colors);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_colors);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_colors), cube_colors, GL_STATIC_DRAW);


    /* init_resources */
    GLushort cube_elements[] = {
        0, 1, 2,
        2, 3, 0,
        1, 5, 6,
        6, 2, 1,
        7, 6, 5,
        5, 4, 7,
        4, 0, 3,
        3, 7, 4,
        4, 5, 1,
        1, 0, 4,
        3, 2, 6,
        6, 7, 3,
    };
    glGenBuffers(1, &ibo_cube_elements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cube_elements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);

}


float avg = 0;
float avgn = 0;

static float frame_no = 0;
void display()
{
    glClearColor(0, 0 , 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int start = glutGet(GLUT_ELAPSED_TIME);

    static int f = 0;

    Matrix4f m;
    get_camera_matrix(camera, m);

    render_Skybox(skybox, m);

    Matrix4f model = {0};
    model [0] =1;
    model[5] = 1;
    model[10] = 1;
    model[15] = 1;
    model[14] = 10;
    Matrix4f mmm;
    mat4f_mul(m, model, mmm);
    glUseProgram(program);
    glUniformMatrix4fv(mvp, 1 ,GL_FALSE, mmm);
    glEnableVertexAttribArray(attribute_coord);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_vertices);
    glVertexAttribPointer(
            attribute_coord, // attribute
            3,                 // number of elements per vertex, here (x,y)
            GL_FLOAT,          // the type of each element
            GL_FALSE,          // take our values as-is
            0,                 // no extra data between each position
            0 // pointer to the C array
            );

    glEnableVertexAttribArray(attribute_color);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_colors);
    glVertexAttribPointer(
            attribute_color, // attribute
            3,                 // number of elements per vertex, here (x,y)
            GL_FLOAT,          // the type of each element
            GL_FALSE,          // take our values as-is
            0,                 // no extra data between each position
            0 // pointer to the C array
            );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cube_elements);
    int size;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);

    glDisableVertexAttribArray(attribute_coord);
    glDisableVertexAttribArray(attribute_color);

    f+=1;
    Matrix4f rotation1 = {};
    struct Vector4f axis = {1,0,0,1};
    mat4f_rot(rotation1, &axis, 90);
    axis.x = 0;
    axis.y = 1;
    axis.z = 0;
    Matrix4f rotation2;
    mat4f_rot(rotation2, &axis, f*4);
    mat4f_mul(rotation2, rotation1, rotation1);
    Matrix4f model2 = {0};
    model2[0] = 1;
    model2[5] = 1;
    model2[10] = 1;
    model2[12] = 5;
    model2[15] = 1;
    mat4f_mul(model2, rotation1, model2);

    Matrix4f rotation3;
    mat4f_rot(rotation3, &axis, f*0.5);
    mat4f_mul(rotation3, model2, model2);

    Matrix4f mmm2;
    mat4f_mul(m, model2, mmm2);
    object_render(sun, mmm2, model2);

    char hello[4096];
    sprintf ( hello, "FPS: %f\nRadek\n123", avg);

    render_text(0, height-18, hello, width, height);

    glFlush();
    glutSwapBuffers(); //bufory zalezne od systemu

    int end  = glutGet(GLUT_ELAPSED_TIME);
    int delta = end - start;
    if (delta == 0)
        delta = 1;
    avgn += (1000/((float)delta*100));
    frame_no ++;
    if (frame_no > 100) {
        avg = avgn;
        avgn = 0;
        frame_no = 0;
    }
    if (keys['w']) {
        camera_move_forward(camera, 0.05);
    }
    if (keys['s']) {
        camera_move_backward(camera, 0.05);
    }
    if (keys['a']) {
        camera_move_left(camera, 0.05);
    }
    if (keys['d']) {
        camera_move_right(camera, 0.05);
    }
    if (keys['q'] || keys['e']) {
        float delta= keys['e'] ? 0.5 : -0.5;
        camera_roll(camera, delta);
    }
    static int ok = 0;
    if (keys['m'] && ok == 0) {
        if (mode == 0) {
            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            mode = 1;
        }
        else if (mode == 1) {
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
            mode = 0;
        }
        ok = 1;
    }
    if (keys['m'] == 0)
        ok = 0;
}

int spin = 0;
int startx;
int starty;
static int s = 0;
static float sx;
static float sy;
void mouseFunc(int button, int state, int x, int y) {
    if (button == 4) {
        camera->fov += 1;
        if (camera->fov > 179)
            camera->fov = 179;
    }
    else if (button == 3) {
        camera->fov -= 1;
        if (camera->fov < 1)
            camera->fov = 1;
    }
    else if (button == 2) {
        if (state == GLUT_DOWN && s == 0) {
            glutWarpPointer(width/2, height/2);
            sx = width/2;
            sy = height/2;
            s = 1;
        }
        if (state == GLUT_UP) {
            s = 0;
        }
    }
}

void MotionFunc(int x, int y)
{
    static int wrapped = 0;
    if (s == 1 && !wrapped) {
        float delta = (sx - x)*0.1;
        Matrix4f m;
        mat4f_rot(m, &camera->frame.forward, delta);
        mat4f_vec_mul(m, &camera->frame.up);
        vec4f_normalize(&camera->frame.up);
        sx = width/2;
        sy = height/2;
        glutWarpPointer(width/2, height/2);
        wrapped = 1;
    }
    else
        wrapped = 0;
}
void passiveMotionFunc(int x, int y)
{
    static int wrapped = 0;
    if (!wrapped && s == 0) {
        wrapped = 1;
        int middlex = width/2;
        int middley = height/2;
        float deltax = (x - middlex)*0.1;
        float deltay = (middley - y)*0.1;
        Matrix4f m;
        mat4f_rot(m, &camera->frame.up, deltax);
        mat4f_vec_mul(m, &camera->frame.forward);

        struct Vector4f right;
        vec4f_cross(&camera->frame.up, &camera->frame.forward, &right);

        mat4f_rot(m, &right, deltay);
        mat4f_vec_mul(m, &camera->frame.forward);

        vec4f_cross(&camera->frame.forward, &right, &camera->frame.up);
        vec4f_normalize(&camera->frame.up);

        glutWarpPointer(width/2, height/2);
    }
    else
        wrapped = 0;
}

void processNormalKeys(unsigned char key, int xx, int yy) {
    if (key == 27)
        exit(0);
    keys[key] = 1;
}
void processNormalKeysUp(unsigned char key, int xx, int yy) {
    keys[key] = 0;
}
void reshape(GLsizei w, GLsizei h)
{
    if( h > 0 && w > 0 ) {
        glViewport( 0, 0, w, h );
        camera->width = w;
        camera->height = h;
    }
}
int main(int argc, char** argv)
{
    glutInit( &argc, argv );

    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glEnable(GL_MULTISAMPLE);

    glutInitWindowPosition( 0, 0 );
    glutInitWindowSize( width , height );

    glutCreateWindow( "OpenGL" );

    glutSetCursor(GLUT_CURSOR_NONE);
    glutDisplayFunc( display );
    glutReshapeFunc( reshape );
    glutIdleFunc( display );
    glutMouseFunc( mouseFunc );
    glutKeyboardFunc(processNormalKeys);
    glutKeyboardUpFunc(processNormalKeysUp);
    glutPassiveMotionFunc(passiveMotionFunc);
    glutMotionFunc(MotionFunc);

    glewExperimental = GL_TRUE ;
    GLint status;
    if ((status = glewInit()) != GLEW_OK)
    {
        printf("%d\n", status);
        return -1;
    }
    init();
    glutMainLoop();

    return 0;
}
