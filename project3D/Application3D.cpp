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

struct tank
{
	mat4 world = mat4(1);

	mat4 tank_base = world;
	mat4 tank_base_translation = glm::translate(tank_base, vec3(0, 0, 0));
	mat4 tank_base_rotation = glm::rotate(tank_base, 0.0f, vec3(0, 1, 0));

	mat4 turret_base = glm::translate(tank_base, vec3(0, 0, 0));
	mat4 turret_base_translation = glm::translate(turret_base, vec3(0, 0.5, 0));
	mat4 turret_base_rotation = glm::rotate(turret_base, 0.0f, vec3(0, 1, 0));

	mat4 turret_barrel = glm::translate(turret_base, vec3(0, 0.3, 0.3));
	mat4 turret_barrel_offset = glm::translate(turret_base, vec3(0, 0.3, 0.3));
	mat4 turret_barrel_translation = glm::translate(turret_barrel, vec3(0, 0, 0));
	mat4 turret_barrel_rotation = glm::rotate(turret_barrel, 1.57f, vec3(1, 0, 0));
};

struct axisRotation
{
	float deg = 0.f;
	float ninety = 90.f;

	// index[1] will return 0, cos(deg), sin(deg), 0
	mat4 x_rot_degrees = mat4(
		1.f, 0.f, 0.f, 0.f,
		0.f, cos(deg), sin(deg), 0.f,
		0.f, -sin(deg), cos(deg), 0.f,
		0.f, 0.f, 0.f, 1.f);

	mat4 y_rot_degrees = mat4(
		cos(deg), 0.f, -sin(deg), 0.f,
		0.f, 1.f, 0.f, 0.f,
		sin(deg), 0.f, cos(deg), 0.f,
		0.f, 0.f, 0.f, 1.f);

	mat4 z_rot_degrees = mat4(
		cos(deg), sin(deg), 0.f, 0.f,
		-sin(deg), cos(deg), 0.f, 0.f,
		0.f, 0.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 1.f);

	mat4 z_rot_90 = mat4(
		cos(90), sin(90), 0.f, 0.f,
		-sin(90), cos(90), 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f);

	mat4 swivel_rotation = mat4(z_rot_90 * y_rot_degrees);
};

// tank and rotations
tank abrams;

float deg = 0.f;




void Application3D::update(float deltaTime) {

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
	
	abrams.tank_base = abrams.world * abrams.tank_base_translation * abrams.tank_base_rotation;
	abrams.turret_base = abrams.tank_base * abrams.turret_base_translation * abrams.tank_base_rotation;
	abrams.turret_barrel = abrams.tank_base * abrams.turret_barrel_translation * abrams.turret_barrel_rotation;

	// demonstrate a few shapes
	Gizmos::addAABBFilled(abrams.world[3], vec3(1,0.5,1), green, &abrams.tank_base);
	Gizmos::addSphere(abrams.world[3], 0.8, 8, 8, blue, &abrams.turret_base);
	Gizmos::addCylinderFilled(abrams.world[3], 0.3, 1, 15, black, &abrams.turret_barrel);

	vec3 forward = abrams.tank_base[2] * (10 * deltaTime);
	if (input->isKeyDown(aie::INPUT_KEY_W))
		abrams.tank_base_translation = glm::translate(abrams.tank_base_translation, forward);

	if (input->isKeyDown(aie::INPUT_KEY_A))
		abrams.tank_base_rotation = glm::rotate(abrams.tank_base_rotation, 0.1f, vec3(0, 1, 0));
	if (input->isKeyDown(aie::INPUT_KEY_D))
		abrams.tank_base_rotation = glm::rotate(abrams.tank_base_rotation, -0.1f, vec3(0, 1, 0));

	if (input->isKeyDown(aie::INPUT_KEY_RIGHT))
		abrams.turret_barrel_rotation = abrams.turret_barrel_rotation * glm::rotate(-.1f, vec3(0,0,1));

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