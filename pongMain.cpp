#include <iostream>
//#include <GL/glut.h>

#include <se/graphics/gl/FreeGLUTWindow.h>
#define FREEGLUT_STATIC			//enable freeglut as a static lib
#define FREEGLUT_LIB_PRAGMAS 0	//turn off autolinking
#include <GL/freeglut.h>
//#include <GL/glut.h>
#include <se/sim/UpdateTimer.h>

#include "ScreenSpaceQuad.h"
//#include "CausticsObject.h"

//todo::fix my uniform initialization methods for vector types and structs
#include <se/graphics/cpGL/GlslShaderProgram.h>
#include <se/graphics/cpGL/GlslVertexShader.h>
#include <se/graphics/cpGL/GlslFragmentShader.h>
#include <se/graphics/cpGL/OpenGLFramebufferObject.h>
#include <se/graphics/cpGL/FboTextureAttachment.h>
//#include <cpGL/OpenGLTextureObject.h>
#include <se/graphics/cpGL/FboDepthAttachment.h>
#include <se/graphics/cpGL/FboDepthTexture.h>
#include <fstream>
#include <se/graphics/image/formats/TgaImage.h>

#include <Eigen/Core>
#include <vector>

#include "ball.h"
#include "Paddle.h"
#include "pongInputTranslator.h"
#include "PlayerInputSample.h"
#include "AiPaddle.h"

#include <Eigen/LeastSquares>
#include <Eigen/SVD>
#include <sstream>

#include "PongParticles.h"

#define TABLESCALE 3.0f

std::list<Particle> ParticleManager::particles;
ParticleManager pman;

USING_PART_OF_NAMESPACE_EIGEN

double fps = 0.0f;

enum GAME_STATE { GS_PAUSED, GS_RUNNING, GS_INSTRUCTIONS };
GAME_STATE gamestate = GS_INSTRUCTIONS;

#include "GameTranslator.h"

Ball ball;
PongInputTranslator pcPaddle(-2.50);

#ifdef AI_GEN
PongInputTranslator aiPaddle(2.50, Paddle::TOP);
#else
AiPaddle aiPaddle(ball, 2.50);
#endif

using namespace se::graphics::cpGL;
using namespace se::graphics;


const unsigned int resWidth = 1024;
const unsigned int resHeight = 768;
//const float pointSize = 10.0f;

cpGL::GlslShaderProgram* lightingProg = NULL;
cpGL::GlslShaderProgram* prepassProg = NULL;

cpGL::OpenGLTextureObject* tableTexture = NULL;
OpenGLTextureObject* metalTexture = NULL;
OpenGLTextureObject* glassTexture = NULL;

const unsigned int shadowmapres = 512;
cpGL::FboDepthTexture<shadowmapres, shadowmapres>* depth = NULL;
cpGL::OpenGLFramebufferObject<shadowmapres, shadowmapres>* depthFbo = NULL;

float lightDirection[3] = { 0.0,0.0,0.0 };
static  float lightPosition[3] = { 0.0,0.0,-4.35 };
float lookatPoint[3] = { 0.0,0.0,BALLZ };

const float spotCutoff = 40.0f;

bool collisionThisFrame = false;

const char* gameInstructionString = "Press p to pause or i for instructions. Escape quits.";
const char* pausedString = "PAUSED - Press P to Resume";


template<typename T>
const unsigned char* getNumberString(const T& in)
{
    std::string number;
    std::ostringstream oss;

    oss << in;

    number = oss.str();

    return (const unsigned char*)number.c_str();
}


void hackInput(int a, int, int)
{
    if (a == 101)pcPaddle.onButtonUp(269);
    else if (a == 103)pcPaddle.onButtonUp(271);
}


//enum targetConfig{RT_NONE, RT_RECEIVERS, RT_CASTERS, RT_CAUSTICS};
void setRenderTargets()
{
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

}

void bindScreen()
{
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    glViewport(0, 0, resWidth, resHeight);
}


void sendTableWorldMatrix()
{
    float lightMV[16];
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -8.0);
    glScalef(TABLESCALE, TABLESCALE, 1.0);
    glGetFloatv(GL_MODELVIEW_MATRIX, lightMV);
    glUniformMatrix4fv(glGetUniformLocation(lightingProg->getID(), "lightModelViewMatrixWorld"), 1, false, lightMV);
    glPopMatrix();
}

void sendSmallBallWorldMatrix()
{
    float lightMV[16];
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0.0, 1.0, -4.85);
    glGetFloatv(GL_MODELVIEW_MATRIX, lightMV);
    glUniformMatrix4fv(glGetUniformLocation(lightingProg->getID(), "lightModelViewMatrixWorld"), 1, false, lightMV);
    glPopMatrix();
}

void sendLargeBallWorldMatrix()
{
    float lightMV[16];
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(.75, .5, -4.75);
    glGetFloatv(GL_MODELVIEW_MATRIX, lightMV);
    glUniformMatrix4fv(glGetUniformLocation(lightingProg->getID(), "lightModelViewMatrixWorld"), 1, false, lightMV);
    glPopMatrix();
}

void renderTable()
{
    glPushMatrix();
    glTranslatef(0.0, 0.0, -8.0);
    glScalef(TABLESCALE, TABLESCALE, 1.0);
    ScreenSpaceQuad::render();
    glPopMatrix();
}

enum matType { WOOD = 0, GLASS = 1, COPPER = 2 };
void setMaterial(const matType& mat)
{
    const float shininess[3] = { 32.0f,10.0f,51.2f };

    const float glassSpecular[3] = { 1.0f,1.0f,1.0f };
    const float woodSpecular[3] = { .0f,.0f,.0f };
    const float copperSpecular[3] = { .256777,.137622,.086014 };

    const float* const specularColor[3] = { woodSpecular,glassSpecular,copperSpecular };

    lightingProg->bind();
    lightingProg->setUniformValue("shininess", shininess[mat]);


    switch (mat) {
    case WOOD:tableTexture->bind(); break;
    case GLASS: glassTexture->bind(); break;
    case COPPER:metalTexture->bind(); break;
    default:;
    };

    lightingProg->setTexture("texture", OpenGLTextureUnit::getActiveUnit());

    //todo:repair shader class api wrapper for this

    const float* const sc = specularColor[mat];
    glUniform3fv(glGetUniformLocation(lightingProg->getID(), "matSpecularColor"), 1, sc);

    lightingProg->setUniformValue("spotCutoff", spotCutoff);
}




//takes in a float[3] containing the light position and sets the shader program's uniform vec3 representing the light direction
void updateLightDirection(float* vec)
{
    //float vec2[3] = {0.0f,0.0f,0.0f};
    float length = 0.0f;

    for (int i = 0; i < 3; i++) {
        length += pow(lookatPoint[i] - vec[i], 2.0);
    }

    length = sqrt(length);

    for (int i = 0; i < 3; i++) {
        lightDirection[i] = lookatPoint[i] - vec[i];
        lightDirection[i] /= length;
    }

    glUniform3fv(glGetUniformLocation(lightingProg->getID(), "lightDirection"), 1, lightDirection);

}



void updateLight()
{
    lightingProg->bind();
    //lightPosition[1] = sin(frameCount+=.01);
    lookatPoint[1] = ball.getPosition();
    glUniform3fv(glGetUniformLocation(lightingProg->getID(), "lightPositionWorld"), 1, lightPosition);
    updateLightDirection(lightPosition);
}



void initialize()
{
    cpGL::GlslFragmentShader* prepassFrag = NULL;
    cpGL::GlslVertexShader* prepassVert = NULL;
    cpGL::GlslVertexShader* lightingVertShader = NULL;
    cpGL::GlslFragmentShader* lightingFragShader = NULL;


    glViewport(0, 0, resWidth, resHeight);//todo:: does steve's call to bind in renderTarget/window/whatever set the viewport?
    OpenGLTextureUnit::initialize();

    //load textures
    std::ifstream file("textures/wood.tga", std::fstream::binary);
    cpGL::OpenGLTextureObject::Settings sets1;
    image::TgaImage img(file);

    std::ifstream file2("textures/copper.tga", std::fstream::binary);
    image::TgaImage img2(file2);


    std::ifstream file3("textures/glass.tga", std::fstream::binary);
    image::TgaImage img3(file3);

    //glEnable(GL_TEXTURE_2D);
    //todo: revise the texture generation api to be less redundant and confusing
    sets1.width = img.width;
    sets1.height = img.height;
    sets1.colors_per_pixel = img.colors_per_pixel;
    sets1.bits_per_color = img.bits_per_color;
    sets1.is_floating_point = false;
    tableTexture = new OpenGLTextureObject(img, sets1);
    tableTexture->bind();
    tableTexture->setTextureRepeat(true); ///TODO:: these shouldn't work!
    tableTexture->setTextureFilter(OpenGLTextureObject::FILTER_TRILINEAR);

    tableTexture->generateMipMaps();

    sets1.width = img2.width;
    sets1.height = img2.height;
    sets1.colors_per_pixel = img2.colors_per_pixel;
    sets1.bits_per_color = img2.bits_per_color;
    metalTexture = new OpenGLTextureObject(img2, sets1);
    metalTexture->bind();
    metalTexture->setTextureFilter(OpenGLTextureObject::FILTER_TRILINEAR);
    metalTexture->generateMipMaps();

    sets1.width = img3.width;
    sets1.height = img3.height;
    sets1.colors_per_pixel = img3.colors_per_pixel;
    sets1.bits_per_color = img3.bits_per_color;
    glassTexture = new OpenGLTextureObject(img3, sets1);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /***Create shaders***/
    std::vector<std::string> svec;

    svec.push_back("shaders/Lighting.vert");
    lightingVertShader = new GlslVertexShader(svec);
    svec.clear();
    svec.push_back("shaders/Lighting.frag");
    lightingFragShader = new GlslFragmentShader(svec);

    svec.clear();
    svec.push_back("shaders/prepass.vert");
    prepassVert = new GlslVertexShader(svec);
    svec.clear();
    svec.push_back("shaders/prepass.frag");
    prepassFrag = new GlslFragmentShader(svec);

    std::vector<int> pvec;
    pvec.clear();
    pvec.push_back(lightingVertShader->getId());
    pvec.push_back(lightingFragShader->getId());
    lightingProg = new GlslShaderProgram(pvec);
    // pvec.clear();
    // pvec.push_back(showDepthFS->getId());
    // pvec.push_back(showDepthVS->getId());
    // showDepthProg = new GlslShaderProgram(pvec);
    pvec.clear();
    pvec.push_back(prepassVert->getId());
    pvec.push_back(prepassFrag->getId());
    prepassProg = new GlslShaderProgram(pvec);

    /////////////////////render the caustics point sprite splat texture
    glPointSize(cpGL::OpenGLRenderConfig::getMaxPointSize());
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_POINT_SPRITE);

    bindScreen();

    glDisable(GL_COLOR_MATERIAL);//todo::read up on this

    /////////////////////////generate a quadrics rendering struct//////////////////////////
    // 
    //initialize the fbo for shadow map rendering
    depth = new FboDepthTexture<shadowmapres, shadowmapres>();
    depthFbo = new OpenGLFramebufferObject<shadowmapres, shadowmapres>();
    depthFbo->bind();
    setRenderTargets();
    depthFbo->attachRenderTarget(*depth);

    bindScreen();

    delete lightingVertShader;
    delete lightingFragShader;
    delete prepassVert;
    delete prepassFrag; // we can delete shader objects once we're created the programs we want with them, and they won't be deleted until the program object is deleted too
}


void render()
{
    static float lightMV[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
    static  float lightProj[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
    //static OpenGLTextureObject table(esm::image::TgaImage(file));

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    updateLight();

    prepassProg->bind();
    depthFbo->bind();
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    //set up a projection matrix for the light
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90.0f, 1.0f//float(resWidth)/resHeight
        , 1.0, 50.0);
    glGetFloatv(GL_PROJECTION_MATRIX, lightProj);

    //get the camera matrix for the light
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPushMatrix();
    gluLookAt(lightPosition[0], lightPosition[1], lightPosition[2], 0, 0, -5, 0, -1, 0);
    glGetFloatv(GL_MODELVIEW_MATRIX, lightMV);
    glUniformMatrix4fv(glGetUniformLocation(prepassProg->getID(), "lightMatrix"), 1, false, lightMV); //todo:temp:refactor shader uniform and attribute setter API
    glPopMatrix();

    //we want the table to receive the caustics
    renderTable();

    //setRenderTargets(RT_CASTERS); //render the casters' positions and normals
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); //clear depth buffer because we don't want the table to self shadow because its unnecessary and precision errors, and because we know nothing will be behind it: applicaiton specific

    ball.render();
    pcPaddle.render();
    aiPaddle.render();

    /*******lighting pass****/
    //render scene with shadows and caustics
    bindScreen();
    lightingProg->bind();
    glUniformMatrix4fv(glGetUniformLocation(lightingProg->getID(), "lightProjectionMatrix"), 1, false, lightProj);//***
    glUniformMatrix4fv(glGetUniformLocation(lightingProg->getID(), "lightModelViewMatrixEye"), 1, false, lightMV);//***
    depth->bind();

    OpenGLTextureUnit::getUnit(1).bind();
    depth->bind();
    lightingProg->setTexture("shadowmap", OpenGLTextureUnit::getActiveUnit());
    OpenGLTextureUnit::getUnit(0).bind();
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, float(resWidth) / resHeight, 1.0, 50.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor3f(1, 1, 1);
    sendTableWorldMatrix();
    setMaterial(WOOD);
    renderTable();
    glColor3f(1, 0, 0);

    setMaterial(COPPER);

    aiPaddle.sendWorldMatrix(*lightingProg);
    aiPaddle.render();

    pcPaddle.sendWorldMatrix(*lightingProg);
    pcPaddle.render();

    ball.sendWorldMatrix(*lightingProg);
    ball.render();

    pman.render();

    OpenGLTextureUnit::getUnit(0).bind();

    cpGL::GlslShaderProgram::disableShaders();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_DEPTH_BUFFER_BIT);

    glColor3f(0, 0, 1);
    glRasterPos2f(-.75, -.75);
    //glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,'3');

    //char abc[7];//only 7 because if the user scores more than 999,999 points they're a loser

    unsigned int playerpts = ball.getPlayerPoints();
    //if(playerpts > 999999){
    //sprintf(abc,"NOLIFE");//prevent buffer overflow
    //}else{sprintf(abc, "%d",playerpts);}
    glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, getNumberString(playerpts));

    glColor3f(1, 0, 0);
    glRasterPos2f(-.75, .75);

    unsigned int comppoints = ball.getComputerPoints();

    glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, getNumberString(comppoints));

    if (ball.getResetTime() > 0.0f)
    {
        glLineWidth(5.0);
        const float ARROWX = .1;
        switch (ball.getLastHit())
        {
            case Ball::COMPUTER_HIT:glColor3f(1, 0, 0); break;
            case Ball::PLAYER_HIT:glColor3f(0, 0, 1); break;
            default:break;
        }

        glRasterPos2f(0, 0);
        // sprintf(abc, "%d", ((int)(ball.getResetTime())+1));
        glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, getNumberString(((int)(ball.getResetTime()) + 1)));

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        const float arrowDim = .05;
        glBegin(GL_LINES);

        if (ball.positive)
        {
            glColor3f(0, 0, 1);

            glVertex3f(ARROWX, arrowDim, 0);//shaft
            glVertex3f(ARROWX, -arrowDim, 0);

            glVertex3f(ARROWX, -arrowDim, 0);
            glVertex3f(ARROWX + arrowDim, -arrowDim / 2.0, 0);

            glVertex3f(ARROWX, -arrowDim, 0);
            glVertex3f(ARROWX - arrowDim, -arrowDim / 2.0, 0);
        }
        else
        {
            glColor3f(1, 0, 0);
            glVertex3f(ARROWX, arrowDim, 0);//shaft
            glVertex3f(ARROWX, -arrowDim, 0);

            glVertex3f(ARROWX, arrowDim, 0);
            glVertex3f(ARROWX + arrowDim, arrowDim / 2.0, 0);

            glVertex3f(ARROWX, arrowDim, 0);
            glVertex3f(ARROWX - arrowDim, arrowDim / 2.0, 0);
        }

        glEnd();
        glPopMatrix();

        glMatrixMode(GL_MODELVIEW);
    }

    glColor3f(1, 1, 1);
    glRasterPos2f(.8, .9);
    //sprintf(abc, "%d fps", (unsigned int)fps);
    const char* fpsString = " fps";
    glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, getNumberString(((unsigned int)fps)));
    glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)fpsString);

    if (gamestate == GS_PAUSED)
    {
        glRasterPos2f(-.25, .5);
        glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)pausedString);
    }
    else if (gamestate == GS_INSTRUCTIONS) {

        const char* line1 = "1D Pong: A game about mastering friction, by Chris Pugh";

#ifndef AI_GEN
        const char* line2 = "You control the bottom paddle.  Hold down the up and down arrows to accelerate your paddle.";
#else
        const char* line2 = "Hold w/s to accelerate top paddle, and up/down to accelerate bottom paddle.";
#endif

        const char* line3 = "Points go to the player who did not hit the ball last when it stops.";
        const char* line4 = "This means you should avoid touching the ball or hit it with just enough force that it reaches";
        const char* line5 = "the other player, but doesn't have enough speed to get back to you.";
        const char* line7 = "Press I to disable the instructions screen and to begin playing";

        const char* line8 = "When the ball stops, a timer will count down in the middle showing how long until the ball resets.";
        const char* line82 = "An arrow indicator will show which direction the ball will be going when it resets.";
        const char* line83 = "The color of the timer indicates which player hit the ball last, letting you know";
        const char* line84 = "whether you should try and recover or leave the ball alone.";

        const char* line9 = "Credits:";
        const char* line10 = "cgtextures.com for textures";
        const char* line11 = "Testing and Feedback:Steve Braeger, Sarah Loewy, Lindsey Turnbull, Sean McDonnell,";
        const char* line11half = " Peter Zaccagnino, John Pugh, and Thomas DeCleene";
        const char* line12 = "Using Eigen, Freeglut,and base code developed by Steve Braeger, Sarah Loewy, and Chris Pugh";

        glColor3f(1, 1, 0);
        glRasterPos2f(-.9, .9);
        glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)line1);
        glColor3f(1, 1, 1);
        glRasterPos2f(-.9, .8);
        glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)line2);
        glRasterPos2f(-.9, .7);
        glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)line3);
        glRasterPos2f(-.9, .6);
        glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)line4);
        glRasterPos2f(-.9, .5);
        glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)line5);


        glRasterPos2f(-.9, .4);
        glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)line8);
        glRasterPos2f(-.9, .3);
        glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)line82);
        glRasterPos2f(-.9, .2);
        glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)line83);
        glRasterPos2f(-.9, .1);
        glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)line84);


        glRasterPos2f(-.9, -.1);
        glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)line7);

        //credits text: always gets drawn
        glColor3f(1, 1, 0);
        glRasterPos2f(-.95, -0.30);
        glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)line9);
        glColor3f(1, 1, 1);
        glRasterPos2f(-.95, -.4);
        glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)line10);
        glRasterPos2f(-.95, -.5);
        glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)line11);
        glRasterPos2f(-.95, -.6);
        glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)line11half);
        glRasterPos2f(-.95, -.7);
        glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)line12);

        glRasterPos2f(-.95, -.8);
        glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)line13);

    }

    glRasterPos2f(-.99, -.99);
    glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, (const unsigned char*)gameInstructionString);
}


se::sim::Quantity<double, se::sim::physics::seconds> updateGame(se::sim::Quantity<double, se::sim::physics::seconds> secs, Paddle& paddle) {

    se::sim::Quantity<double, se::sim::physics::meters>  paddleCPos(0.0f);
    se::sim::Quantity<double, se::sim::physics::meters>  ballCPos(0.0f);

    //find the relevant points of interest for collision
    if (paddle.PLACE == paddle.TOP)
    {
        paddleCPos = paddle.getPosition() - se::sim::Quantity<double, se::sim::physics::meters>(paddle.getRadius()); //bottom of the paddle
        ballCPos = ball.getPosition() + se::sim::Quantity<double, se::sim::physics::meters>(ball.getRadius());//top of the ball

        if (paddleCPos < ballCPos)
        {
            se::sim::Quantity<double, se::sim::physics::meters> tpx(paddleCPos - se::sim::Quantity<double, se::sim::physics::meters>(ball.getRadius()));
            ball.setPosition(tpx);
            ballCPos = ball.getPosition() + se::sim::Quantity<double, se::sim::physics::meters>(ball.getRadius());//top of the ball
        }
    }
    else
    {
        paddleCPos = paddle.getPosition() + se::sim::Quantity<double, se::sim::physics::meters>(paddle.getRadius()); //top of the paddle
        ballCPos = ball.getPosition() - se::sim::Quantity<double, se::sim::physics::meters>(ball.getRadius());//bottom of the ball

        if (paddleCPos > ballCPos)
        {
            se::sim::Quantity<double, se::sim::physics::meters> tpx(paddleCPos + se::sim::Quantity<double, se::sim::physics::meters>(ball.getRadius()));
            ball.setPosition(tpx);
            ballCPos = ball.getPosition() - se::sim::Quantity<double, se::sim::physics::meters>(ball.getRadius());//bottom of the ball
        }
    }

    //now find the time at which these points will intersect.  if the equations are solvable and the intersection time registers between last frame and this frame
    //register a collision

    //solve the quadratic formula to get the intersection times
    se::sim::Quantity<double, se::sim::physics::acceleration> a = (ball.getAcceleration() - paddle.getAcceleration()) * .5f;
    se::sim::Quantity<double, se::sim::physics::velocity> b = ball.getVelocity() - paddle.getVelocity();
    se::sim::Quantity<double, se::sim::physics::meters> c = ballCPos - paddleCPos;

    se::sim::Quantity<double, se::sim::physics::seconds> maxTime(secs);
    se::sim::Quantity<double, se::sim::physics::acceleration> denominator(0.0f);
    se::sim::Quantity<double, se::sim::physics::seconds> left(0.0f);
    se::sim::Quantity<double, se::sim::physics::seconds> right(0.0f);
    se::sim::Quantity<double, se::sim::physics::seconds> time1(0.0f);
    se::sim::Quantity<double, se::sim::physics::seconds> time2(0.0f);

    while (true) //process all colliions to occur this frame
    {
        double cTime = 0.0f;
        if (a != 0.0f)
        {
            float radical = b * b - (a * c * 4.0f); //units under radical are m^2 / s^2
            if (radical < 0.0f)
            {
                return maxTime;;
            }

            denominator = se::sim::Quantity<double, se::sim::physics::acceleration>(a * 2.0f);
            left = se::sim::Quantity<double, se::sim::physics::seconds>(double(-b) / double(denominator));//incorrect compile error here!!!!!
            right = se::sim::Quantity<double, se::sim::physics::seconds>(sqrt(double(radical)) / double(denominator));
            time1 = left + right;
            time2 = left - right;

            //from our results pick a time that falls in between the current and previous frames
            bool t1Valid = (time1 >= 0.0f && time1 <= maxTime);
            bool t2Valid = (time2 >= 0.0f && time2 <= maxTime);

            //pick the collision time

            if (!(t1Valid || t2Valid)) break;// no collisions

            if (t1Valid && t2Valid)
            {
                cTime = fmin(double(time1), double(time2));
            }
            else if (t1Valid)
            { 
                cTime = double(time1);
            }
            else { cTime = double(time2); }
        }
        else {
            cTime = -double(c) / double(b);
            if (!(cTime >= 0.0f && cTime <= maxTime))return maxTime;
        }

        //std::cout<<"Selected Collision Time"<<std::endl;
        //now we have the time at which the ball and paddle intersected. now we need to recompute the ball + paddle's positions and velocities post-collision
        collisionThisFrame = true;

        switch (paddle.PLACE)
        {
            case Paddle::TOP: ball.setLastHit(Ball::COMPUTER_HIT); break;
            case Paddle::BOTTOM: ball.setLastHit(Ball::PLAYER_HIT); break;
        }


        se::sim::Quantity<double, se::sim::physics::seconds> iTime(cTime);
        maxTime -= iTime;//for the future collisions and updates, decrease the time left in the frame


        //std::cout<<"adjusting frametime"<<std::endl;

        se::sim::Quantity< double, se::sim::physics::meters> np(ball.getPosition() + ball.getVelocity() * iTime + ball.getAcceleration() * iTime * iTime * .5f);
        ball.setPosition(np);

        se::sim::Quantity<double, se::sim::physics::meters> np2(paddle.getPosition() + paddle.getVelocity() * iTime + paddle.getAcceleration() * iTime * iTime * .5f);
        paddle.setPosition(np2);


        se::sim::Quantity<double, se::sim::physics::velocity> nbv(ball.getVelocity() + ball.getAcceleration() * iTime);
        se::sim::Quantity<double, se::sim::physics::velocity> npv(paddle.getVelocity() + paddle.getAcceleration() * iTime);
        ball.setVelocity(nbv);
        paddle.setVelocity(npv);

        //std::cout<<"Set collision time velocities and positions"<<std::endl;

        //get post collision ball velocity
        se::sim::Quantity<double, se::sim::physics::velocity>	pcb(
            ball.getVelocity() *
            double(double((ball.getMass() - paddle.getMass())) / double((paddle.getMass() + ball.getMass())))  //runtime crash here when not casting to double
            +
            paddle.getVelocity() * double(double(paddle.getMass()) * 2.0f / (double(paddle.getMass()) + double(ball.getMass())))
        );
        ball.setVelocity(pcb);


        se::sim::Quantity<double, se::sim::physics::velocity>	pcp(
            paddle.getVelocity() * double((double(-ball.getMass()) + double(paddle.getMass())) / (double(paddle.getMass()) + double(ball.getMass())))
            +
            double(ball.getVelocity()) * double(double(ball.getMass() * 2.0f) / (double(paddle.getMass()) + double(ball.getMass())))
        );
        //pcp*=-1.0f;
        paddle.setVelocity(pcp);

        //recompute b and c
        b = ball.getVelocity() - paddle.getVelocity();
        c = ball.getPosition() - paddle.getPosition();

        //std::cout<<"Set new post collision velocities"<<std::endl;


        if (ball.timeStart)
        {
            ball.timeStart = false;
            ball.resetTime *= 0.0f;
        }
    }

    return maxTime;
}





int main(void)
{
    se::graphics::gl::FreeGLUTWindow::Settings windowSettings;
    windowSettings.window_name = "1DP";
    windowSettings.width = resWidth;
    windowSettings.height = resHeight;

#ifdef AI_GEN
    GameTranslator git(&gamestate, &aiPaddle, &pcPaddle);
#else
    GameTranslator git(&gamestate, NULL, &pcPaddle);
#endif

    se::graphics::gl::FreeGLUTWindow win(windowSettings, git);

    se::sim::UpdateTimer gameTimer;

    try
    {
        initialize();

        gameTimer.update();

        glutSpecialUpFunc(hackInput);

        while (true)
        {
            collisionThisFrame = false;
            se::sim::Quantity<double, se::sim::physics::time> tim = gameTimer.getElapsedSeconds();
            gameTimer.update();//get the amount of time that's elapsed since the last frame


            static double frames = 0.0f;
            static double time = 0.0f;
            time += double(tim);
            fps = frames / time;
            frames += 1.0f;

            //process collisions and get the time left in the frame

            //assujme there will only be a single object to collide with the ball per frame

            if (gamestate == GS_RUNNING)
            {
                se::sim::Quantity<double, se::sim::physics::time> lt = updateGame(tim, pcPaddle);
                if (!collisionThisFrame)lt = updateGame(tim, aiPaddle);

                ball.update(lt);
                pcPaddle.update(lt);
                aiPaddle.update(lt);

                pman.update(tim);//update the particles for the jet effect
            }

            render();


            win.update(se::sim::Quantity<double, se::sim::physics::time>(0.0f));
            win.swap(win.getGLRuntime());

            /*
            #ifdef AI_GEN
            static std::vector<PlayerInputSample> inputs;
            //////////////////////////////////////////
            //if enough time elapsed,
            static double accumTime = 0.0;
            accumTime +=double(tim);
            const unsigned int samplecount = 100;
            if(accumTime >= .500){
                accumTime = 0.0;
                double playerAccel = -double(pcPaddle.getAcceleration());
                double ballVel = -double(ball.getVelocity());
                double ballPos = -double(ball.getPosition());
                double padPos = -double(pcPaddle.getPosition());
                double lastHit;
                if(ball.getLastHit()==Ball::PLAYER_HIT){lastHit = double(Ball::COMPUTER_HIT);}
                            else if(ball.getLastHit() == Ball::COMPUTER_HIT){ lastHit = double(Ball::PLAYER_HIT);}
                            else{lastHit = double(Ball::NONE);}

                inputs.push_back(PlayerInputSample(ballPos, ballVel, padPos,lastHit, playerAccel));
            //std::cout<<inputs.size()<<std::endl;


                //if its time to recompute
                if(inputs.size() == samplecount){ //we want a matrix of dimensionality 500

                        double c1n,  c2n,  c3n,  c4n,  c5n,c6n,c7n, c8n;//new values

                        //Matrix<double,8,1> points[samplecount]  = {};
                        //VectorXd points[samplecount];
                        //for(int q = 0; q < samplecount; q++){
                            //	points[samplecount] = VectorXd(8);
                        //}

                    //do matrix computations

                        //create the A matrix
                        Matrix<double, samplecount, 8> aMatrix;
                        VectorXd bColumn(samplecount);
                        for(unsigned int i = 0; i < samplecount; i++){
                            const PlayerInputSample& is = inputs[i];

                            bColumn[i] = is.paddleAcceleration;

                            aMatrix(i,0) = is.ballPosition;
                            aMatrix(i,1) = is.ballPosition*is.ballPosition;
                            aMatrix(i,2) = is.ballVelocity;
                            aMatrix(i,3) = is.ballVelocity*is.ballVelocity;
                            aMatrix(i,4) = is.paddlePosition;
                            aMatrix(i,5) = is.paddlePosition*is.paddlePosition;
                            aMatrix(i,6) = is.lastHit;
                            aMatrix(i,7) = 1.0;


                            //points[i](0) = is.ballPosition;
                            //points[i](1) = is.ballPosition*is.ballPosition;
                            //points[i](2) = is.ballVelocity;
                            //points[i](3) = is.ballVelocity*is.ballVelocity;
                            //points[i](4) = is.paddlePosition;
                            //points[i](5) = is.paddlePosition*is.paddlePosition;
                            //points[i](6) = is.lastHit;
                            //points[i](7) = is.paddleAcceleration


                        }//create the points


                        //VectorXd result(8);
                         // Matrix<double, 8,1> result;


                        //linearRegression(100,&points,&result, 7);//fuck eigen
                        //linearRegression<(100, &points, &result,7);

                        Matrix<double, 8, samplecount> aTranspose(aMatrix.transpose());
                        Matrix<double,8,8>mProduct =(aTranspose*aMatrix);

                        Matrix<double, 8,1> rhs = aTranspose*bColumn;
                        Matrix<double, 8,1> solution;

                        Eigen::SVD<Matrix<double, 8, 8> > svd(mProduct);
                        svd.solve(rhs, &solution);


                        c1n = solution(0);
                        c2n = solution(1);
                        c3n = solution(2);
                        c4n = solution(3);
                        c5n = solution(4);
                        c6n = solution(5);
                        c7n = solution(6);
                        c8n = solution(7);

                        //VectorXd result = (mProduct.inverse())*aTranspose*bColumn;

                        //std::cout<<aMatrix;


                        std::cout<<c1n<<std::endl<<c2n<<std::endl<<c3n<<std::endl<<c4n<<std::endl<<c5n<<std::endl<<c6n<<std::endl<<c7n<<std::endl<<c8n<<std::endl<<std::endl;//print new values

                        //average new and olf values
                        aiPaddle.c1 = (.1*c1n+.9*aiPaddle.c1);
                        aiPaddle.c2 = (.1*c2n+.9*aiPaddle.c2);
                        aiPaddle.c3 = (.1*c3n+.9*aiPaddle.c3);
                        aiPaddle.c4 = (.1*c4n+.9*aiPaddle.c4);
                        aiPaddle.c5 = (.1*c5n+.9*aiPaddle.c5);
                        aiPaddle.c6 = (.1*c6n+.9*aiPaddle.c6);
                        aiPaddle.c7 = (.1*c7n+.9*aiPaddle.c7);
                        aiPaddle.c8 = (.1*c8n+.9*aiPaddle.c8);


                    std::cout<<"averages"<<aiPaddle.c1<<std::endl<<aiPaddle.c2<<std::endl<<aiPaddle.c3<<std::endl<<aiPaddle.c4<<std::endl<<aiPaddle.c5<<std::endl<<aiPaddle.c6<<std::endl<<aiPaddle.c7<<std::endl<<std::cout<<aiPaddle.c8<<std::endl<<std::endl;//print new values

                        inputs.clear();
                }

            }
            #endif
            */

            ///////////////////////////////////////////
        }
    }
    catch (std::string s)
    {
        std::cerr << s << std::endl;
    }

    delete lightingProg;
    delete prepassProg;

    delete tableTexture;
    delete glassTexture;
    delete metalTexture;

    delete depthFbo;
    delete depth;

    return 0;
}
