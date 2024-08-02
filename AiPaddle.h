#include "Paddle.h"
#include "ball.h"

#define AIPADDLEACCEL .7

/*
#define cx1 -.552799
#define cx2 -.122959
#define cx3 .132003
#define cx4 .122819
#define cx5 -.673076
#define cx6 .130054
#define cx7 .176457
#define cx8 0.0*///early


/*
#define cx1 -.570195
#define cx2 -.0364054
#define cx3 .143056
#define cx4 .0187148
#define cx5 .358782
#define cx6 -.0901297
#define cx7 .207595
#define cx8 -.482883
*///AI from the first night


/*
#define cx1 -.588836
#define cx2 -.0071494
#define cx3 .169248
#define cx4 -.00115908
#define cx5 .281436
#define cx6 -.0700421
#define cx7 .182883
#define cx8 0.0
*///weak


#define cx1 -.534879
#define cx2 .0175637
#define cx3 .0968308
#define cx4 -.180039
#define cx5 .914933
#define cx6 -.119838
#define cx7 -.201344
#define cx8 -.566005


class AiPaddle:public Paddle
{
public:

    double c1;
    double c2;
    double c3;
    double c4;
    double c5;
    double c6;
    double c7;
    double c8;

    const Ball& targetBall;

    AiPaddle(const Ball& bll, const float& p, const Place& pl = TOP):Paddle(p, pl), c1(cx1), c2(cx2), c3(cx3), c4(cx4),c5(cx5), c6(cx6), c7(cx7), c8(cx8), targetBall(bll){}

    virtual void update(const se::sim::Quantity<double,se::sim::physics::seconds>& tim)
    {
    //special cases the training didn't account for well
    if(targetBall.resetTime> 0.0) //the ball is stopped and waiting to reset or be hit
    {
        //if the ball is outside of our reach or the last person to hit it was the player back up for the next round
        if(double(targetBall.getPosition()) < .75 || targetBall.getLastHit() == Ball::PLAYER_HIT){
            acceleration = se::sim::Quantity<double, se::sim::physics::acceleration>(AIPADDLEACCEL);
        }
    }

    //if the ball is headed for us and its the beginning of our turn we know we want to hit it really hard
    else if(targetBall.getLastHit() == Ball::NONE && targetBall.getVelocity() > 0.0)
    {
        acceleration = se::sim::Quantity<double, se::sim::physics::acceleration>(-AIPADDLEACCEL);
    }

    else
    {
        double ballPos = double(targetBall.getPosition());
        double ballVel = double(targetBall.getVelocity());
        double myPos = double(position);
        double accel = c1*ballPos + c2 * ballPos*ballPos + c3 * ballVel + c4 * ballVel*ballVel + c5*myPos + c6*myPos*myPos + c7* double(targetBall.getLastHit()) +c8;

        //intensify the acceleration, since the curve is likely a smooth gradient
        accel*=1.750;//was 4 true lines down, referring to accel(eration)

        //cap the acceleration magnitude
        if( accel < 0.0f) accel = fmax(-.7, accel);
        else {accel = fmin ( AIPADDLEACCEL, accel);}

        acceleration = se::sim::Quantity<double, se::sim::physics::acceleration>(accel);
    }

    Paddle::update(tim);
    }
};
