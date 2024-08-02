#ifndef PONGINPUTTRANSLATOR_H
#define PONGINPUTTRANSLATOR_H

#include <se/sim/ui/PCInputTranslator.h>
#include "Paddle.h"

class PongInputTranslator:public se::sim::ui::PCInputTranslator, public Paddle
{
    public:

    PongInputTranslator(const float& position,const Paddle::Place& pl = Paddle::BOTTOM):Paddle(position, pl){}

    virtual void onMouseMove();
    virtual void onButtonDown(const unsigned int&);
    virtual void onButtonUp(const unsigned int&);
    virtual void onJoystickMove();
};

#endif
