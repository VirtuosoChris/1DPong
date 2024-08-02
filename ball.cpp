#include "ball.h"

#include <iostream>
Ball::Ball():timeStart(false),resetTime(0.0f),radius(.25f),rotationDegrees(0.0f),lastHit(NONE)
{
    std::cout<<"Ball constructor"<<std::endl;
    positive = false;
    reset();
}


void Ball::sendWorldMatrix(const se::graphics::cpGL::GlslShaderProgram& shader) const
{
    float lightMV[16];
    glPushMatrix();
        glLoadIdentity();
        glTranslatef(0.0,position,BALLZ);
        glRotatef( -rotationDegrees, 1.0,0.0,0.0);
        glGetFloatv(GL_MODELVIEW_MATRIX, lightMV);
        glUniformMatrix4fv(glGetUniformLocation(shader.getID(), "lightModelViewMatrixWorld"), 1, false, lightMV);
    glPopMatrix();
}


void Ball::update(const se::sim::Quantity<double,se::sim::physics::seconds>& secs)
{
    //finding acceleration due to force of kinetic friction fk
    //fk = normal force (in this case weight) * coefficient of kinetic friction
    //force = mass * acceleration => acceleration due to friction is fk/m
    //this implies accelF = m*g*coeff / m = g* coeff

    //if(position > 2.5 - .5){velocity*=-1;}

    if( fabs(double(velocity)) < .005)
    {
        if(!timeStart)
        {
            resetTime = se::sim::Quantity<double,se::sim::physics::seconds>(3.0f);
            timeStart = true;
        }
        else
        {
            resetTime -= secs;
        }

        if(resetTime <= 0.0f)
        {
            timeStart = false;

            switch(lastHit){
                    case COMPUTER_HIT:
                        points[HUMAN]++;
                    break;

                    case PLAYER_HIT:
                        points[COMPUTER]++;
                    break;

                    default:break;
            }

            reset();
        }
    }

    const double fcoeff = .0350f;
    //const double mass = 5.0f;

    //se::sim::Quantity<double, se::sim::physics::kilograms> ballmass(mass);
    float modifier;
    if(velocity > 0.0f)modifier = -1.0f;
    else{modifier = 1.0f;}

    se::sim::Quantity<double, se::sim::physics::acceleration> friction(modifier*9.8*fcoeff);
    acceleration = friction;

    se::sim::Quantity<double, se::sim::physics::meters> dist  = velocity*secs + acceleration*(secs*secs)*.5;
    velocity += acceleration * secs;

    position += dist;

    double radians = dist/radius;
    double degrees = 360.0f * radians;
    degrees/=  2*3.14159;

    rotationDegrees += degrees;
}

void Ball::render() const
{
    glPushMatrix();
        glTranslatef(0.0,position,BALLZ);
        glRotatef( -rotationDegrees, 1.0,0.0,0.0);
        gluSphere(OpenGLQuadricObject::getInstance().getObject(), radius, 20,20);
    glPopMatrix();
}


Ball::~Ball()
{
}
