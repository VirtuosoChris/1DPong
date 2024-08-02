#ifndef PADDLE_H
#define PADDLE_H

#include "1DPPhysicsObject.h"
#include <se/graphics/cpGL/GlslShaderProgram.h>

#include "ball.h"

#include <iostream>

class Paddle:public _1DPPhysicsObject
{

public:
    enum Place {TOP, BOTTOM};
    Place PLACE;

private:

    float rotationDegrees;
    const float radius;

    float timeSinceEmission;

public:

    const se::sim::Quantity<double, se::sim::physics::kilograms> getMass(){
        return se::sim::Quantity<double, se::sim::physics::kilograms>(5.00f);
    }

    const float& getRadius(){return radius;}

    Paddle(const float& p, const Place& pl = TOP):_1DPPhysicsObject(p),PLACE(pl),radius(.25f),timeSinceEmission(0.0f){

    std::cout<<"Paddle Constructor"<<std::endl;}

    virtual ~Paddle(){}

    void sendWorldMatrix(const se::graphics::cpGL::GlslShaderProgram& shader)const;

    virtual void render()const;

    virtual void update(const se::sim::Quantity<double,se::sim::physics::seconds>&);
};

#endif
