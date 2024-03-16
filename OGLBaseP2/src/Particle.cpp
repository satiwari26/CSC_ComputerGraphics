//
// sueda - geometry edits Z. Wood
// 3/16
//

#include <iostream>
#include "Particle.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Texture.h"


float randFloat(float l, float h)
{
	float r = rand() / (float) RAND_MAX;
	return (1.0f - r) * l + r * h;
}

Particle::Particle(vec3 start) :
	charge(1.0f),
	m(1.0f),
	d(0.0f),
	x(start),
	v(0.0f, 0.0f, 0.0f),
	lifespan(1.0f),
	tEnd(0.0f),
	scale(1.0f),
	color(0.0f, 0.0f, 0.3f, 0.2f)
{
}

Particle::~Particle()
{
}

void Particle::load(vec3 start)
{
	// Random initialization
	rebirth(0.0f, start, false);
}

/* all particles born at the origin */
void Particle::rebirth(float t, vec3 start, bool closerParticles)
{
	charge = randFloat(0.0f, 1.0f) < 0.5 ? -1.0f : 1.0f;	
	m = 1.0f;
  	d = randFloat(0.0f, 0.02f);
	// x = start;
	// Assign a random start position
    x.x = start.x + randFloat(-50.0f, 80.0f);
    x.y = start.y + randFloat(0.0f, 0.0f);
    x.z = start.z + randFloat(-100.0f, 100.0f);

	if(closerParticles) {
		x.x = start.x + randFloat(-40.0f, 40.0f);
		x.y = start.y + randFloat(0.0f, 0.0f);
		x.z = start.z + randFloat(-40.0f, 40.0f);
	}

	v.x = randFloat(-20.27f, -30.3f);
	v.y = randFloat(-50.1f, -60.9f);
	v.z = randFloat(-0.3f, 0.27f);
	lifespan = randFloat(1.0f, 10.0f); 
	tEnd = t + lifespan;
	scale = randFloat(0.2, 1.0f);
   	color.r = 0.0f;
   	color.g = 0.0f;
   	color.b = 0.3f;
	color.a = 1.0f;
}

void Particle::update(float t, float h, const vec3 &g, const vec3 start, bool closerParticles)
{
	if(t > (tEnd)) {
		rebirth(t, start, closerParticles);
	}

	//very simple update
	x += h*v;
	//To do - how do you want to update the forces?
	color.a = (tEnd-t)/lifespan;
}
