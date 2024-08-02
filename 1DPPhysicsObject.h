#ifndef _1DPPHYSICSOBJECT_H
#define _1DPPHYSICSOBJECT_H

#include <se/sim/Updatable.h>
#include <se/sim/Quantity.h>
#include <se/sim/physics/StandardUnits.h>

class _1DPPhysicsObject:public se::sim::Updatable
{

protected:
	se::sim::Quantity<double, se::sim::physics::meters> position;
	se::sim::Quantity<double, se::sim::physics::velocity> velocity;
	se::sim::Quantity<double, se::sim::physics::acceleration> acceleration;

public:

	_1DPPhysicsObject(const float& pos=0.0f, const float& vel=0.0f, const float& accel=0.0f);
	virtual ~_1DPPhysicsObject(){}

	const se::sim::Quantity<double, se::sim::physics::acceleration>& getAcceleration()const{return acceleration;}
	void setAcceleration(se::sim::Quantity<double, se::sim::physics::acceleration>& p){acceleration = p;}


	const se::sim::Quantity<double, se::sim::physics::meters>& getPosition()const{return position;}
	const se::sim::Quantity<double, se::sim::physics::velocity>& getVelocity()const{return velocity;}

	void setPosition(se::sim::Quantity<double, se::sim::physics::meters>& p){position = p;}
	void setVelocity(se::sim::Quantity<double, se::sim::physics::velocity>& v){velocity = v;}

	virtual void update(const se::sim::Quantity<double,se::sim::physics::seconds>&)=0;

	virtual void render()const=0;
};


#endif
