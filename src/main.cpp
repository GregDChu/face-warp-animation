/*
ZJ Wood CPE 471 Lab 3 base code
*/

#include <iostream>
#include <glad/glad.h>
#include <stdlib.h>
#include <time.h>

#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "WindowManager.h"
#include "Shape.h"
#include "WeightNode.h"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
using namespace glm;
vector<shared_ptr<Shape>> faces;
shared_ptr<Shape> box;

double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}
class camera
{
public:
	glm::vec3 pos, rot;
	int w, a, s, d;
	camera()
	{
		w = a = s = d = 0;
		pos = glm::vec3(0, 0, -10);
		rot = glm::vec3(0, 0, 0);
	}
	glm::mat4 process(double ftime)
	{
		float speed = 0;
		if (w == 1)
		{
			speed = 10*ftime;
		}
		else if (s == 1)
		{
			speed = -10*ftime;
		}
		float yangle=0;
		if (a == 1)
			yangle = -1*ftime;
		else if(d==1)
			yangle = 1*ftime;
		rot.y += yangle;
		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::vec4 dir = glm::vec4(0, 0, speed,1);
		dir = dir*R;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return R*T;
	}
};

camera mycam;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog;
	std::shared_ptr<Program> bgprog;

	// Contains vertex information for OpenGL
	//GLuint VertexArrayIDA, VertexArrayIDB;
	GLuint VAO;

	// Data necessary to give our box to OpenGL
	//GLuint VertexBufferIDA, VertexColorIDBox, IndexBufferIDBoxA,
		//VertexBufferIDB, IndexBufferIDBoxB;
	vector<GLuint> VBOs;
	GLuint VBONorm;
	GLuint VIB;
	GLuint VBOVIndex;

	GLuint VAObox, VBOPos, VBOText, VBIbox;

	GLint selectedVert;
	vector<vec3> vertOffset;
	int leftRightFlag;
	int upDownFlag;
	vector<int> faceAnchors;
	vector<vec3> faceAnchorVecs;
	vector<vec3> faceInitVecs;
	vector<vec3> expresAnchorPos;

	GLuint BGTexture;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			upDownFlag = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			upDownFlag = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			upDownFlag = -1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			upDownFlag = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			leftRightFlag = -1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			leftRightFlag = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			leftRightFlag = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			leftRightFlag = 0;
		}
		if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		{
			selectedVert -= 1;
			selectedVert %= faceAnchors.size();
		}
		if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		{
			selectedVert += 1;
			selectedVert %= faceAnchors.size();
		}
	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;
		float newPt[2];
	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom()
	{
		
		//generate the VAO
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		string resourceDirectory = "../resources/" ;
		// Initialize mesh.
		vector<string> faceFile;
		faceFile.push_back("face.obj");
		faceFile.push_back("happy.obj");
		faceFile.push_back("sad.obj");
		faceFile.push_back("mad.obj");
		faceFile.push_back("scared.obj");
		
		for (int i = 0; i < faceFile.size(); i++)
		{
			shared_ptr<Shape> face = make_shared<Shape>();
			string tmpDir = resourceDirectory;
			face->loadMesh(tmpDir.append(faceFile.at(i)));
			face->resize();
			face->init();
			faces.push_back(face);
		}
		glBindVertexArray(VAO);
		for(int i = 0; i < faces.size(); i++)
		{
			GLuint VBO;
			glGenBuffers(1, &VBO);
			//set the current state to focus on our vertex buffer
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			VBOs.push_back(VBO);

			vector<float> faceVertices = faces.at(i)->posBuf[0];
			glBufferData(GL_ARRAY_BUFFER, faceVertices.size() * sizeof(float), faceVertices.data(), GL_STATIC_DRAW);
			glEnableVertexAttribArray(i);
			glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
		}

		glGenBuffers(1, &VBONorm);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, VBONorm);

		vector<float> faceNorms = faces.at(1)->norBuf[0];
		glBufferData(GL_ARRAY_BUFFER, faceNorms.size() * sizeof(float), faceNorms.data(), GL_STATIC_DRAW);
		glEnableVertexAttribArray(faceFile.size());
		glVertexAttribPointer(faceFile.size(), 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

		vector<float> smallestDistToAnchor;
		for (int i = 0; i < faceAnchors.size(); i++)
		{
			smallestDistToAnchor.push_back(distance(vec3(0, 0, 0), faceInitVecs.at(i)));
			faceAnchorVecs.push_back(faceInitVecs.at(i));
			vertOffset.push_back(vec3(0, 0, 0));
			expresAnchorPos.push_back(vec3(0, 0, 0));
		}
		for (int i = 0; i < faces.at(0)->posBuf[0].size() / 3; i++)
		{
			vec3 linkedPos = vec3(faces.at(0)->posBuf[0].at((i * 3)),
								faces.at(0)->posBuf[0].at((i * 3) + 1),
								faces.at(0)->posBuf[0].at((i * 3) + 2));
			for (int j = 0; j < faceAnchors.size(); j++)
			{
				float dist = distance(linkedPos, faceInitVecs.at(j));
				if (dist < smallestDistToAnchor.at(j))
				{
					smallestDistToAnchor.at(j) = dist;
					faceAnchors.at(j) = i;
					faceAnchorVecs.at(j) = linkedPos;
				}
			}
		}

		glGenBuffers(1, &VIB);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VIB);
		vector<GLuint> faceIndicies = faces.at(1)->eleBuf[0];

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, faceIndicies.size() * sizeof(GLuint), faceIndicies.data(), GL_STATIC_DRAW);
		

		//GLuint VAObox, VBOPos, VBOText;
		glGenVertexArrays(1, &VAObox);
		glBindVertexArray(VAObox);

		glGenBuffers(1, &VBOPos);
		glBindBuffer(GL_ARRAY_BUFFER, VBOPos);

		GLfloat boxPos[] =
		{ 
			1, 1, -1,
			1, -1, -1,
			-1, -1, -1,
			-1, 1, -1
		};
		glBufferData(GL_ARRAY_BUFFER, sizeof(boxPos), boxPos, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
		
		glGenBuffers(1, &VBOText);
		glBindBuffer(GL_ARRAY_BUFFER, VBOText);
		vec2 boxTex[] =
		{
			vec2(1, 0),
			vec2(1, 1),
			vec2(0, 1),
			vec2(0, 0)
		};
		glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), boxTex, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

		glGenBuffers(1, &VBIbox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBIbox);
		GLushort boxIndex[] =
		{
			0, 1, 2,
			2, 3, 0
		};
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(boxIndex), boxIndex, GL_STATIC_DRAW);

		glBindVertexArray(0);

		int width, height, channels;
		char filepath[1000];
		
		//texture 1
		string str = resourceDirectory + "/backgroundblur.jpg";
		strcpy(filepath, str.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &BGTexture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, BGTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (const void*)data);
		glGenerateMipmap(GL_TEXTURE_2D);
		GLuint Tex1Location = glGetUniformLocation(bgprog->pid, "tex");//tex, tex2... sampler in the fragment shader
		// Then bind the uniform samplers to texture units:
		glUseProgram(bgprog->pid);
		glUniform1i(Tex1Location, 0);

	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		//Enable transparency
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Initialize the GLSL program.
		prog = std::make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("campos");
		prog->addUniform("t");
		prog->addUniform("lightPos");
		prog->addUniform("faceState");
		prog->addUniform("anchors");
		prog->addUniform("selectedAnchor");
		prog->addUniform("vertOffset");
		prog->addAttribute("face");
		prog->addAttribute("happy");
		prog->addAttribute("sad");
		prog->addAttribute("mad");
		prog->addAttribute("scared");
		prog->addAttribute("vertNor");
		
		bgprog = std::make_shared<Program>();
		bgprog->setVerbose(true);
		bgprog->setShaderNames(resourceDirectory + "/bg_vertex.glsl", resourceDirectory + "/bg_fragment.glsl");
		bgprog->init();
		bgprog->addUniform("P");
		bgprog->addUniform("V");
		bgprog->addUniform("M");
		bgprog->addAttribute("vertPos");
		bgprog->addAttribute("vertTex");

		// Init rand
		srand(time(NULL));

		// Init current selected vert (ID)
		selectedVert = 0;

		// Init control falgs
		leftRightFlag = 0;
		upDownFlag = 0;

		// Face Anchor Key Points
		// Center forehead
		faceAnchors.push_back(0);
		faceInitVecs.push_back(vec3(0, 0.6, 1));
		// Right forehead
		faceAnchors.push_back(0);
		faceInitVecs.push_back(vec3(0.4, 0.6, 1));
		// Right undereye
		faceAnchors.push_back(0);
		faceInitVecs.push_back(vec3(0.2, 0.3, 0.5));
		// Right mouth
		faceAnchors.push_back(0);
		faceInitVecs.push_back(vec3(0.8, -0.2, 1));
		// Center bottom mouth
		faceAnchors.push_back(0);
		faceInitVecs.push_back(vec3(0, -0.2, 1));
		// Left mouth
		faceAnchors.push_back(0);
		faceInitVecs.push_back(vec3(-0.8, -0.2, 1));
		// Left undereye
		faceAnchors.push_back(0);
		faceInitVecs.push_back(vec3(-0.2, 0.3, 0.5));
		// Left forehead
		faceAnchors.push_back(0);
		faceInitVecs.push_back(vec3(-0.4, 0.6, 1));
	}


	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render(GLFWwindow *window)
	{
		double frametime = get_last_elapsed_time();

		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width/(float)height;
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now
		
		glm::mat4 V, M, P; //View, Model and Perspective matrix
		V = glm::mat4(1);
		M = glm::mat4(1);
		// Apply orthographic projection....
		P = glm::ortho(-1 * aspect, 1 * aspect, -1.0f, 1.0f, -2.0f, 100.0f);		
		if (width < height)
			{
			P = glm::ortho(-1.0f, 1.0f, -1.0f / aspect,  1.0f / aspect, -2.0f, 100.0f);
			}
		// ...but we overwrite it (optional) with a perspective projection.
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width/ (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones

		//animation with the model matrix:
		static float w = 0.0;
		w += 1.0 * frametime;//rotation angle

#define ROUND_TIME 10.0 // in seconds
#define EMOT_TIME 2.0
		static int round = 0;
		bool printed = true;
		static float elapsedTime = ROUND_TIME;
		static float t = 0;
		elapsedTime += frametime;
		static float lastRoundTimeStamp = 0;
		static vec3 faceState = vec3(0, 0, 0);
		static bool emoting = false;
		static float emotTimeStamp = 0;

		if (elapsedTime - lastRoundTimeStamp > ROUND_TIME + EMOT_TIME)
		{
			lastRoundTimeStamp = elapsedTime;
			int a = rand() % 4;
			int b = rand() % 4;
			while (b == a)
			{
				b = rand() % 4;
			}
			float v = rand() % 11;
			faceState = vec3(a, v, b);

			// Calculate previous round's points
			float score = 0;
			for (int i = 0; i < faceAnchors.size(); i++)
			{
				float x = faces.at(0)->posBuf[0].at(faceAnchors.at(i) * 3);
				float y = faces.at(0)->posBuf[0].at((faceAnchors.at(i) * 3) + 1);
				float z = faces.at(0)->posBuf[0].at((faceAnchors.at(i) * 3) + 2);
				vec3 userPos = vec3(x, y, z) + vertOffset.at(i);
				float userDist = distance(expresAnchorPos.at(i), userPos);
				float neutralDist = distance(expresAnchorPos.at(i), vec3(x, y, z));
				float distFactor = clamp(userDist / neutralDist, 0.0f, 1.0f);
				if (neutralDist == 0 && userDist == 0)
				{
					distFactor = 0;
				}
				score += (100.0f / faceAnchors.size()) * (1.0 - distFactor);
			}
			cout << "Score: " << score << endl;

			// Get positions of all anchor points for A
			vector<vec3> anchorPosA;
			for (int i = 0; i < faceAnchors.size(); i++)
			{
				float x = faces.at(a + 1)->posBuf[0].at(faceAnchors.at(i) * 3);
				float y = faces.at(a + 1)->posBuf[0].at((faceAnchors.at(i) * 3) + 1);
				float z = faces.at(a + 1)->posBuf[0].at((faceAnchors.at(i) * 3) + 2);
				vec3 pos = vec3(x, y, z);
				anchorPosA.push_back(pos);
			}
			// Get positions of all anchor points for B
			vector<vec3> anchorPosB;
			for (int i = 0; i < faceAnchors.size(); i++)
			{
				float x = faces.at(b + 1)->posBuf[0].at(faceAnchors.at(i) * 3);
				float y = faces.at(b + 1)->posBuf[0].at((faceAnchors.at(i) * 3) + 1);
				float z = faces.at(b + 1)->posBuf[0].at((faceAnchors.at(i) * 3) + 2);
				vec3 pos = vec3(x, y, z);
				anchorPosB.push_back(pos);
			}
			// Get interpolated position
			for (int i = 0; i < faceAnchors.size(); i++)
			{
				float mix = pow(v * 0.1, 5);
				vec3 posInterp = (anchorPosA.at(i) * (float)(1.0 - mix)) +
									(anchorPosB.at(i) * mix);
				// Store positions for new rounds' anchors
				expresAnchorPos.at(i) = posInterp;
			}
			

			emoting = true;
			emotTimeStamp = elapsedTime;
			round++;
			printed = false;
			for (int i = 0; i < vertOffset.size(); i++)
			{
				vertOffset.at(i) = vec3(0, 0, 0);
			}
		}

		if (emoting && elapsedTime - emotTimeStamp < (EMOT_TIME / 4.0))
		{
			t = (elapsedTime - emotTimeStamp) / (EMOT_TIME / 4.0);
		}
		else if (emoting && elapsedTime - emotTimeStamp > (EMOT_TIME / 4.0)
			&& elapsedTime - emotTimeStamp < ((3 * EMOT_TIME) / 4.0))
		{
			t = 1.0;
		}
		else if (emoting && elapsedTime - emotTimeStamp < EMOT_TIME)
		{
			t = (EMOT_TIME - (elapsedTime - emotTimeStamp)) / (EMOT_TIME / 4.0);
		}
		else if (emoting)
		{
			t = 0;
			emoting = false;
		}

		if (!printed && round % 10 == 0)
		{
			cout << "Round " << round / 10 << " complete!" << endl;
			cout << "Next player's turn..." << endl;
		}
		if (!printed && round == 30)
		{
			cout << "Game complete!" << endl;
			cout << " won the game..." << endl;
			printed = true;
		}

		static vec3 lightPos = vec3(0, 0, 50);
		lightPos.x = cos(elapsedTime / 2) * 10;
		lightPos.y = (cos(elapsedTime / 2) * 5) + 10;

		if (!emoting && leftRightFlag < 0)
		{
			vertOffset.at(selectedVert).x -= 0.2 * frametime;
		}
		else if (!emoting && leftRightFlag > 0)
		{
			vertOffset.at(selectedVert).x += 0.2 * frametime;
		}
		if (!emoting && upDownFlag < 0)
		{
			vertOffset.at(selectedVert).y -= 0.2 * frametime;
		}
		else if (!emoting && upDownFlag > 0)
		{
			vertOffset.at(selectedVert).y += 0.2 * frametime;
		}

		// Draw the box using GLSL.
		prog->bind();
#define PI 3.1415926f
		V = mycam.process(frametime);
		//send the matrices to the shaders
		mat4 R = glm::rotate(mat4(1.0f), PI, vec3(0, 1, 0));
		mat4 S = glm::scale(mat4(1.0f), vec3(3, 3, 3));
		mat4 T = glm::translate(mat4(1.0f), vec3(0, sin(elapsedTime) / 10.0f, 0));
		M *= T * S;
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(prog->getUniform("campos"), 1, &mycam.pos[0]);
		glUniform1f(prog->getUniform("t"), t);
		glUniform3fv(prog->getUniform("faceState"), 1, &faceState[0]);
		glUniform3fv(prog->getUniform("lightPos"), 1, &lightPos[0]);

		glUniform1i(prog->getUniform("selectedAnchor"), selectedVert);
		glUniform3fv(prog->getUniform("vertOffset"), faceAnchors.size(), &vertOffset.at(0)[0]);
		glUniform3fv(prog->getUniform("anchors"), faceAnchors.size(), &faceAnchorVecs.at(0)[0]);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VIB);
		glDrawElements(GL_TRIANGLES, faces.at(0)->eleBuf->size() * sizeof(GLuint), GL_UNSIGNED_INT, (void*)0);
		prog->unbind();

		bgprog->bind();
		mat4 SBG = scale(mat4(1.0f), vec3(7.5, 4.5, 0));

		M = SBG;
		glUniformMatrix4fv(bgprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(bgprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(bgprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glBindVertexArray(VAObox);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBIbox);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0);
		
		bgprog->unbind();

	}

};
//******************************************************************************************
int main(int argc, char **argv)
{
	std::string resourceDir = "../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom();

	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render(windowManager->getHandle());

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
