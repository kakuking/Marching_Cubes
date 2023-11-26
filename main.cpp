#include <GL/glut.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <stdlib.h>

static int slices = 16;
static int stacks = 16;

glm::vec3 initialCameraPos(0.0f, 0.0f, 3.0f);      // Initial pos
glm::vec3 initialCameraTarget(0.0f, 0.0f, 0.0f);   // Initial target
glm::vec3 initialUpVector(0.0f, 1.0f, 0.0f);       // Initial Up vector

glm::vec3 cameraPos(0.0f, 0.0f, 3.0f);      // pos
glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);   // target
glm::vec3 upVector(0.0f, 1.0f, 0.0f);       // Up vector
glm::mat4 viewMatrix;

float moveSpeed = 0.5f;
float rotateSpeed = 0.09f;

float yawAngle = 0.0f;      // n axis
float pitchAngle = 0.0f;    // u axis
float rollAngle = 0.0f;     // v axis

float grid[32][32][32];
float gridVerts[32][32][32][3];

/* GLUT callback Handlers */
void initCam()
{
    yawAngle = 0.0f;
    pitchAngle = 0.0f;
    rollAngle = 0.0f;

    cameraPos = initialCameraPos;
    cameraTarget = initialCameraTarget;
    upVector = initialUpVector;

    viewMatrix = glm::lookAt(cameraPos, cameraTarget, upVector);

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(viewMatrix));
}

// Takes increment in yawAngle, pitchAngle, and rollAngle and then calculates new position of camera
// calls lookAt()
void updateView()
{
    glm::vec3 n = glm::normalize(cameraPos - cameraTarget);
    glm::vec3 u = glm::normalize(glm::cross(upVector, n));
    glm::vec3 v = glm::cross(n, u);

    glm::quat yawQuat = glm::angleAxis(yawAngle, upVector);
    glm::quat pitchQuat = glm::angleAxis(pitchAngle, u);
    glm::quat rollQuat = glm::angleAxis(rollAngle, n);

    glm::quat rotationQuat = yawQuat * pitchQuat * rollQuat;
    rotationQuat = glm::normalize(rotationQuat);

    glm::mat4 rotationMat = glm::mat4_cast(rotationQuat);

    cameraTarget = cameraPos - glm::vec3(rotationMat * glm::vec4(n, 0.0f));
    upVector = glm::vec3(rotationMat * glm::vec4(upVector, 0.0f));

    glm::mat4 viewMatrix = glm::lookAt(cameraPos, cameraTarget, upVector);

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(viewMatrix));

    // Resets angles so that angles dont add up
    yawAngle = 0.0f;
    pitchAngle = 0.0f;
    rollAngle = 0.0f;
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

static void display(void)
{
    const double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    const double a = t*90.0;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3d(1,0,0);

    glBegin(GL_POINTS);
        for(int i = 0; i < 32; i++){
            for(int j = 0; j < 32; j++){
                for(int k = 0; k < 32; k++){
                    glColor3f(grid[i][j][k],grid[i][j][k],0);
                    glVertex3f(gridVerts[i][j][k][0], gridVerts[i][j][k][1], gridVerts[i][j][k][2]);
                }
            }
        }

    glEnd();
    

    glutSwapBuffers();
}


static void key(unsigned char key, int x, int y)
{
    glm::vec3 moveBy;
    glm::mat4 viewMatrix;

    glm::vec3 n = glm::normalize(cameraPos - cameraTarget);
    glm::vec3 u = glm::normalize(glm::cross(upVector, n));
    glm::vec3 v = glm::cross(n, u);

    switch (key)
    {
        case 27 :
        case 'q':
            exit(0);
            break;

        // Forwards or backwards
        case 'w':
            moveBy = moveSpeed * n;//(cameraTarget - cameraPos);
            cameraPos -= moveBy;
            cameraTarget -= moveBy;
            //upVector -= moveBy;

            viewMatrix = glm::lookAt(cameraPos, cameraTarget, upVector);
            glMatrixMode(GL_MODELVIEW);
            glLoadMatrixf(glm::value_ptr(viewMatrix));
            //updateView();
            break;
        case 's':
            moveBy = moveSpeed * n;// (cameraTarget - cameraPos);
            cameraPos += moveBy;
            cameraTarget += moveBy;
            //upVector += moveBy;

            viewMatrix = glm::lookAt(cameraPos, cameraTarget, upVector);
            glMatrixMode(GL_MODELVIEW);
            glLoadMatrixf(glm::value_ptr(viewMatrix));
            //updateView();
            break;

        // Left and right
        case 'a':
            moveBy = moveSpeed * u;//glm::normalize(glm::cross(upVector, cameraTarget - cameraPos));
            cameraPos -= moveBy;
            cameraTarget -= moveBy;
            //upVector -= moveBy;

            viewMatrix = glm::lookAt(cameraPos, cameraTarget, upVector);
            glMatrixMode(GL_MODELVIEW);
            glLoadMatrixf(glm::value_ptr(viewMatrix));
            //updateView();
            break;
        case 'd':
            moveBy = moveSpeed * u;//glm::normalize(glm::cross(upVector, cameraTarget - cameraPos));
            cameraPos += moveBy;
            cameraTarget += moveBy;
            //upVector += moveBy;

            viewMatrix = glm::lookAt(cameraPos, cameraTarget, upVector);
            glMatrixMode(GL_MODELVIEW);
            glLoadMatrixf(glm::value_ptr(viewMatrix));
            //updateView();
            break;

        // Up and Down
        case 't':
            moveBy = moveSpeed * v;//glm::normalize(glm::cross(upVector, cameraTarget - cameraPos));
            cameraPos += moveBy;
            cameraTarget += moveBy;
            //upVector += moveBy;

            viewMatrix = glm::lookAt(cameraPos, cameraTarget, upVector);
            glMatrixMode(GL_MODELVIEW);
            glLoadMatrixf(glm::value_ptr(viewMatrix));
            //updateView();
            break;
        case 'g':
            moveBy = moveSpeed * v;//glm::normalize(glm::cross(upVector, cameraTarget - cameraPos));
            cameraPos -= moveBy;
            cameraTarget -= moveBy;
            //upVector -= moveBy;

            viewMatrix = glm::lookAt(cameraPos, cameraTarget, upVector);
            glMatrixMode(GL_MODELVIEW);
            glLoadMatrixf(glm::value_ptr(viewMatrix));
            //updateView();
            break;

        // Yaw Angle
        case '1':
            yawAngle = rotateSpeed;
            updateView();
            break;
        case '4':
            yawAngle = -rotateSpeed;
            updateView();
            break;

        // Pitch rotation
        case '2':
            pitchAngle = rotateSpeed;
            updateView();
            break;
        case '5':
            pitchAngle = -rotateSpeed;
            updateView();
            break;

        // Roll Rotation
        case '3':
            rollAngle = rotateSpeed;
            updateView();
            break;
        case '6':
            rollAngle = -rotateSpeed;
            updateView();
            break;

        // Resets camera
        case 'r':
            initCam();
            break;

    }

    glutPostRedisplay();
}

static void idle(void)
{
    glutPostRedisplay();
}

const GLfloat light_ambient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
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
    glutInitWindowSize(640,480);
    glutInitWindowPosition(10,10);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

    glutCreateWindow("GLUT Shapes");

    glutReshapeFunc(resize);
    glutDisplayFunc(display);
    glutKeyboardFunc(key);
    glutIdleFunc(idle);

    glClearColor(0,0,0,1);

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

    float radius = 2.f;
    float centerX = -3 + 16.0/5.0;
    float centerY = -3 + 16.0/5.0;
    float centerZ = -3 - 16.0/5.0;
    for(int i = 0; i < 32; i++){
        for(int j = 0; j < 32; j++){
            for(int k = 0; k < 32; k++){
                float x = -3 + i/5.0;
                float y = -3 + j/5.0;
                float z = -3 - k/5.0;
                gridVerts[i][j][k][0] = x;
                gridVerts[i][j][k][1] = y;
                gridVerts[i][j][k][2] = z;

                // grid[32*32*i + 32*j + k] =  (-3 - i/5)/-9.4;
                float delX = x - centerX;
                float delY = y - centerY;
                float delZ = z - centerZ;
                float dis = sqrtf(powf(delX, 2.f) + powf(delY, 2.f) + powf(delZ, 2.f));
                grid[i][j][k] = dis <= radius ? 1 : 0;
            }
        }
    }

    glutMainLoop();

    return EXIT_SUCCESS;
}
