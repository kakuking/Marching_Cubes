#include <GL/glut.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdlib.h>

#include "lookupTable.cpp"

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

float gridSide = 1.0/2.0;

const int numSideVerts = 32;

float grid[numSideVerts][numSideVerts][numSideVerts];
float gridVerts[numSideVerts][numSideVerts][numSideVerts][3];


std::vector<std::vector<float>> triangles;  // So we can pop the last step easily
int curMarch = 0;

float oldVertDeltas[8][3] = {
    {0, 0, 0},
    {1, 0, 0},
    {0, 1, 0},
    {1, 1, 0},
    {0, 0, 1},
    {1, 0, 1},
    {0, 1, 1},
    {1, 1, 1},
};

double newVertDeltas[12][3] = {
    {gridSide/2.0, 0, 0},
    {gridSide/1.0, gridSide/2.0, 0},
    {gridSide/2.0, gridSide/1.0, 0},
    {0, gridSide/2.0, 0},
    {gridSide/2.0, 0, gridSide/1.0},
    {gridSide/1.0, gridSide/2.0, gridSide/1.0},
    {gridSide/2.0, gridSide/1.0, gridSide/1.0},
    {0, gridSide/2.0, gridSide/1.0},
    {0, 0, gridSide/2.0},
    {gridSide/1.0, 0, gridSide/2.0},
    {gridSide/1.0, gridSide/1.0, gridSide/2.0},
    {0, gridSide/1.0, gridSide/2.0},
};

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

void marchForwards(){
    if(curMarch >= (numSideVerts - 1)*(numSideVerts - 1)*(numSideVerts - 1))
        return;
    
    int curI = curMarch/((numSideVerts - 1)*(numSideVerts - 1));
    int curJ = (curMarch - curI*(numSideVerts - 1)*(numSideVerts - 1))/(numSideVerts - 1);
    int curK = (curMarch - curI*(numSideVerts - 1)*(numSideVerts - 1) - curJ*(numSideVerts - 1));

    int idx = 0;
    for(int i = 0; i < 8; i++){
        int idxI = curI + oldVertDeltas[i][0];
        int idxJ = curJ + oldVertDeltas[i][1];
        int idxK = curK + oldVertDeltas[i][2];

        if(grid[idxI][idxJ][idxK] >= 1)
            idx += 1 << i;
    }

    std::vector<float> thisTris; 
    for(int i = 0; i < 13; i++){
        if(lookupTable[idx][i] == -1)
            break;
        // std::cout << lookupTable[idx][i] << " ";
        int curLookupIdx = lookupTable[idx][i];
        float newVertX = gridVerts[curI][curJ][curK][0] + (float)newVertDeltas[curLookupIdx][0];
        float newVertY = gridVerts[curI][curJ][curK][1] + (float)newVertDeltas[curLookupIdx][1];
        float newVertZ = gridVerts[curI][curJ][curK][2] - (float)newVertDeltas[curLookupIdx][2];

        thisTris.push_back(newVertX);
        thisTris.push_back(newVertY);
        thisTris.push_back(newVertZ);

        // std::cout << curLookupIdx << ": " <<  gridVerts[curI][curJ][curK][0] << " " << gridVerts[curI][curJ][curK][1] << " " << gridVerts[curI][curJ][curK][2] << "========== ";
        // std::cout << newVertX << " " << newVertY << " " << newVertZ << std::endl;
    }

    if(thisTris.size() > 0){
        triangles.push_back(thisTris);
        // std::cout << std::endl;
    }

    // std::cout << idx << ": " << lookupTable[idx][0] << std::endl;
    curMarch++;
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
    marchForwards();
    const double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    const double a = t*90.0;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor4f(1,0,0, 0.2);

    // glBegin(GL_POINTS);
    //     for(int i = 0; i < numSideVerts; i++){
    //         for(int j = 0; j < numSideVerts; j++){
    //             for(int k = 0; k < numSideVerts; k++){
    //                 glColor4f(1,grid[i][j][k],0, 0.1f);
    //                 glVertex3f(gridVerts[i][j][k][0], gridVerts[i][j][k][1], gridVerts[i][j][k][2]);
    //             }
    //         }
    //     }
    // glEnd();

    glColor4f(0.0, 0.0, 1.0f, 0.3f);

    glBegin(GL_TRIANGLES);
        for(std::vector<float> tri: triangles){
            int numTri = tri.size()/3;
            for(int i = 0; i < numTri; i++){
                glVertex3f(tri.at(3*i + 0), tri.at(3*i + 1), tri.at(3*i + 2));
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

    // std::cout << (int)key;

    switch (key)
    {
        case 27 :
        case 'q':
            exit(0);
            break;

        case 32 : // space
            marchForwards();
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


std::vector<int> readSkull(){
    // Open the file
    std::ifstream file("skull.txt");

    // Check if the file is open
    if (!file.is_open()) {
        std::cerr << "Error opening file.\n";
        return std::vector<int>(); // Exit with an error
    }

    std::vector<int> numbers; // Use a vector to dynamically store the integers

    // Read each line from the file
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        int number;

        // Read integers from the current line
        while (iss >> number) {
            numbers.push_back(number);
        }
    }

    // Close the file
    file.close();

    return numbers;
}

/* Program entry point */
int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitWindowSize(640,480);
    glutInitWindowPosition(10,10);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

    glutCreateWindow("Marching Cubes");

    glutReshapeFunc(resize);
    glutDisplayFunc(display);
    glutKeyboardFunc(key);
    glutIdleFunc(idle);

    glClearColor(1,1,1,1);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);

    // Initialize grid

    // Makes a random sphere
    float radiusC = 2.2f;
    float centerCX = -3 + 16.0/5.0;
    float centerCY = -3 + 8.0/5.0;
    float centerCZ = -3 - 16.0/5.0;
    for(int i = 0; i < numSideVerts; i++){
        for(int j = 0; j < numSideVerts; j++){
            for(int k = 0; k < numSideVerts; k++){
                
                float x = -3 + i*gridSide;
                float y = -3 + j*gridSide;
                float z = -3 - k*gridSide;
                gridVerts[i][j][k][0] = x;
                gridVerts[i][j][k][1] = y;
                gridVerts[i][j][k][2] = z;

                // grid[numSideVerts*numSideVerts*i + numSideVerts*j + k] =  (-3 - i/5)/-9.4;
                float delX = x - centerCX;
                float delY = y - centerCY;
                float delZ = z - centerCZ;
                float dis = sqrtf(powf(delX, 2.f) + powf(delY, 2.f) + powf(delZ, 2.f));
                grid[i][j][k] = dis <= radiusC ? 1 : 0;                                          // If circle is not first do 1 : grid[i][j][k]
            }
        }
    }

    float radiusT = 2.2f;
    float radiusTubeT = 0.5f;
    float centerTX = -3 + 24.0/5.0;
    float centerTY = -3 + 12.0/5.0;
    float centerTZ = -3 - 16.0/5.0;
    for(int i = 0; i < numSideVerts; i++){
        for(int j = 0; j < numSideVerts; j++){
            for(int k = 0; k < numSideVerts; k++){

                float x = -3 + i*gridSide;
                float y = -3 + j*gridSide;
                float z = -3 - k*gridSide;
                // gridVerts[i][j][k][0] = x;
                // gridVerts[i][j][k][1] = y;
                // gridVerts[i][j][k][2] = z;

                float delX = x - centerTX;
                float delY = y - centerTY;
                float delZ = z - centerTZ;
                float bigCircle = radiusT - sqrtf(powf(delX, 2) + powf(delY, 2));
                float dis = sqrtf(powf(bigCircle, 2.f) + powf(delZ, 2.f));
                grid[i][j][k] = dis <= radiusTubeT ? 1 : grid[i][j][k];                                          // If circle is not first do 1 : grid[i][j][k]
            }
        }
    }

    // So that sides of grid are 0, (closed figures made)
    for(int i = 0; i < numSideVerts; i++){
        for(int j = 0; j < numSideVerts; j++){
            grid[i][j][0] = 0;
            grid[i][j][(numSideVerts - 1)] = 0;
            
            grid[0][i][j] = 0;
            grid[(numSideVerts - 1)][i][j] = 0;

            grid[i][0][j] = 0;
            grid[i][(numSideVerts - 1)][j] = 0;
        }
    }

    // A base of 1s at y = 1
    for(int i = 1; i < (numSideVerts - 1); i++){
        for(int j = 1; j < (numSideVerts - 1); j++){
            grid[i][1][j] = 1;
        }
    }

    // Random Pillar
    for(int i = 2; i < (numSideVerts - 1); i++){
        grid[16][i][16] = 1;
    }

    // std::vector<int> skull = readSkull();
    // for(int i = 0; i < (numSideVerts - 1); i++){
    //     for(int j = 0; j < (numSideVerts - 1); j++){
    //         for(int k = 0; k < (numSideVerts - 1); k++){
    //             if((numSideVerts - 1)*(numSideVerts - 1)*i + (numSideVerts - 1)*j + k < skull.size())
    //                 grid[i][j][j] = skull.at((numSideVerts - 1)*(numSideVerts - 1)*i + (numSideVerts - 1)*j + k);
    //         }
    //     }
    // }

    // grid[0][0][0] = 1;
    // grid[1][0][0] = 1;
    // grid[1][1][0] = 1;

    glPointSize(2.0f); 
    glutMainLoop();

    return EXIT_SUCCESS;
}
