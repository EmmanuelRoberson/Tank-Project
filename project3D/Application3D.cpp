#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_SWIZZLE

#include "Application3D.h"
#include "Gizmos.h"
#include "Input.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

using glm::vec3;
using glm::vec4;
using glm::mat4;
using aie::Gizmos;

Application3D::Application3D() {

}

Application3D::~Application3D() {

}

bool Application3D::startup() {
	
	setBackgroundColour(0.25f, 0.25f, 0.25f);

	// initialise gizmo primitive counts
	Gizmos::create(10000, 10000, 10000, 10000);

	// create simple camera transforms
	m_viewMatrix = glm::lookAt(vec3(10), vec3(0), vec3(0, 1, 0));
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f,
										  getWindowWidth() / (float)getWindowHeight(),
										  0.1f, 1000.f);

	return true;
}

void Application3D::shutdown() {

	Gizmos::destroy();
}

//struct for the Tank
struct Tank
{
	mat4 world = mat4(1);

	mat4 tank_base = world;
	mat4 tank_base_translation = glm::translate(tank_base, vec3(0, 0, 0));
	mat4 tank_base_rotation = glm::rotate(tank_base, 0.0f, vec3(0, 1, 0));

	mat4 turret_base = glm::translate(tank_base, vec3(0, 0, 0));
	mat4 turret_base_translation = glm::translate(turret_base, vec3(0, 0.5, 0));
	mat4 turret_base_rotation = glm::rotate(turret_base, 0.0f, vec3(0, 1, 0));


	//using a parent for this object will expose a transform that can be used for pitch
	//the completed heirarchy would look like this in Unity
	/* -tank_base
	 * --turret_base
	 *  --barrel_root
	 *  ---barrel
	 */
	mat4 turret_barrel = glm::translate(turret_base, vec3(0, 0.3, 0.3));
	mat4 turret_barrel_translation = glm::translate(turret_barrel, vec3(0, 0, 0));
	mat4 turret_barrel_rotation = glm::rotate(turret_barrel, 1.57f, vec3(1, 0, 0));

	mat4 turret_bullet = glm::translate(turret_base, vec3(0, 0.3, 0.3));
	mat4 turret_bullet_translation = glm::translate(turret_barrel, vec3(0, 0, 1));
	mat4 turret_bullet_rotation = glm::rotate(turret_bullet, 1.57f, vec3(0, 0, 1));

	float turret_bullet_size = 0.4f;

};

// Tank
Tank abrams;

void Application3D::update(float deltaTime) 
{

	// query time since application started
	float time = getTime();

	// input
	aie::Input* input = aie::Input::getInstance();

	// rotate camera
	m_viewMatrix = glm::lookAt(vec3(glm::sin(0) * 10, 10, glm::cos(0) * 10),
							   vec3(0), vec3(0, 1, 0));

	// wipe the gizmos clean for this frame
	Gizmos::clear();

	// draw a simple grid with gizmos
	vec4 white(1);
	vec4 black(0, 0, 0, 1);
	vec4 red(1, 0, 0, 1);
	vec4 blue(0, 0, 1, 1);
	vec4 green(0, 1, 0, 1);
	for (int i = 0; i < 21; ++i) {
		Gizmos::addLine(vec3(-10 + i, 0, 10),
						vec3(-10 + i, 0, -10),
						i == 10 ? white : black);
		Gizmos::addLine(vec3(10, 0, -10 + i),
						vec3(-10, 0, -10 + i),
						i == 10 ? white : black);
	}

	// add a transform so that we can see the axis
	Gizmos::addTransform(mat4(1));

	// transform matrices for the Tank
	abrams.tank_base = abrams.world * abrams.tank_base_translation * abrams.tank_base_rotation;
	abrams.turret_base = abrams.world * abrams.turret_base_translation * abrams.turret_base_rotation;
	abrams.turret_barrel = abrams.turret_base * abrams.turret_barrel_translation * abrams.turret_barrel_rotation;
	//the bullet has no parent, it has an initial rotation of the barrels rotation in world 
	abrams.turret_bullet = abrams.turret_base *   abrams.turret_bullet_translation * abrams.turret_bullet_rotation;

	// input :: moves the Tank/turret base forward in the direction that its facing


	//recommend seperating the vectors into magnitude and direction for readability and comprehension
	//ex vec3 tank_dir = abrams.tank_base[2]; 
	//float speed = 10 * deltaTime
	//movement = tank_dir * speed;
	vec3 forward_tank = abrams.tank_base[2] * (10 * deltaTime);
	if (input->isKeyDown(aie::INPUT_KEY_W))
	{
		abrams.tank_base_translation = glm::translate(abrams.tank_base_translation, forward_tank);
		abrams.turret_base_translation = glm::translate(abrams.turret_base_translation, forward_tank);
	}

	// input :: shoots the bullet in the direction of the turret
	//that 40 is arbitrary as fuck but can be variablized into something meaningful like speed
	//you could then set it to 40 when "shotsFired" and 0 after some timer
	vec3 forward_bullet = vec3(abrams.turret_barrel[2].x, abrams.turret_barrel[2].z, -abrams.turret_barrel[2].y) * (40 * deltaTime);
	forward_bullet = abrams.turret_base[3].xyz;
	if (input->isKeyDown(aie::INPUT_KEY_SPACE))
	{
		abrams.turret_bullet_size = 0.4f;
		abrams.turret_bullet_translation = glm::translate(glm::mat4(1), forward_bullet + glm::vec3(2,0,0));
	}
	else
	{
		abrams.turret_bullet_translation = abrams.turret_barrel_translation;
		abrams.turret_bullet_size = 0.0f;
	}

	// input :: rotates the Tank left and right
	if (input->isKeyDown(aie::INPUT_KEY_A))
		abrams.tank_base_rotation = glm::rotate(abrams.tank_base_rotation, 0.1f, vec3(0, 1, 0));
	if (input->isKeyDown(aie::INPUT_KEY_D))
		abrams.tank_base_rotation = glm::rotate(abrams.tank_base_rotation, -0.1f, vec3(0, 1, 0));

	// input :: rotate just the turret right and left
	if (input->isKeyDown(aie::INPUT_KEY_RIGHT))
	{
		abrams.turret_base_rotation = glm::rotate(abrams.turret_base_rotation, .05f, vec3(0,1,0));
	}
	if (input->isKeyDown(aie::INPUT_KEY_LEFT))
	{
		abrams.turret_base_rotation = glm::rotate(abrams.turret_base_rotation, -.05f, vec3(0, 1, 0));
	}


	Gizmos::addTransform(abrams.tank_base, 5);
	Gizmos::addTransform(abrams.turret_base, 5);
	Gizmos::addTransform(abrams.turret_barrel, 5);
	// renders the Tank, Tank's turret base, and Tank's turret's base
	Gizmos::addAABBFilled(abrams.world[3], vec3(1, 0.5, 1), green, &abrams.tank_base);
	Gizmos::addSphere(abrams.world[3], 0.8, 8, 8, blue, &abrams.turret_base);
	Gizmos::addCylinderFilled(abrams.world[3], 0.3, 1, 15, black, &abrams.turret_barrel);
	Gizmos::addSphere(vec3(1), abrams.turret_bullet_size, 8, 8, white, &abrams.turret_bullet);


	// quit if we press escape
	if (input->isKeyDown(aie::INPUT_KEY_ESCAPE))
		quit();
}


void Application3D::draw() {

	// wipe the screen to the background colour
	clearScreen();

	// update perspective in case window resized
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f,
										  getWindowWidth() / (float)getWindowHeight(),
										  0.1f, 1000.f);

	// draw 3D gizmos
	Gizmos::draw(m_projectionMatrix * m_viewMatrix);

	// draw 2D gizmos using an orthogonal projection matrix (or screen dimensions)
	Gizmos::draw2D((float)getWindowWidth(), (float)getWindowHeight());
}