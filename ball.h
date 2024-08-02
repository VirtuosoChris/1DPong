#ifndef BALL_H
#define BALL_H

#include <se/graphics/cpGL/GlslShaderProgram.h>
#include "OpenGLQuadricObject.h"
#include "1DPPhysicsObject.h"

#define BALLZ -7.5f

class Ball:public _1DPPhysicsObject
{
public:

    enum LastHit{NONE, PLAYER_HIT, COMPUTER_HIT};
    bool timeStart;
    se::sim::Quantity<double, se::sim::physics::seconds>  resetTime;
    bool positive;

private:

    const float radius;
    float rotationDegrees;
    LastHit lastHit;
    enum player{COMPUTER, HUMAN};
    float points[2];

public:

    Ball();
    virtual ~Ball();

    const LastHit& getLastHit()const { return lastHit; }
    void setLastHit(const LastHit& l){ lastHit = l; }

    const se::sim::Quantity<double, se::sim::physics::seconds>& getResetTime() { return resetTime; }

    const se::sim::Quantity<double, se::sim::physics::kilograms> getMass()
    {
        return se::sim::Quantity<double, se::sim::physics::kilograms>(1.0f);
    }

    const float& getPlayerPoints(){return points[HUMAN];}
    const float& getComputerPoints(){return points[COMPUTER];}

    void render()const;
    void sendWorldMatrix(const se::graphics::cpGL::GlslShaderProgram& shader)const;

    const float& getRotationDegrees()const{return rotationDegrees;}
    const float& getRadius()const{return radius;}

    virtual void update(const se::sim::Quantity<double,se::sim::physics::seconds>&);

    void reset()
    {
        static const float ballstartvelocity = 2.0f;

        positive = !positive;

        resetTime *=0.0f;
        const float modifier[2] = {-1.0f,1.0f};

        velocity  = se::sim::Quantity<double, se::sim::physics::velocity>(ballstartvelocity*modifier[positive]);//* modifier[positive];
        position = se::sim::Quantity<double, se::sim::physics::position>(0.0f);
        acceleration = se::sim::Quantity<double, se::sim::physics::acceleration>(0.0f);

        lastHit = NONE;
    }
};

#endif
