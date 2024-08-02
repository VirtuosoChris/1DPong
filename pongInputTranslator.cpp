#include "pongInputTranslator.h"
#include <se/sim/Quantity.h>
#include <se/sim/physics/StandardUnits.h>
#include <GL/freeglut.h>
#include <iostream>


 void PongInputTranslator::onMouseMove(){}

void PongInputTranslator::onButtonDown(const unsigned int& b)
{
    switch(b)
    {
        case 269:
        case 'w':
        case 'W': acceleration = se::sim::Quantity<double, se::sim::physics::acceleration>(.6);//.7
            break;
        case 271:
        case 's':
        case 'S': acceleration = se::sim::Quantity<double, se::sim::physics::acceleration>(-.6);
        break;
        case 27:
        exit(0);
        break;
        default:break;
    }
}

void PongInputTranslator::onButtonUp(const unsigned int& b)
{
    switch(b)
    {
        case 269:
        case 271:
        case 'w':
        case 'W':
        case 's':
        case 'S': acceleration = se::sim::Quantity<double, se::sim::physics::acceleration>(0.0f);
        break;
        default:break;
    }
}

void PongInputTranslator::onJoystickMove(){}
