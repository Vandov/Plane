﻿//=============================================================================================
// Szamitogepes grafika hazi feladat keret. Ervenyes 2016-tol.
// A //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// sorokon beluli reszben celszeru garazdalkodni, mert a tobbit ugyis toroljuk.
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kivéve
// - new operatort hivni a lefoglalt adat korrekt felszabaditasa nelkul
// - felesleges programsorokat a beadott programban hagyni
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak 
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : 
// Neptun : 
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================

#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <vector>

#if defined(__APPLE__)
#include <GLUT/GLUT.h>
#include <OpenGL/gl3.h>
#include <OpenGL/glu.h>
#else
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#include <windows.h>
#endif
#include <GL/glew.h>		// must be downloaded 
#include <GL/freeglut.h>	// must be downloaded unless you have an Apple
#endif

const unsigned int windowWidth = 600, windowHeight = 600;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Innentol modosithatod...

// OpenGL major and minor versions
int majorVersion = 3, minorVersion = 0;

void getErrorInfo(unsigned int handle) {
	int logLen;
	glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logLen);
	if (logLen > 0) {
		char * log = new char[logLen];
		int written;
		glGetShaderInfoLog(handle, logLen, &written, log);
		printf("Shader log:\n%s", log);
		delete log;
	}
}

// check if shader could be compiled
void checkShader(unsigned int shader, char * message) {
	int OK;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &OK);
	if (!OK) {
		printf("%s!\n", message);
		getErrorInfo(shader);
	}
}

// check if shader could be linked
void checkLinking(unsigned int program) {
	int OK;
	glGetProgramiv(program, GL_LINK_STATUS, &OK);
	if (!OK) {
		printf("Failed to link shader program!\n");
		getErrorInfo(program);
	}
}

// vertex shader in GLSL
const char *vertexSource = R"(
	#version 130
    precision highp float;

		uniform mat4 MVP;			// Model-View-Projection matrix in row-major format

		in vec3 vertexPosition;		// variable input from Attrib Array selected by glBindAttribLocation
	in vec3 vertexColor;	    // variable input from Attrib Array selected by glBindAttribLocation
	out vec3 color;				// output attribute

		void main() {
		color = vertexColor;														// copy color from input to output
		gl_Position = vec4(vertexPosition.x, vertexPosition.y, vertexPosition.z, 1) * MVP; 		// transform to clipping space
	}
)";

// fragment shader in GLSL
const char *fragmentSource = R"(
	#version 130
    precision highp float;

		in vec3 color;				// variable input: interpolated color of vertex shader
	out vec4 fragmentColor;		// output that goes to the raster memory as told by glBindFragDataLocation

		void main() {
		fragmentColor = vec4(color, 1); // extend RGB to RGBA
	}
)";

// row-major matrix 4x4
struct mat4 {
	float m[4][4];
public:
	mat4() {}
	mat4(float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33) {
		m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
		m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
		m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
		m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
	}

	mat4 operator*(const mat4& right) {
		mat4 result;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				result.m[i][j] = 0;
				for (int k = 0; k < 4; k++) result.m[i][j] += m[i][k] * right.m[k][j];
			}
		}
		return result;
	}

	

	mat4 operator*(const float f) {
		mat4 result;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				result.m[i][j] = m[i][j] * f;
			}
		}
		return result;
	}

	mat4 operator+(const mat4& right) {
		mat4 result;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				result.m[i][j] = m[i][j] + right.m[i][j];
			}
		}
		return result;
	}

	operator float*() { return &m[0][0]; }
};



// 3D point in homogeneous coordinates
struct vec4 {
	float v[4];

	vec4(float x = 0, float y = 0, float z = 0, float w = 1) {
		v[0] = x; v[1] = y; v[2] = z; v[3] = w;
	}

	vec4 operator*(const mat4& mat) {
		vec4 result;
		for (int j = 0; j < 4; j++) {
			result.v[j] = 0;
			for (int i = 0; i < 4; i++) result.v[j] += v[i] * mat.m[i][j];
		}
		return result;
	}

	vec4 operator+(const vec4& other) {
		vec4 result;
		for (int i = 0; i < 4; i++) {
			result.v[i] = v[i] + other.v[i];
		}
		return result;
	}

	vec4 operator-(const vec4& other) {
		vec4 result;
		for (int i = 0; i < 4; i++) {
			result.v[i] = v[i] - other.v[i];
		}
		return result;
	}

	vec4 operator/(const float f) {
		vec4 result;
		for (int i = 0; i < 4; i++) {
			result.v[i] = v[i] / f;
		}
		return result;
	}

	vec4 operator*(const float f) {
		vec4 result;
		for (int i = 0; i < 4; i++) {
			result.v[i] = v[i] * f;
		}
		return result;
	}

	float operator*(const vec4& other) {
		float result=0.0f;
		for (int i = 0; i < 4; i++) {
			result += v[i] * other.v[i];
		}
		return result;
	}

	vec4 operator<(const vec4& other) {
		vec4 result;
		result = { v[1] * other.v[2] - v[2] * other.v[1],
					-v[0] * other.v[2] + v[2] * other.v[0],
					v[0] * other.v[1] - v[1] * other.v[0],
					1
		};
		result = result / sqrtf(result.v[0] * result.v[0] + result.v[1] * result.v[1] + result.v[2] * result.v[2]);
		return result;
	}

	
};

float length(vec4 vector) {
	return sqrtf(vector.v[0] * vector.v[0] + vector.v[1] * vector.v[1] + vector.v[2] * vector.v[2]);
}

vec4 MxV(mat4 M, vec4 V) {
	vec4 result;
	for (int i = 0; i < 4; i++) {
		result.v[i] = 0;
		for (int j = 0; j < 4; j++) {
			result.v[i] += M.m[i][j] * V.v[j];
		}
	}
	return result;
}

// 2D camera
struct Camera {
	float wCx, wCy, wCz;	// center in world coordinates
	float wWx, wWy, wWz;	// width and height in world coordinates
public:
	Camera() {
		Animate(0);
	}

	mat4 V() { // view matrix: translates the center to the origin
		return mat4(1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			-wCx, -wCy, -wCz, 1);
	}

	mat4 P() { // projection matrix: scales it to be a square of edge length 2
		return mat4(2 / wWx, 0, 0, 0,
			0, 2 / wWy, 0, 0,
			0, 0, 2/wWz, 0,
			0, 0, 0, 1);
	}

	mat4 Vinv() { // inverse view matrix
		return mat4(1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			wCx, wCy, wCz, 1);
	}

	mat4 Pinv() { // inverse projection matrix
		return mat4(wWx / 2, 0, 0, 0,
			0, wWy / 2, 0, 0,
			0, 0, wWz/2, 0,
			0, 0, 0, 1);
	}

	void Animate(float t) {
		wCx = 0; // 10 * cosf(t);
		wCy = 0;
		wCz = 0;
		wWx = 20;
		wWy = 20;
		wWz = 20;
	}
};

// 2D camera
Camera camera;

// handle of the shader program
unsigned int shaderProgram;

class Triangle {
	unsigned int vao;	// vertex array object id
	float sx, sy;		// scaling
	float wTx, wTy;		// translation
public:
	Triangle() {
		Animate(0);
	}

	void Create() {
		glGenVertexArrays(1, &vao);	// create 1 vertex array object
		glBindVertexArray(vao);		// make it active

		unsigned int vbo[2];		// vertex buffer objects
		glGenBuffers(2, &vbo[0]);	// Generate 2 vertex buffer objects

									// vertex coordinates: vbo[0] -> Attrib Array 0 -> vertexPosition of the vertex shader
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // make it active, it is an array
		static float vertexCoords[] = { -8, -8, -6, 10, 8, -2 };	// vertex data on the CPU
		glBufferData(GL_ARRAY_BUFFER,      // copy to the GPU
			sizeof(vertexCoords),  // number of the vbo in bytes
			vertexCoords,		   // address of the data array on the CPU
			GL_STATIC_DRAW);	   // copy to that part of the memory which is not modified 
								   // Map Attribute Array 0 to the current bound vertex buffer (vbo[0])
		glEnableVertexAttribArray(0);
		// Data organization of Attribute Array 0 
		glVertexAttribPointer(0,			// Attribute Array 0
			2, GL_FLOAT,  // components/attribute, component type
			GL_FALSE,		// not in fixed point format, do not normalized
			0, NULL);     // stride and offset: it is tightly packed

						  // vertex colors: vbo[1] -> Attrib Array 1 -> vertexColor of the vertex shader
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // make it active, it is an array
		static float vertexColors[] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };	// vertex data on the CPU
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexColors), vertexColors, GL_STATIC_DRAW);	// copy to the GPU

																							// Map Attribute Array 1 to the current bound vertex buffer (vbo[1])
		glEnableVertexAttribArray(1);  // Vertex position
									   // Data organization of Attribute Array 1
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL); // Attribute Array 1, components/attribute, component type, normalize?, tightly packed
	}

	void Animate(float t) {
		sx = 1; // *sinf(t);
		sy = 1; // *cosf(t);
		wTx = 0; // 4 * cosf(t / 2);
		wTy = 0; // 4 * sinf(t / 2);
	}

	void Draw() {
		mat4 M(sx, 0, 0, 0,
			0, sy, 0, 0,
			0, 0, 0, 0,
			wTx, wTy, 0, 1); // model matrix

		mat4 MVPTransform = M * camera.V() * camera.P();

		// set GPU uniform matrix variable MVP with the content of CPU variable MVPTransform
		int location = glGetUniformLocation(shaderProgram, "MVP");
		if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, MVPTransform); // set uniform variable MVP to the MVPTransform
		else printf("uniform MVP cannot be set\n");

		glBindVertexArray(vao);	// make the vao and its vbos active playing the role of the data source
		glDrawArrays(GL_TRIANGLES, 0, 3);	// draw a single triangle with vertices defined in vao
	}
};

float R = 40000.0f / 5000.0f;
float omega = 0.25;
int state = 0;



class CircleLongitude {
	unsigned int vao;
	float sx, sy, sz;
	float wTx, wTy, wTz;
	float t;
public:
	CircleLongitude() {
		Animate(0);
	}

	void Create(bool isSil) {
		glGenVertexArrays(1, &vao);	// create 1 vertex array object
		glBindVertexArray(vao);		// make it active

		unsigned int vbo[2];		// vertex buffer objects 
		glGenBuffers(2, &vbo[0]);	// Generate 2 vertex buffer objects

									// vertex coordinates: vbo[0] -> Attrib Array 0 -> vertexPosition of the vertex shader
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // make it active, it is an array
		static float vertexCoords[120];	// vertex data on the CPU

		for (int i = 0; i < 40; i++) {
			float cX = R*cosf(3.14 / 180 * i * 360 / 39);
			float cY = R*sinf(3.14 / 180 * i * 360 / 39);
			float cZ = 0;
			vertexCoords[3 * i] = cX;
			vertexCoords[3 * i + 1] = cY;
			vertexCoords[3 * i + 2] = cZ;
		}


		glBufferData(GL_ARRAY_BUFFER,      // copy to the GPU
			sizeof(vertexCoords),  // number of the vbo in bytes
			vertexCoords,		   // address of the data array on the CPU
			GL_STATIC_DRAW);	   // copy to that part of the memory which is not modified 
								   // Map Attribute Array 0 to the current bound vertex buffer (vbo[0])
		glEnableVertexAttribArray(0);
		// Data organization of Attribute Array 0 
		glVertexAttribPointer(0,			// Attribute Array 0
			3, GL_FLOAT,  // components/attribute, component type
			GL_FALSE,		// not in fixed point format, do not normalized
			0, NULL);     // stride and offset: it is tightly packed

						  // vertex colors: vbo[1] -> Attrib Array 1 -> vertexColor of the vertex shader
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // make it active, it is an array
		static float vertexColors[120];	// vertex data on the CPU
		for (int i = 0; i < 40; i++) {
			if (isSil) {
				vertexColors[3 * i] = 1;
				vertexColors[3 * i + 1] = 1;
				vertexColors[3 * i + 2] = 1;
			}
			else {
				vertexColors[3 * i] = 0.5;
				vertexColors[3 * i + 1] = 0.5;
				vertexColors[3 * i + 2] = 0.5;
			}
		}
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexColors), vertexColors, GL_STATIC_DRAW);	// copy to the GPU

																							// Map Attribute Array 1 to the current bound vertex buffer (vbo[1])
		glEnableVertexAttribArray(1);  // Vertex position
									   // Data organization of Attribute Array 1
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL); // Attribute Array 1, components/attribute, component type, normalize?, tightly packed
	}

	void Animate(float k) {
		sx = 1; // *sinf(t);
		sy = 1; // *cosf(t);
		sz = 1;
		wTx = 0; // 4 * cosf(t / 2);
		wTy = 0; // 4 * sinf(t / 2);
		wTz = 0;
		t = k;
	}

	void Draw(float alfa) {
		mat4 anim(
			cosf(0.2 * t), 0, sinf(0.2 * t), 0,
			0, 1, 0, 0,
			-sinf(0.2 * t), 0, cosf(0.2 * t), 0,
			0, 0, 0, 1
		);

		mat4 rotate(
			cosf(alfa), 0, sinf(alfa), 0,
			0, 1, 0, 0,
			-sinf(alfa), 0, cosf(alfa), 0,
			0, 0, 0, 1
		);

		mat4 M(sx, 0, 0, 0,
			0, sy, 0, 0,
			0, 0, sz, 0,
			wTx, wTy, wTz, 1); // model matrix

		mat4 MVPTransform = M * rotate * anim * camera.V() * camera.P();

		// set GPU uniform matrix variable MVP with the content of CPU variable MVPTransform
		int location = glGetUniformLocation(shaderProgram, "MVP");
		if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, MVPTransform); // set uniform variable MVP to the MVPTransform
		else printf("uniform MVP cannot be set\n");

		glBindVertexArray(vao);	// make the vao and its vbos active playing the role of the data source
		glDrawArrays(GL_LINE_STRIP, 0, 40);	// draw a single triangle with vertices defined in vao
	}
};

class CircleLatitude {
	unsigned int vao;
	float sx, sy, sz;
	float wTx, wTy, wTz;
	float t;
public:
	CircleLatitude() {
		Animate(0);
	}

	void Create(int j) {
		glGenVertexArrays(1, &vao);	// create 1 vertex array object
		glBindVertexArray(vao);		// make it active

		unsigned int vbo[2];		// vertex buffer objects 
		glGenBuffers(2, &vbo[0]);	// Generate 2 vertex buffer objects

									// vertex coordinates: vbo[0] -> Attrib Array 0 -> vertexPosition of the vertex shader
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); // make it active, it is an array
		static float vertexCoords[120];	// vertex data on the CPU

		for (int i = 0; i < 40; i++) {
			float cX = R * cosf(3.14 / 180 * j * 30) * cosf(3.14 / 180 * i * 360 / 39);
			float cY = R * sinf(3.14 / 180 * j * 30);
			float cZ = R * cosf(3.14 / 180 * j * 30) * sinf(3.14 / 180 * i * 360 / 39);
			vertexCoords[3 * i] = cX;
			vertexCoords[3 * i + 1] = cY;
			vertexCoords[3 * i + 2] = cZ;
		}


		glBufferData(GL_ARRAY_BUFFER,      // copy to the GPU
			sizeof(vertexCoords),  // number of the vbo in bytes
			vertexCoords,		   // address of the data array on the CPU
			GL_STATIC_DRAW);	   // copy to that part of the memory which is not modified 
								   // Map Attribute Array 0 to the current bound vertex buffer (vbo[0])
		glEnableVertexAttribArray(0);
		// Data organization of Attribute Array 0 
		glVertexAttribPointer(0,			// Attribute Array 0
			3, GL_FLOAT,  // components/attribute, component type
			GL_FALSE,		// not in fixed point format, do not normalized
			0, NULL);     // stride and offset: it is tightly packed

						  // vertex colors: vbo[1] -> Attrib Array 1 -> vertexColor of the vertex shader
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]); // make it active, it is an array
		static float vertexColors[120];	// vertex data on the CPU
		for (int i = 0; i < 40; i++) {
			vertexColors[3 * i] = 0.5;
			vertexColors[3 * i + 1] = 0.5;
			vertexColors[3 * i + 2] = 0.5;
		}
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexColors), vertexColors, GL_STATIC_DRAW);	// copy to the GPU

																							// Map Attribute Array 1 to the current bound vertex buffer (vbo[1])
		glEnableVertexAttribArray(1);  // Vertex position
									   // Data organization of Attribute Array 1
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL); // Attribute Array 1, components/attribute, component type, normalize?, tightly packed
	}

	void Animate(float k) {
		sx = 1; // *sinf(t);
		sy = 1; // *cosf(t);
		sz = 1;
		wTx = 0; // 4 * cosf(t / 2);
		wTy = 0; // 4 * sinf(t / 2);
		wTz = 0;
		t = k;
	}

	void Draw(float alfa) {
		mat4 scale(
			cosf(alfa), 0, sinf(alfa), 0,
			0, 1, 0, 0,
			-sinf(alfa), 0, cosf(alfa), 0,
			0, 0, 0, 1
		);

		mat4 M(sx, 0, 0, 0,
			0, sy, 0, 0,
			0, 0, sz, 0,
			wTx, wTy, wTz, 1); // model matrix

		mat4 MVPTransform = M * scale * camera.V() * camera.P();

		// set GPU uniform matrix variable MVP with the content of CPU variable MVPTransform
		int location = glGetUniformLocation(shaderProgram, "MVP");
		if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, MVPTransform); // set uniform variable MVP to the MVPTransform
		else printf("uniform MVP cannot be set\n");

		glBindVertexArray(vao);	// make the vao and its vbos active playing the role of the data source
		glDrawArrays(GL_LINE_STRIP, 0, 40);	// draw a single triangle with vertices defined in vao
	}
};

class LineStrip {
	GLuint vao, vbo;        // vertex array object, vertex buffer object
	float  vertexData[1206]; // interleaved data of coordinates and colors
	int    nVertices;       // number of vertices
	int nControl;
	float sx, sy, sz;
	float wTx, wTy, wTz;
	float t;

public:
	LineStrip() {
		nVertices = 0;
		nControl = 0;
	}
	void Create() {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &vbo); // Generate 1 vertex buffer object
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		// Enable the vertex attribute arrays
		glEnableVertexAttribArray(0);  // attribute array 0
		glEnableVertexAttribArray(1);  // attribute array 1
									   // Map attribute array 0 to the vertex data of the interleaved vbo
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(0)); // attribute array, components/attribute, component type, normalize?, stride, offset
																										// Map attribute array 1 to the color data of the interleaved vbo
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
	}

	void AddPoint(float cX, float cY, float cZ, bool isLast) {
		if (nControl >= 19 && !isLast) return;
		
		mat4 animinv(
			cosf(-0.2 * t), 0, sinf(-0.2 * t), 0,
			0, 1, 0, 0,
			-sinf(-0.2 * t), 0, cosf(-0.2 * t), 0,
			0, 0, 0, 1
		);

		vec4 wVertex = vec4(cX, cY, cZ, 1) * animinv *  camera.Pinv() * camera.Vinv();
		
		if (nControl > 0) {
			vec4 previous = {vertexData[(nControl-1) * 10 * 6],
				vertexData[(nControl - 1) * 10 * 6 + 1],
				vertexData[(nControl - 1) * 10 * 6 + 2],
				1
			};

			float alfa = acosf(previous * wVertex / (length(previous) * length(wVertex)))/10;
			vec4 cross = wVertex < previous ;
			

			for (int i = 1; i < 11; i++) {
				mat4 rodrigues {
					cosf(alfa * i) + cross.v[0] * cross.v[0] * (1-cosf(alfa * i)), cross.v[0] * cross.v[1] * (1-cosf (alfa * i)) - cross.v[2] * sinf(alfa * i), cross.v[1] * sinf(alfa * i) + cross.v[0] * cross.v[2] * (1-cosf(alfa * i)), 0,
					cross.v[2] * sinf(alfa * i) + cross.v[0] * cross.v[1] * (1-cosf(alfa * i)), cosf(alfa * i) + cross.v[1] * cross.v[1] * (1-cosf(alfa * i)), -cross.v[0] * sinf(alfa * i) + cross.v[1] * cross.v[2] * (1-cosf(alfa * i)), 0,
					-cross.v[1] * sinf(alfa * i) + cross.v[0] * cross.v[2] * (1-cosf(alfa * i)), cross.v[0] * sinf(alfa * i) + cross.v[1] * cross.v[2] * (1- cosf(alfa * i)) , cosf(alfa * i) + cross.v[2] * cross.v[2] * (1-cosf(alfa * i)), 0,
					0, 0, 0, 1
				};

				vec4 result = previous * rodrigues ;
				vertexData[6 * nVertices] = result.v[0];
				vertexData[6 * nVertices + 1] = result.v[1];
				vertexData[6 * nVertices + 2] = result.v[2];
				vertexData[6 * nVertices + 3] = 1; // red
				vertexData[6 * nVertices + 4] = 0; // green
				vertexData[6 * nVertices + 5] = 0; // blue
				nVertices++;
			}
			nControl++;
		}
		else {
			vertexData[6 * nVertices] = wVertex.v[0];
			vertexData[6 * nVertices + 1] = wVertex.v[1];
			vertexData[6 * nVertices + 2] = wVertex.v[2];
			vertexData[6 * nVertices + 3] = 1; // red
			vertexData[6 * nVertices + 4] = 0; // green
			vertexData[6 * nVertices + 5] = 0; // blue
			nVertices++;
			nControl++;
		}
		
		// copy data to the GPU
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, nVertices * 6 * sizeof(float), vertexData, GL_DYNAMIC_DRAW);
	}

	void Draw() {
		mat4 M(sx, 0, 0, 0,
			0, sy, 0, 0,
			0, 0, sz, 0,
			wTx, wTy, wTz, 1); // model matrix

		mat4 anim(
			cosf(0.2 * t), 0, sinf(0.2 * t), 0,
			0, 1, 0, 0,
			-sinf(0.2 * t), 0, cosf(0.2 * t), 0,
			0, 0, 0, 1
		);

		if (nVertices > 0) {
			mat4 VPTransform = M * anim *  camera.V() * camera.P();

			int location = glGetUniformLocation(shaderProgram, "MVP");
			if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, VPTransform);
			else printf("uniform MVP cannot be set\n");

			glBindVertexArray(vao);
			glDrawArrays(GL_LINE_STRIP, 0, nVertices);
		}
	}

	void Animate(float k) {
		sx = 1; // *sinf(t);
		sy = 1; // *cosf(t);
		sz = 1;
		wTx = 0; // 4 * cosf(t / 2);
		wTy = 0; // 4 * sinf(t / 2);
		wTz = 0;
		t = k;
	}

	vec4 getVertexData(int i) {
		return{ vertexData[6 * i], vertexData[6 * i + 1], vertexData[6 * i + 2], 1 };
	}

	int getnControl(){
		return nControl;
	}

	void reconfigure() {
		nVertices = 0;
		nControl = 0;
	}
};



// The virtual world: collection of two objects
Triangle triangle;
LineStrip lineStrip;
CircleLongitude longitude;
CircleLongitude sil;
CircleLatitude latitude[5];
bool departure = false;
float referenceT;
bool end = true;

class Plane {
	GLuint vao, vbo;
	vec4 points[300];
	float vertexData[1511];
	vec4 controllPoints[11];
	float controllTime[11];
	vec4 vs[11];
	int nCp;
	float sx, sy, sz;		// scaling
	float wTx, wTy, wTz;		// translation
	float t;
	float departure_t = 0.0f;
public:
	Plane() {
		nCp = 0;
	}
	void Create() {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(0));

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
	}

	void AddPoint(float cX, float cY, float sec) {
		if (nCp >= 10) return;

		vec4 wVertex = vec4(cX, cY, 0, 1) * camera.Pinv() * camera.Vinv();
		controllPoints[nCp] = wVertex;
		controllTime[nCp] = sec;
		nCp++;

		if (nCp == 10) {
			controllTime[10] = controllTime[9] + 0.5f;

			float timeBegining = controllTime[0];
			controllTime[0] = 0;
			for (int i = 1; i < 11; i++) {
				controllTime[i] = controllTime[i] - timeBegining;
			}

			controllPoints[10] = controllPoints[0];

			for (int i = 0; i <= nCp; i++) {
				int before;
				int after;
				if (i == 0) before = nCp;
				else before = i - 1;
				after = i + 1;
				if (i == 10)
					vs[10] = vs[0];
				else
					vs[i] = velocity(controllPoints[before], controllPoints[i], controllPoints[after], controllTime[before], controllTime[i], controllTime[after]);
			}

			for (int i = 0; i < 300; i++) {
				points[i] = r(controllTime[10] / 200.0f*float(i));

				vertexData[5 * i] = points[i].v[0];
				vertexData[5 * i + 1] = points[i].v[1];
				vertexData[5 * i + 2] = 0;
				vertexData[5 * i + 3] = 1;
				vertexData[5 * i + 4] = 0;
			}

			float temp[1516];
			temp[0] = 0;
			temp[1] = 0;
			temp[2] = 0;
			temp[3] = 1;
			temp[4] = 0;
			for (int i = 0; i < 300; i++) {
				temp[5 + 5 * i] = vertexData[5 * i];
				temp[5 + 5 * i + 1] = vertexData[5 * i + 1];
				temp[5 + 5 * i + 2] = vertexData[5 * i + 2];
				temp[5 + 5 * i + 3] = vertexData[5 * i + 3];
				temp[5 + 5 * i + 4] = vertexData[5 * i + 4];
			}


			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(temp), temp, GL_DYNAMIC_DRAW);
		}
	}

	void Draw(int segment) {
		vec4 i = lineStrip.getVertexData(segment*10+1) - lineStrip.getVertexData(segment*10);
		i = i / length(i);

		vec4 j = lineStrip.getVertexData(segment*10);
		j = j / length(j);

		vec4 k = i < j;

		vec4 START = lineStrip.getVertexData(segment*10);

		vec4 cross = lineStrip.getVertexData(segment*10+1) < lineStrip.getVertexData(segment*10);

		

		if (departure)
		{
			mat4 rodrigues{
				cosf(omega * (t-departure_t)) + cross.v[0] * cross.v[0] * (1 - cosf(omega * (t - departure_t))), cross.v[0] * cross.v[1] * (1 - cosf(omega * (t - departure_t))) - cross.v[2] * sinf(omega * (t - departure_t)), cross.v[1] * sinf(omega * (t - departure_t)) + cross.v[0] * cross.v[2] * (1 - cosf(omega * (t - departure_t))), 0,
				cross.v[2] * sinf(omega * (t - departure_t)) + cross.v[0] * cross.v[1] * (1 - cosf(omega * (t - departure_t))), cosf(omega * (t - departure_t)) + cross.v[1] * cross.v[1] * (1 - cosf(omega * (t - departure_t))), -cross.v[0] * sinf(omega * (t - departure_t)) + cross.v[1] * cross.v[2] * (1 - cosf(omega * (t - departure_t))), 0,
				-cross.v[1] * sinf(omega * (t - departure_t)) + cross.v[0] * cross.v[2] * (1 - cosf(omega * (t - departure_t))), cross.v[0] * sinf(omega * (t - departure_t)) + cross.v[1] * cross.v[2] * (1 - cosf(omega * (t - departure_t))) , cosf(omega * (t - departure_t)) + cross.v[2] * cross.v[2] * (1 - cosf(omega * (t - departure_t))), 0,
				0, 0, 0, 1
			};

			START = START * rodrigues;

			i = i * rodrigues;

			j = j * rodrigues;

			k = k * rodrigues;

			if (length(lineStrip.getVertexData((segment + 1) * 10) - START) < 1) {
				state++;
				departure_t = 0.0f;
				if (state + 1 >= lineStrip.getnControl()) {
					departure = false;
					state = 0;
					end = true;
				}
			}
		}

		mat4 M(sx*k.v[0], sy*k.v[1], sz*k.v[2], 0,
			sx*i.v[0], sy*i.v[1], sz*i.v[2], 0,
			sx*j.v[0], sy*j.v[1], sz*j.v[2], 0,
			START.v[0] * 1.025, START.v[1] * 1.025, START.v[2] * 1.025, 1
		);

		mat4 anim(
			cosf(0.2 * t), 0, sinf(0.2 * t), 0,
			0, 1, 0, 0,
			-sinf(0.2 * t), 0, cosf(0.2 * t), 0,
			0, 0, 0, 1
		);

		if (nCp >= 10) {
			mat4 MVPTransform = M * anim * camera.V() * camera.P();
			int location = glGetUniformLocation(shaderProgram, "MVP");
			if (location >= 0) glUniformMatrix4fv(location, 1, GL_TRUE, MVPTransform);
			else printf("uniform MVP cannot be set\n");
			glBindVertexArray(vao);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 202);
		}

	}

	void Animate(float k) {
		sx = 0.2; // *sinf(t);
		sy = 0.2; // *cosf(t);
		sz = 0.2;
		wTx = 0; // 4 * cosf(t / 2);
		wTy = 0; // 4 * sinf(t / 2);
		wTz = 0;
		t = k;
		if (departure && departure_t == 0.0f) {
			departure_t = k;
		}
	}

	vec4 r(float t) {
		if (nCp < 10) return vec4(0, 0, 0, 0);
		while (t > controllTime[10]) t = t - controllTime[10];
		for (int i = 0; i < 10; i++) {
			if (controllTime[i] <= t && t <= controllTime[i + 1])
				return hermite(controllPoints[i], controllPoints[i + 1], vs[i], vs[i + 1], controllTime[i], controllTime[i + 1], t);
		}
	}

	vec4 velocity(vec4 p_before, vec4 p, vec4 p_after, float t_before, float t, float t_after) {
		vec4 v;
		v = ((p_after - p) / (t_after - t) + (p - p_before) / (t - t_before))*0.9f;
		return v;
	}

	vec4 hermite(vec4 p_before, vec4 p_after, vec4 v_before, vec4 v_after, float t_before, float t_after, float t) {
		vec4 a0 = p_before;
		vec4 a1 = v_before;
		vec4 a2 = (p_after - p_before) * 3 / ((t_after - t_before)*(t_after - t_before)) - (v_after + v_before * 2) / (t_after - t_before);
		vec4 a3 = (p_before - p_after) * 2 / ((t_after - t_before)*(t_after - t_before)*(t_after - t_before)) + (v_after + v_before) / ((t_after - t_before)*(t_after - t_before));
		vec4 result = a3* ((t - t_before)*(t - t_before)*(t - t_before)) + a2 * ((t - t_before)*(t - t_before)) + a1 * (t - t_before) + a0;
		return result;
	}
};

Plane plane;

// Initialization, create an OpenGL context
void onInitialization() {
	glViewport(0, 0, windowWidth, windowHeight);
	glEnable(GL_DEPTH_TEST);

	// Create objects by setting up their vertex data on the GPU
	lineStrip.Create();
	longitude.Create(false);
	sil.Create(true);
	plane.Create();
	for (int i = -2; i < 3; i++) {
		latitude[i+2].Create(i);
	}
	plane.AddPoint(0, 0.5, 1);
	plane.AddPoint(0.2, 0.2, 1.5);
	plane.AddPoint(0.5, 0, 2);
	plane.AddPoint(0.2, -0.1, 3);
	plane.AddPoint(0.15, -0.4, 4);
	plane.AddPoint(0, -0.3, 5);
	plane.AddPoint(-0.15, -0.4, 6);
	plane.AddPoint(-0.2, -0.1, 7);
	plane.AddPoint(-0.5, 0, 8);
	plane.AddPoint(-0.2, 0.2, 8.5);
	plane.AddPoint(0, 0.5, 9);

	// Create vertex shader from string
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	if (!vertexShader) {
		printf("Error in vertex shader creation\n");
		exit(1);
	}
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	checkShader(vertexShader, "Vertex shader error");

	// Create fragment shader from string
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	if (!fragmentShader) {
		printf("Error in fragment shader creation\n");
		exit(1);
	}
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	checkShader(fragmentShader, "Fragment shader error");

	// Attach shaders to a single program
	shaderProgram = glCreateProgram();
	if (!shaderProgram) {
		printf("Error in shader program creation\n");
		exit(1);
	}
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	// Connect Attrib Arrays to input variables of the vertex shader
	glBindAttribLocation(shaderProgram, 0, "vertexPosition"); // vertexPosition gets values from Attrib Array 0
	glBindAttribLocation(shaderProgram, 1, "vertexColor");    // vertexColor gets values from Attrib Array 1

															  // Connect the fragmentColor to the frame buffer memory
	glBindFragDataLocation(shaderProgram, 0, "fragmentColor");	// fragmentColor goes to the frame buffer memory

																// program packaging
	glLinkProgram(shaderProgram);
	checkLinking(shaderProgram);
	// make this program run
	glUseProgram(shaderProgram);
}

void onExit() {
	glDeleteProgram(shaderProgram);
	printf("exit");
}

// Window has become invalid: Redraw
void onDisplay() {
	glClearColor(0, 0, 0, 0);							// background color 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the screen

	for (int i = 0; i < 6; i++) {
		longitude.Draw(i * 3.14 / 180 * 30);
	}
	for (int i = 0; i < 5; i++) {
		latitude[i].Draw(0);
	}
	sil.Draw(0);
	if(!end)
		lineStrip.Draw();
	if(departure)
		plane.Draw(state);
	glutSwapBuffers();									// exchange the two buffers
}

// Key of ASCII code pressed
void onKeyboard(unsigned char key, int pX, int pY) {
	if (key == 'd') glutPostRedisplay();         // if d, invalidate display, i.e. redraw
}

// Key of ASCII code released
void onKeyboardUp(unsigned char key, int pX, int pY) {

}

// Mouse click event
void onMouse(int button, int state, int pX, int pY) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && !departure) {  // GLUT_LEFT_BUTTON / GLUT_RIGHT_BUTTON and GLUT_DOWN / GLUT_UP
		float cX = 2.0f * pX / windowWidth - 1;	
		float cY = 1.0f - 2.0f * pY / windowHeight; // flip y axis
		float cZ = - sqrtf(R/10 * R/10 - cX * cX - cY * cY) ;
		if (cX * cX + cY * cY <= R / 10 * R / 10)
			lineStrip.AddPoint(cX, cY, cZ, false);
		end = false;
		glutPostRedisplay();     // redraw
	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN && !departure) {  // GLUT_LEFT_BUTTON / GLUT_RIGHT_BUTTON and GLUT_DOWN / GLUT_UP
		float cX = 2.0f * pX / windowWidth - 1;
		float cY = 1.0f - 2.0f * pY / windowHeight; // flip y axis
		float cZ = -sqrtf(R / 10 * R / 10 - cX * cX - cY * cY);
		if (cX * cX + cY * cY <= R / 10 * R / 10)
			lineStrip.AddPoint(cX, cY, cZ, true);
		departure = true;
		end = false;
		glutPostRedisplay();     // redraw
	}
}

// Move mouse with key pressed
void onMouseMotion(int pX, int pY) {
}

// Idle event indicating that some time elapsed: do animation here
void onIdle() {
	long time = glutGet(GLUT_ELAPSED_TIME); // elapsed time since the start of the program
	float sec = time / 1000.0f;				// convert msec to sec
	camera.Animate(sec);					// animate the camera
	longitude.Animate(sec);
	lineStrip.Animate(sec);
	plane.Animate(sec);
	if (end) {
		lineStrip.reconfigure();
	}
	glutPostRedisplay();					// redraw the scene
}

// Idaig modosithatod...
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int main(int argc, char * argv[]) {
	glutInit(&argc, argv);
#if !defined(__APPLE__)
	glutInitContextVersion(majorVersion, minorVersion);
#endif
	glutInitWindowSize(windowWidth, windowHeight);				// Application window is initially of resolution 600x600
	glutInitWindowPosition(100, 100);							// Relative location of the application window
#if defined(__APPLE__)
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);  // 8 bit R,G,B,A + double buffer + depth buffer
#else
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
	glutCreateWindow(argv[0]);

#if !defined(__APPLE__)
	glewExperimental = true;	// magic
	glewInit();
#endif

	printf("GL Vendor    : %s\n", glGetString(GL_VENDOR));
	printf("GL Renderer  : %s\n", glGetString(GL_RENDERER));
	printf("GL Version (string)  : %s\n", glGetString(GL_VERSION));
	glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
	glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
	printf("GL Version (integer) : %d.%d\n", majorVersion, minorVersion);
	printf("GLSL Version : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	onInitialization();

	glutDisplayFunc(onDisplay);                // Register event handlers
	glutMouseFunc(onMouse);
	glutIdleFunc(onIdle);
	glutKeyboardFunc(onKeyboard);
	glutKeyboardUpFunc(onKeyboardUp);
	glutMotionFunc(onMouseMotion);

	glutMainLoop();
	onExit();
	return 1;
}
