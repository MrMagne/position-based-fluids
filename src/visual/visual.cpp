#include "visual.hpp"

#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <cmath>
#include <vector>

#include "SOIL.h"

using std::runtime_error;
using std::ifstream;
using std::ios;
using std::cout;
using std::cerr;
using std::endl;
using std::isnan;
using std::cin;
using std::istreambuf_iterator;
using std::vector;
using std::ifstream;


CVisual::CVisual (const int width,
                  const int height)
    : mWidth(width),
      mHeight(height),
      mProgramID(0),
#if defined(GLFW3)
      mWindow(NULL),
#endif // GLFW3
      mSystemBufferID(0),
      mPositionAttrib(0),
      mNumParticles(0),
      mParticles(NULL),
      mParticlesBufferID(0),
      mCamTarget( glm::vec3(0.0f, 0.0f, 0.0f) ),
      mCamSphere( glm::vec3(0.0f, 20.0f, -0.5f) ),
      mSharingBufferID(0)
{

}

CVisual::~CVisual ()
{
    // Destructor never reached if ESC is pressed in GLFW
    //cout << "Finish." << endl;
    glDeleteProgram(mProgramID);

#if defined(USE_CGL_SHARING)
    delete[] mParticles;
#endif // USE_CGL_SHARING

    glFinish();
    glfwTerminate();
}


void
CVisual::initWindow(const string windowname)
{
    // Initialise GLFW
    if ( !glfwInit() )
    {
        throw runtime_error("Could not initialize GLFW!");
    }

#if defined(GLFW3)
    // We want OpenGL 3.2
    glfwWindowHint(GLFW_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_VERSION_MINOR, 2);

    // Create a windowed mode window and its OpenGL context
    mWindow = glfwCreateWindow(mWidth, mHeight,
                               windowname.c_str(), NULL, NULL);

    if ( !mWindow )
    {
        throw runtime_error("Could not open GLFW Window!");
        glfwTerminate();
    }

    // Make the window's context current
    glfwMakeContextCurrent(mWindow);
    glfwSetInputMode(mWindow, GLFW_STICKY_KEYS, GL_TRUE);

#else
    // We want OpenGL 3.2
    glfwOpenWindowHint(GLFW_VERSION_MAJOR, 3);
    glfwOpenWindowHint(GLFW_VERSION_MINOR, 2);

    if ( !glfwOpenWindow(mWidth, mHeight, 32, 32, 32, 32, 32, 0, GLFW_WINDOW) )
    {
        throw runtime_error("Could not open GLFW Window!");
        glfwTerminate();
    }

    glfwSetWindowTitle( windowname.c_str() );
    glfwEnable(GLFW_STICKY_KEYS); // Enable escape key

    // glEnable(GL_CULL_FACE);
    // glEnable(GL_POINT_SMOOTH);
#endif // GLFW3

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_POINT_SPRITE);
    glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
}

glm::vec3
CVisual::resolveCamPosition(void) const
{
    GLfloat phi = (M_PI / 180.0f) * (mCamSphere.x - 90.0f);
    GLfloat theta = (M_PI / 180.0f) * (mCamSphere.y + 90.0f);

    GLfloat fSinTheta = sinf(theta);
    GLfloat fCosTheta = cosf(theta);
    GLfloat fCosPhi = cosf(phi);
    GLfloat fSinPhi = sinf(phi);

    glm::vec3 dirToCamera(fSinTheta * fCosPhi, fCosTheta, fSinTheta * fSinPhi);

    return (dirToCamera * mCamSphere.z) + mCamTarget;
}

glm::mat4
CVisual::calcLookAtMatrix(const glm::vec3 &cameraPt,
                          const glm::vec3 &lookPt,
                          const glm::vec3 &upPt) const
{
    glm::vec3 lookDir = glm::normalize(lookPt - cameraPt);
    glm::vec3 upDir = glm::normalize(upPt);

    glm::vec3 rightDir = glm::normalize(glm::cross(lookDir, upDir));
    glm::vec3 perpUpDir = glm::cross(rightDir, lookDir);

    glm::mat4 rotMat(1.0f);

    rotMat[0] = glm::vec4(rightDir, 0.0f);
    rotMat[1] = glm::vec4(perpUpDir, 0.0f);
    rotMat[2] = glm::vec4(-lookDir, 0.0f);

    rotMat = glm::transpose(rotMat);

    glm::mat4 transMat(1.0f);

    transMat[3] = glm::vec4(-cameraPt, 1.0f);

    return rotMat * transMat;
}

GLvoid
CVisual::initSystemVisual(const hesp_float4 sizesMin,
                          const hesp_float4 sizesMax)
{
    // Set system sizes
    mSizeXmin = sizesMin.s[0];
    mSizeXmax = sizesMax.s[0];
    mSizeYmin = sizesMin.s[1];
    mSizeYmax = sizesMax.s[1];
    mSizeZmin = sizesMin.s[2];
    mSizeZmax = sizesMax.s[2];

    const GLfloat systemVertices[] =
    {
        -2.0f, mSizeYmin, mSizeZmin, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        2.0f, mSizeYmin, mSizeZmin, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        2.0f, 2.0f, mSizeZmin, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        -2.0f, 2.0f, mSizeZmin, 1.0f,
        0.0f, 0.0f, 1.0f, 0.0f,

        -2.0f, mSizeYmin, mSizeZmin, 1.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        2.0f, mSizeYmin, mSizeZmin, 1.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        2.0f, mSizeYmin, 2.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        -2.0f, mSizeYmin, 2.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
    };

    glGenTextures(1, &mWallTexture);
    glBindTexture(GL_TEXTURE_2D, mWallTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    int texWidth = 0;
    int texHeight = 0;
    int channels = 0;

    unsigned char *image = SOIL_load_image("wall.tga", &texWidth, &texHeight,
                                           &channels, SOIL_LOAD_RGB);

#if defined(USE_DEBUG)
    printf("texture: %d %d %d\n", texWidth, texHeight, channels );
#endif // USE_DEBUG

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);

    glBindTexture(GL_TEXTURE_2D, 0);

    glGenBuffers(1, &mSystemBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, mSystemBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(systemVertices),
                 systemVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    mProgramID = this->loadShaders("visual/shadervertex.glsl",
                                   "visual/shaderfragment.glsl");
}

GLvoid
CVisual::initParticlesVisual(const hesp_float4 *positions,
                             const size_t numParticles)
{
    mNumParticles = numParticles;

    mParticles = new GLfloat[numParticles * 4];

    // Fill vertex array
    for (size_t i = 0; i < numParticles; ++i)
    {
        mParticles[4 * i + 0] = positions[i].s[0];
        mParticles[4 * i + 1] = positions[i].s[1];
        mParticles[4 * i + 2] = positions[i].s[2];
        mParticles[4 * i + 3] = 1.0f;
    }

    glGenBuffers(1, &mParticlesBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, mParticlesBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * numParticles,
                 mParticles, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    mPositionAttrib = glGetAttribLocation(mProgramID, "position");
    mNormalAttrib = glGetAttribLocation(mProgramID, "normal");
    mTexcoordAttrib = glGetAttribLocation(mProgramID, "texcoord");
    mCameraToClipMatrixUnif = glGetUniformLocation(mProgramID, "p_matrix");
    mWorldToCameraMatrixUnif = glGetUniformLocation(mProgramID, "mv_matrix");
    mTextureUnif = glGetUniformLocation(mProgramID, "texture");

    mParticleProgramID = this->loadShaders("visual/particlevertex.glsl",
                                           "visual/particlefragment.glsl");

    mParticlePositionAttrib = glGetAttribLocation(mParticleProgramID, "position");
    mParticleCameraToClipMatrixUnif = glGetUniformLocation(mParticleProgramID, "p_matrix");
    mParticleWorldToCameraMatrixUnif = glGetUniformLocation(mParticleProgramID, "mv_matrix");

    glm::mat4 projectionMat = glm::perspective(45.0f,
                              mWidth / (GLfloat) mHeight, 0.1f, 10.0f);

    glUseProgram(mProgramID);
    glUniformMatrix4fv(
        mCameraToClipMatrixUnif,
        1, GL_FALSE,
        glm::value_ptr(projectionMat)
    );
    glUseProgram(0);

    glUseProgram(mParticleProgramID);
    glUniformMatrix4fv(
        mParticleCameraToClipMatrixUnif,
        1, GL_FALSE,
        glm::value_ptr(projectionMat)
    );
    glUseProgram(0);
}

#if defined(USE_CGL_SHARING)
GLvoid
CVisual::initParticlesVisual(const size_t numParticles)
{
    mNumParticles = numParticles;

    mPositionAttrib = glGetAttribLocation(mProgramID, "position");
    mNormalAttrib = glGetAttribLocation(mProgramID, "normal");
    mTexcoordAttrib = glGetAttribLocation(mProgramID, "texcoord");
    mCameraToClipMatrixUnif = glGetUniformLocation(mProgramID, "p_matrix");
    mWorldToCameraMatrixUnif = glGetUniformLocation(mProgramID, "mv_matrix");
    mTextureUnif = glGetUniformLocation(mProgramID, "texture");

    mParticleProgramID = this->loadShaders("visual/particlevertex.glsl",
                                           "visual/particlefragment.glsl");

    mParticlePositionAttrib = glGetAttribLocation(mParticleProgramID, "position");
    mParticleCameraToClipMatrixUnif = glGetUniformLocation(mParticleProgramID, "p_matrix");
    mParticleWorldToCameraMatrixUnif = glGetUniformLocation(mParticleProgramID, "mv_matrix");

    glm::mat4 projectionMat = glm::perspective(45.0f,
                              mWidth / (GLfloat) mHeight, 0.1f, 10.0f);

    glUseProgram(mProgramID);
    glUniformMatrix4fv(
        mCameraToClipMatrixUnif,
        1, GL_FALSE,
        glm::value_ptr(projectionMat)
    );
    glUseProgram(0);

    glUseProgram(mParticleProgramID);
    glUniformMatrix4fv(
        mParticleCameraToClipMatrixUnif,
        1, GL_FALSE,
        glm::value_ptr(projectionMat)
    );
    glUseProgram(0);
}
#endif // USE_CGL_SHARING

GLuint
CVisual::createSharingBuffer(const GLsizeiptr size) const
{
    GLuint bufferID = 0;
    glGenBuffers(1, &bufferID);
    glBindBuffer(GL_ARRAY_BUFFER, bufferID);

    // GL_STATIC_DRAW is recommended by apple if cgl sharing is used
    // STATIC seems only be in respect to the host
    glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW);

    mSharingBufferID = bufferID;

    return bufferID;
}

GLvoid
CVisual::visualizeParticles(const hesp_float4 *positions,
                            const hesp_float4 *velocities)
{
    glClearColor(0.05f, 0.05f, 0.05f, 0.0f); // Dark blue background
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const glm::vec3 &camPos = resolveCamPosition();
    const glm::mat4 lookAtMat = calcLookAtMatrix( camPos, mCamTarget,
                                glm::vec3(0.0f, 1.0f, 0.0f) );

    glUseProgram(mProgramID);

    glUniformMatrix4fv(
        mWorldToCameraMatrixUnif,
        1, GL_FALSE,
        glm::value_ptr(lookAtMat)
    );

    // Texture stuff do not optimize for now
    // 1. only write uniform once
    // 2. use variable offset instead of hardcoded 0
    glUniform1i(mTextureUnif, 0);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, mWallTexture);

    glBindBuffer(GL_ARRAY_BUFFER, mSystemBufferID);
    glEnableVertexAttribArray(mPositionAttrib);
    glEnableVertexAttribArray(mNormalAttrib);
    glVertexAttribPointer(mPositionAttrib, 4, GL_FLOAT, GL_FALSE,
                          8 * sizeof(GLfloat), 0);
    glVertexAttribPointer( mNormalAttrib, 4, GL_FLOAT, GL_FALSE,
                           8 * sizeof(GLfloat), (void *) ( 4 * sizeof(GLfloat) ) );

    glDrawArrays(GL_QUADS, 0, 8);

    glDisableVertexAttribArray(mNormalAttrib);
    glDisableVertexAttribArray(mPositionAttrib);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glUseProgram(0);

    glUseProgram(mParticleProgramID);
    glUniformMatrix4fv(
        mParticleWorldToCameraMatrixUnif,
        1, GL_FALSE,
        glm::value_ptr(lookAtMat)
    );

    glBindBuffer(GL_ARRAY_BUFFER, mParticlesBufferID);
    mParticles = (GLfloat *) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

    // Update data array
    for (size_t i = 0; i < mNumParticles; ++i)
    {
        mParticles[4 * i + 0] = positions[i].s[0];
        mParticles[4 * i + 1] = positions[i].s[1];
        mParticles[4 * i + 2] = positions[i].s[2];
        mParticles[4 * i + 3] = sqrt( velocities[i].s[0] * velocities[i].s[0]
                                      + velocities[i].s[1] * velocities[i].s[1]
                                      + velocities[i].s[2] * velocities[i].s[2] );

#if defined(USE_DEBUG)
        if (isnan(positions[i].s[0])
                || isnan(positions[i].s[1])
                || isnan(positions[i].s[2]))
        {
            cout << "isnan" << i << endl;
        }
#endif // USE_DEBUG
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);

    glEnableVertexAttribArray(mParticlePositionAttrib);
    glVertexAttribPointer(mParticlePositionAttrib, 4, GL_FLOAT,
                          GL_FALSE, 0, 0);
    glDrawArrays(GL_POINTS, 0, mNumParticles);
    glDisableVertexAttribArray(mParticlePositionAttrib);

    glUseProgram(0);

#if defined(GLFW3)
    glfwSwapBuffers(mWindow);
#else
    glfwSwapBuffers();
#endif // GLFW3

}

#if defined(USE_CGL_SHARING)
GLvoid
CVisual::visualizeParticles(void)
{
    glClearColor(0.05f, 0.05f, 0.05f, 0.0f); // Dark blue background
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const glm::vec3 &camPos = resolveCamPosition();
    const glm::mat4 lookAtMat = calcLookAtMatrix( camPos, mCamTarget,
                                glm::vec3(0.0f, 1.0f, 0.0f) );

    glUseProgram(mProgramID);

    glUniformMatrix4fv(
        mWorldToCameraMatrixUnif,
        1, GL_FALSE,
        glm::value_ptr(lookAtMat)
    );

    // Texture stuff do not optimize for now
    // 1. only write uniform once
    // 2. use variable offset instead of hardcoded 0
    glUniform1i(mTextureUnif, 0);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, mWallTexture);

    glBindBuffer(GL_ARRAY_BUFFER, mSystemBufferID);
    glEnableVertexAttribArray(mPositionAttrib);
    glEnableVertexAttribArray(mNormalAttrib);
    glVertexAttribPointer(mPositionAttrib, 4, GL_FLOAT, GL_FALSE,
                          8 * sizeof(GLfloat), 0);
    glVertexAttribPointer( mNormalAttrib, 4, GL_FLOAT, GL_FALSE,
                           8 * sizeof(GLfloat), (void *) ( 4 * sizeof(GLfloat) ) );

    glDrawArrays(GL_QUADS, 0, 8);

    glDisableVertexAttribArray(mNormalAttrib);
    glDisableVertexAttribArray(mPositionAttrib);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glUseProgram(0);

    glUseProgram(mParticleProgramID);
    glUniformMatrix4fv(
        mParticleWorldToCameraMatrixUnif,
        1, GL_FALSE,
        glm::value_ptr(lookAtMat)
    );

    glBindBuffer(GL_ARRAY_BUFFER, mSharingBufferID);
    glEnableVertexAttribArray(mParticlePositionAttrib);
    glVertexAttribPointer(mParticlePositionAttrib, 4, GL_FLOAT,
                          GL_FALSE, 0, 0);
    glDrawArrays(GL_POINTS, 0, mNumParticles);
    glDisableVertexAttribArray(mParticlePositionAttrib);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUnmapBuffer(GL_ARRAY_BUFFER);

    glUseProgram(0);

#if defined(GLFW3)
    glfwSwapBuffers(mWindow);
#else
    glfwSwapBuffers();
#endif // GLFW3

}
#endif // USE_CGL_SHARING

void
CVisual::checkInput(bool &generateWaves)
{

#if defined(GLFW3)
    glfwPollEvents();

    if (glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glFinish();
        glfwTerminate();
        exit(-1);
    }

    if (glfwGetKey(mWindow, 'W') == GLFW_PRESS)
    {
        mCamTarget.z -= 0.01f;
    }

    if (glfwGetKey(mWindow, 'S') == GLFW_PRESS)
    {
        mCamTarget.z += 0.01f;
    }

    if (glfwGetKey(mWindow, 'Q') == GLFW_PRESS)
    {
        mCamTarget.y -= 0.01f;
    }

    if (glfwGetKey(mWindow, 'E') == GLFW_PRESS)
    {
        mCamTarget.y += 0.01f;
    }

    if (glfwGetKey(mWindow, 'D') == GLFW_PRESS)
    {
        mCamTarget.x -= 0.01f;
    }

    if (glfwGetKey(mWindow, 'A') == GLFW_PRESS)
    {
        mCamTarget.x += 0.01f;
    }

    if (glfwGetKey(mWindow, 'O') == GLFW_PRESS)
    {
        mCamSphere.z -= 0.01f;
    }

    if (glfwGetKey(mWindow, 'U') == GLFW_PRESS)
    {
        mCamSphere.z += 0.01f;
    }

    if (glfwGetKey(mWindow, 'I') == GLFW_PRESS)
    {
        mCamSphere.y -= 1.0f;
    }

    if (glfwGetKey(mWindow, 'K') == GLFW_PRESS)
    {
        mCamSphere.y += 1.0f;
    }

    if (glfwGetKey(mWindow, 'J') == GLFW_PRESS)
    {
        mCamSphere.x -= 1.0f;
    }

    if (glfwGetKey(mWindow, 'L') == GLFW_PRESS)
    {
        mCamSphere.x += 1.0f;
    }

    if (glfwGetKey(mWindow, 'G') == GLFW_PRESS)
    {
        generateWaves = !generateWaves;
    }
#else
    if (glfwGetKey(GLFW_KEY_ESC) == GLFW_PRESS)
    {
        glFinish();
        glfwTerminate();
        exit(-1);
    }

    if (glfwGetKey('W') == GLFW_PRESS)
    {
        mCamTarget.z -= 0.01f;
    }

    if (glfwGetKey('S') == GLFW_PRESS)
    {
        mCamTarget.z += 0.01f;
    }

    if (glfwGetKey('Q') == GLFW_PRESS)
    {
        mCamTarget.y -= 0.01f;
    }

    if (glfwGetKey('E') == GLFW_PRESS)
    {
        mCamTarget.y += 0.01f;
    }

    if (glfwGetKey('D') == GLFW_PRESS)
    {
        mCamTarget.x -= 0.01f;
    }

    if (glfwGetKey('A') == GLFW_PRESS)
    {
        mCamTarget.x += 0.01f;
    }

    if (glfwGetKey('O') == GLFW_PRESS)
    {
        mCamSphere.z -= 0.01f;
    }

    if (glfwGetKey('U') == GLFW_PRESS)
    {
        mCamSphere.z += 0.01f;
    }

    if (glfwGetKey('I') == GLFW_PRESS)
    {
        mCamSphere.y -= 1.0f;
    }

    if (glfwGetKey('K') == GLFW_PRESS)
    {
        mCamSphere.y += 1.0f;
    }

    if (glfwGetKey('J') == GLFW_PRESS)
    {
        mCamSphere.x -= 1.0f;
    }

    if (glfwGetKey('L') == GLFW_PRESS)
    {
        mCamSphere.x += 1.0f;
    }

    if (glfwGetKey('G') == GLFW_PRESS)
    {
        generateWaves = !generateWaves;
    }
#endif // GLFW3

}

/**
*  \brief  Loads all shaders.
*/
GLuint
CVisual::loadShaders(const string &vertexFilename,
                     const string &fragmentFilename)
{
    GLuint programID = 0;

    string line; // Used for getline()

    GLint result = GL_FALSE;
    GLint logLength = 0;
    const char *source = NULL;
    char *errorMsg = NULL;

    // Create the shaders
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    string shaderCode;
    ifstream ifs(vertexFilename.c_str(), ios::binary);

    // Read in vertex shader code
    if ( !ifs )
    {
        throw runtime_error("Could not open file for vertex shader!");
    }

    shaderCode = string( istreambuf_iterator<char>(ifs),
                         istreambuf_iterator<char>() );

    ifs.close();

    // Compile Vertex Shader
#if defined(USE_DEBUG)
    cout << "Compiling vertex shader " << vertexFilename << endl;
#endif // USE_DEBUG

    source = shaderCode.c_str();
    glShaderSource(vertexShaderID, 1, &source , NULL);
    glCompileShader(vertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &logLength);

    if (logLength > 0)
    {
        errorMsg = new char[logLength + 1];

        glGetShaderInfoLog(vertexShaderID, logLength, NULL, errorMsg);
        cerr << errorMsg << endl;

        delete[] errorMsg;
    }

    shaderCode.clear();

    // Read the Fragment Shader code from the file
    ifs.open(fragmentFilename.c_str(), ios::binary);

    // Read in fragment shader code
    if ( !ifs )
    {
        throw runtime_error("Could not open file for fragment shader!");
    }

    shaderCode = string( istreambuf_iterator<char>(ifs),
                         istreambuf_iterator<char>() );

    ifs.close();

    // Compile Fragment Shader
#if defined(USE_DEBUG)
    cout << "Compiling fragment shader " << fragmentFilename << endl;
#endif // USE_DEBUG

    source = shaderCode.c_str();
    glShaderSource(fragmentShaderID, 1, &source, NULL);
    glCompileShader(fragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &logLength);

    if (logLength > 0)
    {
        errorMsg = new char[logLength + 1];

        glGetShaderInfoLog(fragmentShaderID, logLength, NULL, errorMsg);
        cerr << errorMsg << endl;

        delete[] errorMsg;
    }

    // Link the program
#if defined(USE_DEBUG)
    cout << "Linking program\n" << endl;
#endif // USE_DEBUG

    programID = glCreateProgram();

    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);

    // Check the program
    glGetProgramiv(programID, GL_LINK_STATUS, &result);
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &logLength);

    if (logLength > 0)
    {
        errorMsg = new char[logLength + 1];

        glGetProgramInfoLog(programID, logLength, NULL, errorMsg);
        cerr << errorMsg << endl;

        delete[] errorMsg;
    }

    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    return programID;
}
