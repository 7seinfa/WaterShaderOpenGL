#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <iostream>
#include <vector>

#include "Shaders.hpp"
#include "Functions.hpp"

class PlaneMesh {
	
	std::vector<float> verts;
	std::vector<float> normals;
	std::vector<int> indices;

	int numIndices;
	int numVerts;
	float min;
	float max;

	GLuint VAO, VBO, NBO, EBO, ProgramID, waterTextureID, dispTextureID
			, MID, VID, PID, LightPos, Time;



	// The function planeMeshQuads is not written by me, rather by my professor!!!
	void planeMeshQuads(float min, float max, float stepsize) {

		// The following coordinate system works as if (min, 0, min) is the origin
		// And then builds up the mesh from that origin down (in z)
		// and then to the right (in x).
		// So, one "row" of the mesh's vertices have a fixed x and increasing z

		//manually create a first column of vertices
		float x = min;
		float y = 0;
		for (float z = min; z <= max; z += stepsize) {
			verts.push_back(x);
			verts.push_back(y);
			verts.push_back(z);
			normals.push_back(0);
			normals.push_back(1);
			normals.push_back(0);
		}

		for (float x = min+stepsize; x <= max; x += stepsize) {
			for (float z = min; z <= max; z += stepsize) {
				verts.push_back(x);
				verts.push_back(y);
				verts.push_back(z);
				normals.push_back(0);
				normals.push_back(1);
				normals.push_back(0);
			}
		}

		int nCols = (max-min)/stepsize + 1;
		int i = 0, j = 0;
		for (float x = min; x < max; x += stepsize) {
			j = 0;
			for (float z = min; z < max; z += stepsize) {
				indices.push_back(i*nCols + j);
				indices.push_back(i*nCols + j + 1);
				indices.push_back((i+1)*nCols + j + 1);
				indices.push_back((i+1)*nCols + j);
				++j;
			}
			++i;
		}
	}

public:

	PlaneMesh(float min, float max, float stepsize) {
		this->min = min;
		this->max = max;
		
		GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
		char const * VertexSourcePointer = VertexShaderCode.c_str();
		glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
		glCompileShader(VertexShaderID);

		GLuint TessControlShaderID = glCreateShader(GL_TESS_CONTROL_SHADER);
		char const * TessControlPointer = TesselationControlShaderCode.c_str();
		glShaderSource(TessControlShaderID, 1, &TessControlPointer , NULL);
		glCompileShader(TessControlShaderID);

		GLuint TessEvalShaderID = glCreateShader(GL_TESS_EVALUATION_SHADER);
		char const * TessEvalPointer = TesselationEvaluationShaderCode.c_str();
		glShaderSource(TessEvalShaderID, 1, &TessEvalPointer , NULL);
		glCompileShader(TessEvalShaderID);

		GLuint GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
		char const * GeometrySourcePointer = GeometryShaderCode.c_str();
		glShaderSource(GeometryShaderID, 1, &GeometrySourcePointer , NULL);
		glCompileShader(GeometryShaderID);

		GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
		char const * FragmentSourcePointer = FragmentShaderCode.c_str();
		glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
		glCompileShader(FragmentShaderID);

		ProgramID = glCreateProgram();
		glAttachShader(ProgramID, VertexShaderID);
		glAttachShader(ProgramID, TessControlShaderID);
		glAttachShader(ProgramID, TessEvalShaderID);
		glAttachShader(ProgramID, GeometryShaderID);
		glAttachShader(ProgramID, FragmentShaderID);
		glLinkProgram(ProgramID);

		std::cout<<"Done shaders";

		// Fill vertices, normals, and indices
		planeMeshQuads(min, max, stepsize);
		numVerts = verts.size()/3;
		numIndices = indices.size();

		
		glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &NBO);
        glGenBuffers(1, &EBO);
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), &verts[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, NBO);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), &normals[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);

        glBindVertexArray(0);

		MID = glGetUniformLocation(ProgramID, "M");
        VID = glGetUniformLocation(ProgramID, "V");
        PID = glGetUniformLocation(ProgramID, "P");
        LightPos = glGetUniformLocation(ProgramID, "lightPos");
        Time = glGetUniformLocation(ProgramID, "time");

		unsigned int width;
		unsigned int height;
		unsigned char* imageData;
		loadBMP("Assets/displacement-map1.bmp", &imageData, &width, &height);
		glGenTextures(1, &dispTextureID);
		glBindTexture(GL_TEXTURE_2D, dispTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, imageData);
		glGenerateMipmap(GL_TEXTURE_2D);
		
		glUseProgram(ProgramID);
		glUniform1i(glGetUniformLocation(ProgramID, "displacementTexture"), 1);
        glActiveTexture(GL_TEXTURE1);
		glUseProgram(0);
		free(imageData);

		
		loadBMP("Assets/water.bmp", &imageData, &width, &height);
		glGenTextures(1, &waterTextureID);
		glBindTexture(GL_TEXTURE_2D, waterTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, imageData);
		glGenerateMipmap(GL_TEXTURE_2D);

		glUseProgram(ProgramID);
		glUniform1i(glGetUniformLocation(ProgramID, "waterTexture"), 0);
        glActiveTexture(GL_TEXTURE0);
		glUseProgram(0);
		free(imageData);
	}

	void draw(glm::vec3 lightPos, glm::mat4 V, glm::mat4 P) {
		glUseProgram(ProgramID);
        glBindVertexArray(VAO);
        glPatchParameteri(GL_PATCH_VERTICES, 4);


		glUniformMatrix4fv(VID, 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(PID, 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(MID, 1, GL_FALSE, &(glm::mat4(1.0f)[0][0]));
		glUniform3fv(LightPos, 1, &lightPos[0]);
		
		double curTime = glfwGetTime();
		static double prevTime = glfwGetTime();
		float timeChange = ( curTime - prevTime );
		glUniform1f(Time, timeChange);

		glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, dispTextureID);
		glEnable(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, waterTextureID);
		glEnable(GL_TEXTURE0);

		// draw triangles using VAO
	
		glDrawElements(GL_PATCHES, numIndices, GL_UNSIGNED_INT, (void*)0);

        glBindVertexArray(0);        
        glUseProgram(0);
        glBindTexture(GL_TEXTURE_2D, 0);
	}


};
