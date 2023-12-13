
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#pragma warning(disable : 5208)

// Windows includes (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <math.h>
#include <vector> // STL dynamic memory.

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

// Project includes
#include "maths_funcs.h"
#include <../Shader.h>
#include <../Mesh.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define MESH_NAME "Hut.dae"

using namespace std;
GLuint shaderProgramID;

ModelData mesh_data[5];
unsigned int mesh_vao = 0;
int width = 800;
int height = 600;

const size_t MAX_BUFFER_SIZE = 40000;
GLuint loc1, loc2, loc3, VBO_p[MAX_BUFFER_SIZE], VBO_n[MAX_BUFFER_SIZE], VBO_tex[MAX_BUFFER_SIZE];
GLfloat rotate_x = 0.0f, rotate_x_child = 0.0f;
GLfloat rotate_y = 0.0f;
GLfloat rotate_y_child = 0.0f;
GLfloat translate_y = 0.0f;
GLfloat pitch = 0.0, yaw = 0.0;
GLfloat lastX = -1, lastY = -1, pauseX, pauseY;
GLfloat translate_lepr[50] = { 0.0f, 0.0f, 0.0f};
GLfloat step[50] = { 0.05f, 0.05f, 0.05f };

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraDir, cameraPauseDir;
bool pauseCamera = false;
int num_buff = 0;
const char* texture_name[] = {"lepface.jpg", "HouseTex.jpg", "Streetlamp1.png", "bin_texture.jpg"};
unsigned int textures[MAX_BUFFER_SIZE];
int rocking_steps = 5;
int rocking_degs = 1;

Shader shader1("simpleVertexShader.txt", "simpleFragmentShader.txt"), shader2("VertexShader.txt", "FragmentShader1.txt");

 //VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS
void generateObjectBufferMesh(const char* mesh_name, unsigned int id) {
	/*----------------------------------------------------------------------------
	LOAD MESH HERE AND COPY INTO BUFFERS
	----------------------------------------------------------------------------*/

	//Note: you may get an error "vector subscript out of range" if you are using this code for a mesh that doesnt have positions and normals
	//Might be an idea to do a check for that before generating and binding the buffer.

	mesh_data[id] = load_mesh(mesh_name);
	unsigned int vp_vbo = id;
	loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");

	int num_meshes = mesh_data[id].mMeshCount;
	
	for (int i = 0; i < num_meshes; i++)
	{
		glGenBuffers(1, &VBO_p[num_buff]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_p[num_buff]);
		glBufferData(GL_ARRAY_BUFFER, mesh_data[id].mPointCount[i] * sizeof(vec3), &mesh_data[id].mVertices[i][0], GL_STATIC_DRAW);

		glGenBuffers(1, &VBO_n[num_buff]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_n[num_buff]);
		glBufferData(GL_ARRAY_BUFFER, mesh_data[id].mPointCount[i] * sizeof(vec3), &mesh_data[id].mNormals[i][0], GL_STATIC_DRAW);

		glGenBuffers (1, &VBO_tex[num_buff]);
		glBindBuffer (GL_ARRAY_BUFFER, VBO_tex[num_buff]);
		glBufferData (GL_ARRAY_BUFFER, mesh_data[id].mPointCount[i] * sizeof (vec2), &mesh_data[id].mTextureCoords[i][0], GL_STATIC_DRAW);
	

		glEnableVertexAttribArray(loc1);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_p[num_buff]);
		glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

		glEnableVertexAttribArray(loc2);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_n[num_buff]);
		glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);


		glGenTextures(1, &textures[num_buff]);
		glBindTexture(GL_TEXTURE_2D, textures[num_buff]);
		// set the texture wrapping/filtering options (on the currently bound texture object)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// load and generate the texture
		int width, height, nrChannels;

		size_t mat_index = mesh_data[id].useMat[i];

		unsigned char* data = stbi_load(mesh_data[id].mTextureFiles[mat_index], &width, &height, &nrChannels, 0);
		
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			mesh_data[id].hasTexture[mat_index] = true;
			data = stbi_load("bin_texture.jpg", &width, &height, &nrChannels, 0);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
			
		}
		stbi_image_free(data);

		glEnableVertexAttribArray (loc3);
		//glBindBuffer (GL_ARRAY_BUFFER, VBO_tex[id]);
		glVertexAttribPointer (loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

		if (num_buff + 1 < MAX_BUFFER_SIZE) {
			num_buff++;
		}
	}

}

void bindBuffer(unsigned int id)
{
	glUniform1f(glGetUniformLocation(shaderProgramID, "id"), 1.0f);
	unsigned int vp_vbo = id;
	loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");

	int num_meshes = mesh_data[id].mMeshCount;

	int location = 0;
	for (int i = 0; i < id; i++)
	{
		location += mesh_data[i].mMeshCount;
	}

	for (int i = 0; i < num_meshes; i++)
	{
		size_t mat_index = mesh_data[id].useMat[i];
		if (mesh_data[id].hasTexture[mat_index])
		{
			glUniform1f(glGetUniformLocation(shaderProgramID, "id"), 0.0f);
		}
		

		glUniform3fv(glGetUniformLocation(shaderProgramID, "Ks"), 1, glm::value_ptr(mesh_data[id].mSpec[mat_index]));
		glUniform3fv(glGetUniformLocation(shaderProgramID, "Kd"), 1, glm::value_ptr(mesh_data[id].mDiffuse[mat_index]));
		glUniform3fv(glGetUniformLocation(shaderProgramID, "Ka"), 1, glm::value_ptr(mesh_data[id].mAmbient[mat_index]));
		glUniform1f(glGetUniformLocation(shaderProgramID, "specular_exponent"), mesh_data[id].mSpecExp[mat_index]);

		glBindBuffer(GL_ARRAY_BUFFER, VBO_p[location + i]);
		glBufferData(GL_ARRAY_BUFFER, mesh_data[id].mPointCount[i] * sizeof(vec3), &mesh_data[id].mVertices[i][0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, VBO_n[location + i]);
		glBufferData(GL_ARRAY_BUFFER, mesh_data[id].mPointCount[i] * sizeof(vec3), &mesh_data[id].mNormals[i][0], GL_STATIC_DRAW);



		glEnableVertexAttribArray(loc1);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_p[location + i]);
		glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

		glEnableVertexAttribArray(loc2);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_n[location + i]);
		glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);


		glBindBuffer(GL_ARRAY_BUFFER, VBO_tex[location + i]);
		glBufferData(GL_ARRAY_BUFFER, mesh_data[id].mPointCount[i] * sizeof(vec2), &mesh_data[id].mTextureCoords[i][0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(loc3);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_tex[location + i]);
		glVertexAttribPointer(loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

		glBindTexture(GL_TEXTURE_2D, textures[location + i]);

		glDrawArrays(GL_TRIANGLES, 0, mesh_data[id].mPointCount[i]);
		
	}
}
#pragma endregion VBO_FUNCTIONS


void display() {

	//	// NB: Make the call to draw the geometry in the currently activated vertex buffer. This is where the GPU starts to work!
	//	glClear(GL_COLOR_BUFFER_BIT);
	//
	//	VBO1.bind();
	//	Shader shader1("VertexShader.txt", "FragmentShader1.txt");
	//	GLuint shaderProgramID = shader1.CompileShaders();
	//	VBO1.linkCurrentBuffertoShader(shaderProgramID);
	//
	//	glm::mat4 M = glm::mat4(1);
	//	glm::mat4 T = glm::mat4(1);
	//	glm::mat4 S = glm::mat4(1);


	//glUseProgram(shaderProgramID);
	//GLuint IDM = glGetUniformLocation(shaderProgramID, "M");
	//if (IDM < 0)
	//{
	//	fprintf(stderr, "Error: Negative uniform location for matrix\n");
	//	exit(1);
	//}
	//GLuint IDuColor = glGetUniformLocation(shaderProgramID, "uColor");
	//if (IDuColor < 0)
	//{
	//	fprintf(stderr, "Error: Negative uniform location for color\n");
	//	exit(1);
	//}

	//S = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
	//T = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.5f, 0.0f));
	//M = T * S;
	//glUniformMatrix4fv(IDM, 1, FALSE, glm::value_ptr(M));
	//uColor = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
	//glUniform4fv(IDuColor, 1, glm::value_ptr(uColor));

	//glDrawArrays(GL_TRIANGLES, 0, 3);

	//VBO2.bind();
	//Shader shader2("VertexShader.txt", "FragmentShader2.txt");
	//shaderProgramID = shader2.CompileShaders();
	//VBO2.linkCurrentBuffertoShader(shaderProgramID);

	//	S = glm::scale(glm::mat4(1.0f), glm::vec3(18.0f, 18.0f, 18.0f));
	//	T = glm::translate(glm::mat4(1.0f), glm::vec3(-4.5f, -8.0f, 0.0f));
	//	M = T * S;
	//	glUniformMatrix4fv(IDM, 1, FALSE, glm::value_ptr(M));
	//	glUniform4fv(IDuColor, 1, glm::value_ptr(uColor));
	//	glDrawArrays(GL_TRIANGLES, 0, 1024);
	//	glutSwapBuffers();
	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(shaderProgramID);
	

	//Declare your uniform variables that will be used in your shader
	int matrix_location = glGetUniformLocation(shaderProgramID, "model");
	int view_mat_location = glGetUniformLocation(shaderProgramID, "view");
	int proj_mat_location = glGetUniformLocation(shaderProgramID, "proj");
	int id_location = glGetUniformLocation(shaderProgramID, "id");

	// Root of the Hierarchy
	mat4 view = identity_mat4();
	mat4 persp_proj = perspective(45.0f, (float)width / (float)height, 0.1f, 1000.0f);
	mat4 model = identity_mat4();
	model = scale(model, vec3(1.5, 1.5, 1.5));
	model = translate(view, vec3(2.0, 0.5, 0.0f));
	//model = rotate_x_deg(model, rotate_x);
	model = rotate_y_deg(model, -rotate_y);

	view = translate(view, vec3(0.0f, -0.7, -6.0f));
	cameraPos.y = 5.0f;
	if (pauseCamera) {
		view = glm::lookAt(cameraPos, cameraPos + cameraPauseDir, cameraUp);
	}
	else
	{
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	}
	// update uniforms & draw
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model.m);
	glUniform1f(id_location, 1.0f);
	bindBuffer(0);

	// Set up the child matrix
	mat4 modelChild = identity_mat4();
	
	
	modelChild = rotate_x_deg(modelChild, -15 + 10 * cos(translate_lepr[0] * 10));


	modelChild = rotate_z_deg(modelChild, 165);
	modelChild = scale(modelChild, vec3(1.5, 1.5, 1.5));
	modelChild = translate(modelChild, vec3(10.0f, 0.5f, 0.0f));


	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, modelChild.m);
	bindBuffer(0);
	//modelChild = translate(modelChild, vec3(0.0f, translate_y, 0.0f));

	// Apply the root matrix to the child matrix
	mat4 lep = identity_mat4();
	lep = scale(lep, vec3(2.f, 2.f, 2.f));
	lep = translate(lep, vec3(10, 1.f + cos(translate_lepr[0] * 10), 2));
	


	// Update the appropriate uniform and draw the mesh again
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, lep.m);
	bindBuffer(3);

	for (int i = 0; i < 5; i++) {
		mat4 house = identity_mat4();
		house = rotate_y_deg(house, 180);
		house = translate(house, vec3(-40.f + 20.f * i, 0.0f, -20.0f));
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, house.m);
		glUniform1f(id_location, 0.0f);
		bindBuffer(1);
	}
	
	for (int i = 0; i < 5; i++) {
		mat4 house = identity_mat4();
		house = translate(house, vec3(-40.f + 20.f * i, 0.0f, 20.0f));
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, house.m);
		glUniform1f(id_location, 0.0f);
		bindBuffer(1);
	}


	mat4 streetlamp = identity_mat4();
	streetlamp = scale(streetlamp, vec3(18.f, 18.f, 18.f));
	streetlamp = translate(streetlamp, vec3(0.0f, -.5f, -0.0f));
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, streetlamp.m);
	glUniform1f(id_location, 0.0f);
	bindBuffer(2);
	//glDrawArrays(GL_TRIANGLES, 0, mesh_data[2].mMeshCount);

	mat4 leprechaun[10];

	for (int i = 0; i < 10; i++) {
		leprechaun[i] = identity_mat4();
		leprechaun[i] = scale(leprechaun[i], vec3(2.f, 2.f, 2.f));

		leprechaun[i] = rotate_y_deg(leprechaun[i], rotate_x_child);

		if (translate_lepr[i] > 30.0f || -30.0f > translate_lepr[i])
		{
			step[i] *= -1;
		}
		leprechaun[i] = translate(leprechaun[i], vec3( translate_lepr[i], 0.5f + cos(translate_lepr[i] * 10), translate_lepr[i]));
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, leprechaun[i].m);
		glUniform1f(id_location, 0.0f);
		bindBuffer(3);
	}
	
	for (int i = 0; i < 50; i++) {
		for (int j = 0; j < 50; j++)
		{
			mat4 road = identity_mat4();
			//road = scale(road, vec3(0.5f, 0.5f, 0.5f));
			road = translate(road, vec3(-50.0f + i * 3.2, -0.5, -50.0f + j * 3.2));
			glUniformMatrix4fv(matrix_location, 1, GL_FALSE, road.m);
			glUniform1f(id_location, 0.0f);
			bindBuffer(4);
		}
		
	}
	

	glutSwapBuffers();
}


void updateScene() {

	static DWORD last_time = 0;
	DWORD curr_time = timeGetTime();
	if (last_time == 0)
		last_time = curr_time;
	float delta = (curr_time - last_time) * 0.001f;
	last_time = curr_time;

	// Rotate the model slowly around the y axis at 20 degrees per second
	rotate_x += 30.0f * delta;
	rotate_x = fmodf(rotate_x, 360.0f);

	rotate_y += 20.0f * delta;
	rotate_y = fmodf(rotate_y, 360.0f);

	rotate_x_child -= 120.0f * delta;
	rotate_x_child = fmodf(rotate_x, 360.0f);

	rotate_y_child += 80.0f * delta;
	rotate_y_child = fmodf(rotate_y_child, 360.0f);

	translate_y = 2 * sin(delta * 5);

	for (int i = 0; i < 50; i++) {
		int random_value = std::rand();
		if (random_value < RAND_MAX / 1000)
		{
			step[i] *= -1;
		}
		translate_lepr[i] += step[i];
	}
	rocking_steps+= rocking_degs;
	if (rocking_steps == 10 || rocking_steps == 0)
	{
		rocking_degs *= -1;
	}
	// Draw the next frame
	glutPostRedisplay();
}


void init()
{
	// Set up the shaders
	shaderProgramID = shader1.CompileShaders();
	// load mesh into a vertex buffer array
	unsigned int vao = 0;
	glBindVertexArray(vao);
	generateObjectBufferMesh("Audi R8.obj", 0);

	generateObjectBufferMesh("House.obj", 1);

	generateObjectBufferMesh("Streetlamp.obj", 2);

	generateObjectBufferMesh("Leprechaun.obj", 3);

	generateObjectBufferMesh("Road.obj", 4);

	for (int i = 0; i < 50; i++)
	{
		step[i] = 0.1f;
		translate_lepr[i] = 0.0f;
	}

	printf(" finished init function\n");
}

void onKeyboardEvent(unsigned char key, int x, int y)
{
	const float cameraSpeed = 0.5f; // adjust accordingly

	switch (key) {
	//case GLUT_KEY_UP:
	//	// sth

	//	break;
	//case GLUT_KEY_DOWN:
	//	// sth
	//	break;
	case 'q':
		if (!pauseCamera)
		{
			cameraFront = cameraPauseDir;
		}
		break;
	case 'a':
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		break;
	case 'w':
		cameraPos += cameraSpeed * cameraFront;
		break;
	case 'd':
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		break;
	case 's':
		cameraPos -= cameraSpeed * cameraFront;
		break;
	}
}

void onKeyboardRelease(unsigned char key, int x, int y)
{
	if (glutGetModifiers() & GLUT_ACTIVE_SHIFT)
	{
		pauseCamera = false;
		
	}
}

void onMouseEvent(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) {
		switch (button) {
		case GLUT_LEFT_BUTTON:
			// sth
			break;
		case GLUT_RIGHT_BUTTON:
			// sth
			break;
		default:
			break;
		}
	}
}

void onMouseMovement(int x, int y)
{
	if (lastX == 0 || lastX == 800 || lastY == 0 || lastY == 600)
	{
		lastX = x;
		lastY = y;
	}
	float xoffset = x - lastX;
	float yoffset = lastY - y; // reversed since y-coordinates range from bottom to top
	lastX = x;
	lastY = y;

	if (!pauseCamera) {
		const float sensitivity = 0.5f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		cameraDir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraDir.y = sin(glm::radians(pitch));
		cameraDir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(cameraDir);
	}

}

void onSpecialPress(int key, int x, int y)
{
	if (glutGetModifiers() & GLUT_ACTIVE_ALT) {
		std::cout << "alt";
		pauseCamera = true;
		cameraPauseDir = cameraFront;
	}
}

void onSpecialRelease(int key, int x, int y)
{
	if (!(glutGetModifiers() & GLUT_ACTIVE_ALT)) {
		std::cout << "alt released \n";
		pauseCamera = false;
	}
}

int main(int argc, char** argv) {

	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(width, height);
	glutCreateWindow("Hello Triangle");

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(onKeyboardEvent);
	glutKeyboardUpFunc(onKeyboardRelease);
	glutSpecialFunc(onSpecialPress);
	glutSpecialUpFunc(onSpecialRelease);
	glutMouseFunc(onMouseEvent);
	glutPassiveMotionFunc(onMouseMovement);

	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}

