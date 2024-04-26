#define _USE_MATH_DEFINES

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <iostream>
#include <vector>

#include <cmath>

#include "PlaneMesh.hpp"


// Camera parameters
float camera_r = 5.0f; // Initial distance from origin
float camera_theta = M_PI / 4.0f; // Initial horizontal angle
float camera_phi = M_PI / 4.0f; // Initial vertical angle
float camera_speed = 0.1f;

// Mouse movement variables
bool mouse_dragging = false;
double last_x = 0.0;
double last_y = 0.0;

// Mouse callback function for handling mouse movement
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (mouse_dragging) {
        double dx = xpos - last_x;
        double dy = ypos - last_y;

        camera_theta += dx * 0.01f;
        camera_phi -= dy * 0.01f;

        // Clamp vertical angle to avoid flipping
        camera_phi = std::max(std::min(camera_phi, static_cast<float>(M_PI - 0.01)), 0.01f);

        last_x = xpos;
        last_y = ypos;
    }
}

// Mouse button callback function for handling mouse clicks
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            mouse_dragging = true;
            glfwGetCursorPos(window, &last_x, &last_y);
        } else if (action == GLFW_RELEASE) {
            mouse_dragging = false;
        }
    }
}

// Keyboard callback function for handling keyboard input
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_UP) {
            camera_r -= camera_speed;
            if (camera_r < 0.1f) // Prevent camera from going below the origin
                camera_r = 0.1f;
        } else if (key == GLFW_KEY_DOWN) {
            camera_r += camera_speed;
        }
    }
}


//////////////////////////////////////////////////////////////////////////////
// Main
//////////////////////////////////////////////////////////////////////////////

int main( int argc, char* argv[])
{

	///////////////////////////////////////////////////////
	float screenW = 1500;
	float screenH = 1500;

	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( screenW, screenH, "Water Waves", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}


	PlaneMesh plane(-20.0f, 20.0f, 1.0f);

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Dark blue background
	glClearColor(0.2f, 0.2f, 0.3f, 0.0f);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)screenW / (float)screenH, 0.1f, 1000.0f);

	glm::vec3 lightpos(8.0f, 10.0f, 0.0f);

	glm::vec3 eye = {5.0f, 5.0f, 5.0f};
	glm::vec3 center = {0.0f, 0.0f, 0.0f};
	glm::vec3 up = {0.0f, 1.0f, 0.0f};
	glm::mat4 V = glm::lookAt(eye, center, up); 

	glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadMatrixf(glm::value_ptr(Projection));

    // Loads the view matrix onto the modelview matrix stack
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadMatrixf(glm::value_ptr(V));
    
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);

	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	do{
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		float camera_x = camera_r * sin(camera_phi) * cos(camera_theta);
        float camera_y = camera_r * cos(camera_phi);
        float camera_z = camera_r * sin(camera_phi) * sin(camera_theta);
        eye = glm::vec3(camera_x, camera_y, camera_z);
		//cameraControlsGlobe(V, 5);
		glm::mat4 V = glm::lookAt(eye, center, up); 
		glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)screenW / (float)screenH, 0.1f, 1000.0f);

		plane.draw(lightpos, V, Projection);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	return 0;
}

