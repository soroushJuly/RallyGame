#pragma once

#include "Common.h"
#include "Camera.h"
#include "GameWindow.h"
#include "MatrixStack.h"


// Classes used in game.  For a new class, declare it here and provide a pointer to an object of this class below.  Then, in Game.cpp, 
// include the header.  In the Game constructor, set the pointer to NULL and in Game::Initialise, create a new object.  Don't forget to 
// delete the object in the destructor.   
class CCamera;
class CSkybox;
class CShader;
class CShaderProgram;
class CPlane;
class CFreeTypeFont;
class CHighResolutionTimer;
class CAudio;
class CTorus;
class CArrow;
class CCube;
class CCatmullRom;
class Actor;
class HUD;
class Quad;
class CFrameBufferObject;
class PlayerPawn;
//class MatrixStack;

enum GameStates
{
	MENU,
	PLAY,
	FINISH,
};

const static glm::vec3 Y_AXIS = glm::vec3(0.f,1.0f,0.f);

class Game {
private:
	// Three main methods used in the game.  Initialise runs once, while Update and Render run repeatedly in the game loop.
	void Initialise();
	void Update();
	void Render();
	void RenderScene(int pass);

	// Pointers to game objects.  They will get allocated in Game::Initialise()
	shared_ptr<CSkybox> m_pSkybox;
	vector <CShaderProgram *> *m_pShaderPrograms;
	CPlane *m_pPlanarTerrain;
	CFreeTypeFont *m_pFtFont;
	CHighResolutionTimer *m_pHighResolutionTimer;
	CAudio *m_pAudio;
	CFrameBufferObject* m_pFBO;
	unsigned int fbo;
	unsigned int texture;
	CFrameBufferObject* m_pFBO1;
	unsigned int fbo1;
	unsigned int texture1;
	shared_ptr<CCatmullRom> m_catmull;

	CCamera::States m_cameraState = CCamera::States::FREE;
	GameStates gameState = GameStates::PLAY;

	//Actor *m_car;
	shared_ptr<Quad> m_quad;
	shared_ptr<CArrow> m_arrow;
	shared_ptr<Actor> m_barrier;
	vector<glm::vec3> m_rocksList;
	vector<glm::vec3> m_timePowerUpList;
	vector<glm::vec3> m_turboPowerUpList;
	vector<glm::vec3> m_torusList;
	shared_ptr<Actor> m_rock;
	shared_ptr<Actor> m_cactus;
	shared_ptr<Actor> m_weed;
	shared_ptr<Actor> m_startingPoint;
	shared_ptr<Actor> m_stopwatch;
	shared_ptr<CTorus> m_torus;
	shared_ptr<CCamera> m_pCamera;
	shared_ptr<PlayerPawn> m_car;
	//shared_ptr<CCube> m_cube;

	shared_ptr<HUD> m_hud;

	// Some other member variables
	double m_dt;
	double m_timePassed;
	double m_playerTime;
	int m_framesPerSecond;
	bool m_appActive;

	float m_currentDistance;
	float diversionFromCenter;

	float m_t; 
	glm::vec3 m_spaceShipPosition; 
	glm::vec3 forwardVec;
	glm::vec3 rightVec;
	glm::mat4 m_spaceShipOrientation;

	glm::mat4* barriersModelMatrices;

	int m_width;
	int m_height;
public:
	Game();
	~Game();
	static Game& GetInstance();
	LRESULT ProcessEvents(HWND window,UINT message, WPARAM w_param, LPARAM l_param);
	void SetHinstance(HINSTANCE hinstance);
	WPARAM Execute();

private:
	static const int FPS = 60;
	void AddLights(CShaderProgram* shader, glm::mat4 viewMatrix, glm::mat3 viewNormalMatrix);
	void DisplayFrameRate();
	void RenderObject(glutil::MatrixStack* modelViewMatrixStack, CShaderProgram* shaderProgram, CCamera* camera, Actor* actor);
	void GameLoop();
	GameWindow m_gameWindow;
	HINSTANCE m_hInstance;
	int m_frameCount;
	double m_elapsedTime;

	const float PATH_WIDTH = 40.0f;

	vector<int> m_randomFoliage;
	bool isLightsOn = true;
	bool isNight = false;
	float toggleOffset = 0;
	double lightToggleTime = 0;
	double sunToggleTime = 0;
	float distanceTraveled = 0.f;

	int m_roadSize;

	glm::vec4 light;
};
