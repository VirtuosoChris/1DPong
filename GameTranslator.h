#include <se/sim/ui/PCInputTranslator.h>
#include "pongInputTranslator.h"

class GameTranslator: public se::sim::ui::PCInputTranslator
{
public:

    GAME_STATE* gamestate;
    PongInputTranslator* topPaddle;
    PongInputTranslator* bottomPaddle;

    GameTranslator(GAME_STATE* gs, PongInputTranslator* tp, PongInputTranslator* bp):gamestate(gs),topPaddle(tp),bottomPaddle(bp)
    {
    }

    virtual void onMouseMove() {}

    virtual void onButtonDown(const unsigned int& b)
    {
        switch(b)
        {
            case 'p':
            case 'P': if(*gamestate == GS_PAUSED)*gamestate = GS_RUNNING;
                    else *gamestate = GS_PAUSED;
                    break;

            case 'i':
            case 'I': if(*gamestate == GS_INSTRUCTIONS)*gamestate = GS_RUNNING;
                    else *gamestate = GS_INSTRUCTIONS;
                    break;
            case 'w':
            case 's':
            case 'S':
            case 'W':
            {
#ifdef AI_GEN

                topPaddle->onButtonDown(b);
#else
                bottomPaddle->onButtonDown(b);
#endif
                break;
            }
            case 269:
            case 271:
            {
                bottomPaddle->onButtonDown(b);
                break;
            }
            case 27:
            {
                exit(0);
            }
        }
    }

    //arrow up and down is in the hack special key up function-- not handled here.  So that means w/s gets the AI paddle
    virtual void onButtonUp(const unsigned int& b)
    {
        switch(b)
        {
            case 'w':
            case 's':
            case 'S':
            case 'W':
            {
#ifdef AI_GEN
                topPaddle->onButtonUp(b);
#else
                bottomPaddle->onButtonUp(b);
#endif
                break;
            }
        }
    }

    virtual void onJoystickMove() {}
};
