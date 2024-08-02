#include <iostream>

class PlayerInputSample
{

public:

     double ballPosition;
     double ballVelocity;
     double paddlePosition;
     double lastHit;
     double paddleAcceleration;

    virtual ~PlayerInputSample(){}
    PlayerInputSample(const double& bp, const double& bv,const double& ppos, const double& lh, const double& pa): ballPosition(bp), ballVelocity(bv), paddlePosition(ppos), lastHit(lh),paddleAcceleration(pa) {}
};
