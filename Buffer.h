#include <glm/glm.hpp>
#include <GL/freeglut.h>
#include <GL/glew.h>

#include <../Mesh.h>

#include <string>
#include <vector>
using namespace std;

// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#define num_vert 3072

class Buffer
{
	GLuint numVertices;
	GLuint VBO;
public:
	Buffer(GLuint numVert = 0) : numVertices(numVert) {};
	~Buffer() {};
	GLuint generateObjectBuffer(GLfloat vertices[], GLfloat colors[]);
	GLuint generateObjectBuffer(glm::vec3 vertices[], glm::vec4 colors[]);
	void linkCurrentBuffertoShader(GLuint shaderProgramID);
	void bind() const;
	//bool generateObjectBufferMesh(const char*, GLuint);
};


// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS
GLuint Buffer::generateObjectBuffer(GLfloat vertices[], GLfloat colors[]) {
	// Genderate 1 generic buffer object, called VBO
	glGenBuffers(1, &VBO);
	// In OpenGL, we bind (make active) the handle to a target name and then execute commands on that target
	// Buffer will contain an array of vertices 
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// After binding, we now fill our object with data, everything in "Vertices" goes to the GPU
	glBufferData(GL_ARRAY_BUFFER, numVertices * 7 * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	// if you have more data besides vertices (e.g., vertex colours or normals), use glBufferSubData to tell the buffer when the vertices array ends and when the colors start
	glBufferSubData(GL_ARRAY_BUFFER, 0, numVertices * 3 * sizeof(GLfloat), vertices);
	glBufferSubData(GL_ARRAY_BUFFER, numVertices * 3 * sizeof(GLfloat), numVertices * 4 * sizeof(GLfloat), colors);
	return VBO;
}

GLuint Buffer::generateObjectBuffer(glm::vec3 vertices[], glm::vec4 colors[]) {
	// Genderate 1 generic buffer object, called VBO
	glGenBuffers(1, &VBO);
	// In OpenGL, we bind (make active) the handle to a target name and then execute commands on that target
	// Buffer will contain an array of vertices 
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// After binding, we now fill our object with data, everything in "Vertices" goes to the GPU
	glBufferData(GL_ARRAY_BUFFER, numVertices * (sizeof(glm::vec3) + sizeof(glm::vec4)), NULL, GL_STATIC_DRAW);
	// if you have more data besides vertices (e.g., vertex colours or normals), use glBufferSubData to tell the buffer when the vertices array ends and when the colors start
	glBufferSubData(GL_ARRAY_BUFFER, 0, numVertices * sizeof(glm::vec3), vertices);
	glBufferSubData(GL_ARRAY_BUFFER, numVertices * sizeof(glm::vec3), numVertices * sizeof(glm::vec4), colors);
	return VBO;
}

void Buffer::linkCurrentBuffertoShader(GLuint shaderProgramID) {
	// find the location of the variables that we will be using in the shader program
	GLuint positionID = glGetAttribLocation(shaderProgramID, "vPosition");
	GLuint colorID = glGetAttribLocation(shaderProgramID, "vColor");
	// Have to enable this
	glEnableVertexAttribArray(positionID);
	// Tell it where to find the position data in the currently active buffer (at index positionID)
	glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// Similarly, for the color data.
	glEnableVertexAttribArray(colorID);
	glVertexAttribPointer(colorID, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(numVertices * 3 * sizeof(GLfloat)));
}

void Buffer::bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
}

//bool Buffer::generateObjectBufferMesh(const char* MESH_NAME, GLuint shaderProgramID)
//{
//	//Note: you may get an error "vector subscript out of range" if you are using this code for a mesh that doesnt have positions and normals
//	//Might be an idea to do a check for that before generating and binding the buffer.
//	GLuint loc1, loc2, loc3;
//	ModelData mesh_data;
//	mesh_data = load_mesh(MESH_NAME);
//	unsigned int vp_vbo = 0;
//	loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
//	loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
//	loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");
//
//	glGenBuffers(1, &vp_vbo);
//	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
//	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mVertices[0], GL_STATIC_DRAW);
//	unsigned int vn_vbo = 0;
//	glGenBuffers(1, &vn_vbo);
//	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
//	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mNormals[0], GL_STATIC_DRAW);
//
//	//	This is for texture coordinates which you don't currently need, so I have commented it out
//	//	unsigned int vt_vbo = 0;
//	//	glGenBuffers (1, &vt_vbo);
//	//	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
//	//	glBufferData (GL_ARRAY_BUFFER, monkey_head_data.mTextureCoords * sizeof (vec2), &monkey_head_data.mTextureCoords[0], GL_STATIC_DRAW);
//
//	unsigned int vao = 0;
//	glBindVertexArray(vao);
//
//	glEnableVertexAttribArray(loc1);
//	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
//	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
//	glEnableVertexAttribArray(loc2);
//	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
//	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
//
//	//	This is for texture coordinates which you don't currently need, so I have commented it out
//	//	glEnableVertexAttribArray (loc3);
//	//	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
//	//	glVertexAttribPointer (loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
//	return true;
//}

#pragma endregion VBO_FUNCTIONS