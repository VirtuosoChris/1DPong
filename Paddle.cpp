#include "Paddle.h"
#include "OpenGLQuadricObject.h"
#include "PongParticles.h"


void Paddle::sendWorldMatrix(const se::graphics::cpGL::GlslShaderProgram& shader) const
{
    float lightMV[16];
    glPushMatrix();
    glLoadIdentity();
        glTranslatef(-.5,position,-8.0 + 2*radius);
        glRotatef(-rotationDegrees, 1.0f,0.0f,0.0f);
        glRotatef(90.0f, 0.0f,1.0f,0.0f);
        glGetFloatv(GL_MODELVIEW_MATRIX, lightMV);
        glUniformMatrix4fv(glGetUniformLocation(shader.getID(), "lightModelViewMatrixWorld"), 1, false, lightMV);
    glPopMatrix();
}


void Paddle::render() const
{
    glDisable(GL_CULL_FACE);

    glPushMatrix();
        glTranslatef(-.5 - .25,position + .25,-8.0 + 2*radius);
        glRotatef(90.0f, 1.0f,0.0f,0.0f);
        gluCylinder(OpenGLQuadricObject::getInstance().getObject(), radius/2, radius/2, .50f, 20,20);
    glPopMatrix();

    glPushMatrix();
        glTranslatef(.5 + .25,position + .25,-8.0 + 2*radius);
        glRotatef(90.0f, 1.0f,0.0f,0.0f);
        gluCylinder(OpenGLQuadricObject::getInstance().getObject(), radius/2, radius/2, .50f, 20,20);
    glPopMatrix();

    glPushMatrix();
        glTranslatef(-.5,position,-8.0 + 2*radius);

        //glTranslatef(-1,0,0);
        glPushMatrix();
        glTranslatef(-.25,0,0);
        glRotatef(90.0f, 0.0f,1.0f,0.0f);
        gluCylinder(OpenGLQuadricObject::getInstance().getObject(), radius/3, radius/3, 1.50f, 20,20);
        glPopMatrix();

        glRotatef(-rotationDegrees, 1.0f,0.0f,0.0f);
        glRotatef(90.0f, 0.0f,1.0f,0.0f);
        gluCylinder(OpenGLQuadricObject::getInstance().getObject(), radius, radius, 1.0f, 20,20);
    glPopMatrix();

    glEnable(GL_CULL_FACE);
}


void Paddle::update(const se::sim::Quantity<double,se::sim::physics::seconds>& secs)
{
    const double fcoeff = .0350f;
    float modifier = 0.0f;

    if(velocity > 0.0f)modifier = -1.0f;
    else if (velocity <0.0f){modifier = 1.0f;}

    se::sim::Quantity<double, se::sim::physics::acceleration> friction(modifier*9.8*fcoeff);

    se::sim::Quantity<double, se::sim::physics::meters> dist  = velocity*secs + (acceleration+friction)*(secs*secs)*.5;
    velocity += (acceleration+friction) * secs;

    position += dist;

    float minBarrier  (0.0f);
    float maxBarrier (0.0f);

    switch(PLACE)
    {
        case TOP:
            minBarrier = .750f;
            maxBarrier = 3.0f - 2*radius;
            break;

        case BOTTOM:
            minBarrier = -3.0f + 2*radius;
            maxBarrier = -.750f;
            break;
    }

    if(position <= minBarrier)
    {
        position = se::sim::Quantity<double, se::sim::physics::meters>(minBarrier);
        if(velocity < 0.0) velocity *=0.0f;//if moving against the barrier

    }
    else if(position >= maxBarrier)
    {
        position = se::sim::Quantity<double, se::sim::physics::meters>(maxBarrier);
        if(velocity > 0.0) velocity*=0.0f;
    }
    else
    {
        double radians = dist/radius;
        double degrees = 360.0f * radians;
        degrees/=  2*3.14159;
        rotationDegrees += degrees;
    }

    timeSinceEmission += secs;

    if(timeSinceEmission >= .05)
    {
        if(acceleration > 0.0 && !(position>= maxBarrier) )
        {
            ParticleManager::particles.push_back( Particle(.75,float(position)-.25,-8.0+radius,-1.5));
            ParticleManager::particles.push_back( Particle(-.75,float(position)-.25,-8.0+radius,-1.5));
            timeSinceEmission = 0.0f;
        }
        else if(acceleration < 0.0 && !(position<=minBarrier))
        {
            ParticleManager::particles.push_back( Particle(.75,float(position)+.25,-8.0+radius,1.5));
            ParticleManager::particles.push_back( Particle(-.75,float(position)+.25,-8.0+radius,1.5));
            timeSinceEmission = 0.0f;
        }
    }
}
