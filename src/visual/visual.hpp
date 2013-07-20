#ifndef _VISUAL_HPP
#define _VISUAL_HPP

#define GL_GLEXT_PROTOTYPES // Necessary for vertex buffer

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../hesp.hpp"

#include "../DataLoader.hpp"

#include <string>


// Macros
static const unsigned int DIM = 3;


using std::string;


/**
 *  \brief  CVisual
 */
class CVisual
{
public:
    // Default constructor
    CVisual (DataLoader *dataLoader,
             const int width = 800,
             const int height = 600);

    ~CVisual(); // Destructor

    /**
     *  \brief  Initializes window.
     */
    void initWindow(const string windowname = "GLFW Window");

    /**
     *  \brief  Loads vertex and fragment shader.
     */
    GLuint loadShaders(const string &vertexFilename,
                       const string &fragmentFilename);

    /**
     *  \brief  Initializes particles and buffer objects.
     */
    GLvoid
    initParticlesVisual(const cl_float4 *positions,
                        const size_t numParticles);

#if defined(USE_CGL_SHARING)
    GLvoid
    initParticlesVisual(const size_t numParticles);
#endif // USE_CGL_SHARING

    /**
     *  \brief  Initializes system sizes, textures and buffer objects.
     */
    GLvoid
    initSystemVisual(const cl_float4 sizesMin,
                     const cl_float4 sizesMax);
    /**
     *  \brief  Prints all particles to the GLFW window with OpenGL.
     */
    GLvoid
    visualizeParticles(const cl_float4 *positions,
                       const cl_float4 *velocities);

#if defined(USE_CGL_SHARING)
    GLvoid
    visualizeParticles(void);
#endif // USE_CGL_SHARING

    GLuint
    createSharingBuffer(const GLsizeiptr size) const;

    /**
     *  \brief  Checks if we want to generate waves with 'G'.
     */
    void
    checkInput(bool &generateWaves);

    glm::vec3
    resolveCamPosition(void) const;

    glm::mat4
    calcLookAtMatrix(const glm::vec3 &cameraPt,
                     const glm::vec3 &lookPt,
                     const glm::vec3 &upPt) const;

private:
    // Window stuff
    int mWidth;
    int mHeight;
    DataLoader *mDataLoader;
    GLuint mProgramID; /**< Program ID for OpenGL shaders */
    GLuint mParticleProgramID;

    GLFWwindow *mWindow;

    // System sizes
    GLfloat mSizeXmin;
    GLfloat mSizeXmax;
    GLfloat mSizeYmin;
    GLfloat mSizeYmax;
    GLfloat mSizeZmin;
    GLfloat mSizeZmax;
    GLuint mSystemBufferID;

    GLuint mPositionAttrib;
    GLuint mNormalAttrib;
    GLuint mTexcoordAttrib;

    // Particle stuff
    size_t mNumParticles;
    GLfloat *mParticles; /**< Vertex array */
    GLuint mParticlesBufferID;

    GLint mCameraToClipMatrixUnif;
    GLint mWorldToCameraMatrixUnif;
    GLint mTextureUnif;

    // Camera stuff
    glm::vec3 mCamTarget;
    glm::vec3 mCamSphere;

    mutable GLuint mSharingBufferID;

    GLuint mParticlePositionAttrib;
    GLint mParticleCameraToClipMatrixUnif;
    GLint mParticleWorldToCameraMatrixUnif;

    GLuint mWallTexture;

};

#endif // _VISUAL_HPP
