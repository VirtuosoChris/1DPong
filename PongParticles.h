//#include <se/sim/TimeUpdatable.h>
#include <list>
#include <se/graphics/cpGL/GlslShaderProgram.h>
#include <cstdlib>

using namespace se::graphics::cpGL;

class Particle: public se::sim::Updatable
{
private:
    float posX;
    float posY;
    float posZ;
    float velocity;
    float noise;

public:

    se::sim::Quantity<double,se::sim::physics::seconds> timeAlive;

    virtual void update(const se::sim::Quantity<double,se::sim::physics::seconds>& time)
    {
        posY+=velocity*time;
        posX += noise*time;
        timeAlive+=time;
    }

    virtual void render() const
    {
        //glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_POINT_SPRITE);
        GlslShaderProgram::disableShaders();
        glDisable(GL_TEXTURE_2D);
        glPointSize(2);
        glPushMatrix();
        glLoadIdentity();
        glBegin(GL_POINTS);
        glVertex3f(posX, posY, posZ);
        glEnd();
        glPopMatrix();
    }

    virtual ~Particle(){}

    Particle(const float& px, const float& py, const float& pz, const float& vel):posX(px), posY(py), posZ(pz),velocity(vel),timeAlive(0.0){

    noise = float(rand() % 15) - 7.5;
    noise/=10.0;

    velocity+= (float(rand()%10) - 5.0)/ 10.0;

    }
};



class ParticleManager: public se::sim::Updatable
{
public:
    static std::list<Particle> particles;

    virtual void render() const
    {
        glColor3f(1,1,0);

        std::list<Particle>::const_iterator ci = particles.begin();

        for(; ci != particles.end(); ci++){
            (*ci).render();
        }
    }

    virtual void update(const se::sim::Quantity<double,se::sim::physics::seconds>& time)
    {
        std::list<Particle>::iterator ci = particles.begin();

        for(; ci != particles.end();)
        {
            (*ci).update(time);

            std::list<Particle>::iterator ci2 =ci;

            ci++;

            if((*ci2).timeAlive > .4)particles.erase(ci2);
        }
    }
};






