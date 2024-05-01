/*
OpenGL Template for INM376 / IN3005
City University London, School of Mathematics, Computer Science and Engineering
Source code drawn from a number of sources and examples, including contributions from
 - Ben Humphrey (gametutorials.com), Michal Bubner (mbsoftworks.sk), Christophe Riccio (glm.g-truc.net)
 - Christy Quinn, Sam Kellett and others

 For educational use by Department of Computer Science, City University London UK.

 This template contains a skybox, simple terrain, camera, lighting, shaders, texturing

 Potential ways to modify the code:  Add new geometry types, shaders, change the terrain, load new meshes, change the lighting,
 different camera controls, different shaders, etc.

 Template version 5.0a 29/01/2017
 Dr Greg Slabaugh (gregory.slabaugh.1@city.ac.uk)

 version 6.0a 29/01/2019
 Dr Eddie Edwards (Philip.Edwards@city.ac.uk)

 version 6.1a 13/02/2022 - Sorted out Release mode and a few small compiler warnings
 Dr Eddie Edwards (Philip.Edwards@city.ac.uk)

*/

#include "game.h"


// Setup includes
#include "HighResolutionTimer.h"
#include "GameWindow.h"

// Game includes
#include "Camera.h"
#include "Skybox.h"
#include "Plane.h"
#include "Shaders.h"
#include "FreeTypeFont.h"
#include "MatrixStack.h"
#include "Audio.h"
#include "Arrow.h"
#include "Cube.h"
#include "HUD.h"
#include "Quad.h"
#include "Torus.h"
#include "Actor.h"
#include "FrameBufferObject.h"
#include "PlayerPawn.h"
#include "Common.h"
#include "CatmullRom.h"

// Constructor
Game::Game()
{
	m_pShaderPrograms = NULL;
	m_pPlanarTerrain = NULL;
	m_pFtFont = NULL;
	m_pHighResolutionTimer = NULL;
	m_pAudio = NULL;

	m_dt = 0.0;
	m_timePassed = 0.0;
	m_playerTime = 0.0;
	m_framesPerSecond = 0;
	m_frameCount = 0;
	m_elapsedTime = 0.0f;
	diversionFromCenter = 0.f;
	m_currentDistance = 0.f;
}

// Destructor
Game::~Game()
{
	//game objects
	delete m_pPlanarTerrain;
	delete m_pFtFont;
	delete m_pAudio;
	m_pFBO1->Release();
	m_pFBO->Release();

	if (m_pShaderPrograms != NULL) {
		for (unsigned int i = 0; i < m_pShaderPrograms->size(); i++)
			delete (*m_pShaderPrograms)[i];
	}
	delete m_pShaderPrograms;

	//setup objects
	delete m_pHighResolutionTimer;
}

// Initialisation:  This method only runs once at startup
void Game::Initialise()
{
	// Providing a seed value
	srand((unsigned)time(NULL));

	// Set the clear colour and depth
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f);

	/// Create objects
	m_pShaderPrograms = new vector <CShaderProgram*>;
	m_pPlanarTerrain = new CPlane;
	m_pFtFont = new CFreeTypeFont;
	m_pAudio = new CAudio;
	m_pFBO = new CFrameBufferObject(fbo, texture);
	m_pFBO1 = new CFrameBufferObject(fbo1, texture1);

	m_arrow = make_shared<CArrow>();
	m_pCamera = make_shared<CCamera>();
	m_barrier = make_shared<Actor>();
	m_rock = make_shared<Actor>();
	m_cactus = make_shared<Actor>();
	m_weed = make_shared<Actor>();
	m_stopwatch = make_shared<Actor>();
	m_startingPoint = make_shared<Actor>();
	m_torus = make_shared<CTorus>();
	m_quad = make_shared<Quad>();

	// Create the path
	m_catmull = make_shared<CCatmullRom>("resources\\textures\\dirt3.jpg", "resources\\textures\\sand.jpg");
	m_catmull->CreateCentreline();
	m_catmull->CreateOffsetCurves(PATH_WIDTH);
	m_catmull->CreateTrack();

	// Create and Initilize HUD
	m_hud = make_shared<HUD>();
	m_hud->Initilize();

	// Initilize camera to third person
	m_cameraState = CCamera::States::THIRD_PERSON;

	RECT dimensions = m_gameWindow.GetDimensions();

	m_width = dimensions.right - dimensions.left;
	m_height = dimensions.bottom - dimensions.top;

	// Quad to map the FBO on it.
	m_quad->Create(m_height, m_width);

	// FBOs to capture renders
	m_pFBO->Create(m_width, m_height);
	m_pFBO1->Create(m_width, m_height);

	// Set the orthographic and perspective projection matrices based on the image size
	m_pCamera->SetOrthographicProjectionMatrix(m_width, m_height);
	m_pCamera->SetPerspectiveProjectionMatrix(45.0f, (float)m_width / (float)m_height, 0.5f, 7000.0f);

	// Load shaders
	vector<CShader> shShaders;
	vector<string> sShaderFileNames;
	sShaderFileNames.push_back("mainShader.vert");
	sShaderFileNames.push_back("mainShader.frag");
	sShaderFileNames.push_back("textShader.vert");
	sShaderFileNames.push_back("textShader.frag");

	for (int i = 0; i < (int)sShaderFileNames.size(); i++) {
		string sExt = sShaderFileNames[i].substr((int)sShaderFileNames[i].size() - 4, 4);
		int iShaderType;
		if (sExt == "vert") iShaderType = GL_VERTEX_SHADER;
		else if (sExt == "frag") iShaderType = GL_FRAGMENT_SHADER;
		else if (sExt == "geom") iShaderType = GL_GEOMETRY_SHADER;
		else if (sExt == "tcnl") iShaderType = GL_TESS_CONTROL_SHADER;
		else iShaderType = GL_TESS_EVALUATION_SHADER;
		CShader shader;
		shader.LoadShader("resources\\shaders\\" + sShaderFileNames[i], iShaderType);
		shShaders.push_back(shader);
	}

	// Create the main shader program
	CShaderProgram* pMainProgram = new CShaderProgram;
	pMainProgram->CreateProgram();
	pMainProgram->AddShaderToProgram(&shShaders[0]);
	pMainProgram->AddShaderToProgram(&shShaders[1]);
	pMainProgram->LinkProgram();
	m_pShaderPrograms->push_back(pMainProgram);

	// Create a shader program for fonts
	CShaderProgram* pFontProgram = new CShaderProgram;
	pFontProgram->CreateProgram();
	pFontProgram->AddShaderToProgram(&shShaders[2]);
	pFontProgram->AddShaderToProgram(&shShaders[3]);
	pFontProgram->LinkProgram();
	m_pShaderPrograms->push_back(pFontProgram);

	// Create the skybox
	m_pSkybox = make_shared<CSkybox>();
	m_pSkybox->Create(2500.0f);

	// Create the planar terrain
	m_pPlanarTerrain->Create("resources\\textures\\", "sand.jpg", 4000.0f, 4000.0f, 200.0f);
	// Turbo Pickup
	m_arrow->Create("resources\\textures\\", "arrow.jpg", 1.6f, 1.4f, 3.f, 1.f);

	// Load font
	m_pFtFont->LoadFont("resources\\fonts\\segoeui.ttf", 64);
	m_pFtFont->SetShaderProgram(pFontProgram);

	// Random item generator (rock, powerup, etc)
	const int pathSectionSize = 25;
	for (int i = pathSectionSize; i < m_catmull->GetCenterPoints().size(); i += pathSectionSize)
	{
		int random = rand() % pathSectionSize;
		int random2 = rand() % pathSectionSize;
		int random3 = rand() % pathSectionSize;
		// PATH_WIDTH is the path width right now
		int randomWidth = rand() % ((int)PATH_WIDTH - 4) - (PATH_WIDTH / 2 - 2);
		int randomWidth2 = rand() % ((int)PATH_WIDTH - 4) - (PATH_WIDTH / 2 - 2);
		int randomWidth3 = rand() % ((int)PATH_WIDTH - 4) - (PATH_WIDTH / 2 - 2);
		glm::vec3 p = m_catmull->GetCenterPoints()[(i - 1) > 0 ? i - 1 : 0];
		glm::vec3 pNext = m_catmull->GetCenterPoints()[(i + 1) % m_catmull->GetCenterPoints().size()];

		glm::vec3 T = glm::normalize(pNext - p);
		glm::vec3 N = glm::normalize(glm::cross(T, Y_AXIS));
		glm::vec3 B = glm::normalize(glm::cross(N, T));

		m_rocksList.push_back(m_catmull->GetCenterPoints()[i + random] + (float)randomWidth * N);
		m_timePowerUpList.push_back(m_catmull->GetCenterPoints()[i + random2] + (float)randomWidth2 * N);
		m_turboPowerUpList.push_back(m_catmull->GetCenterPoints()[i + random3] + (float)randomWidth3 * N);
	}
	// Create cactus model
	m_cactus->Initialise("resources\\models\\Cactus\\untitled.obj");
	m_cactus->SetScale(glm::vec3(3.0f));
	// Load desert weed model
	m_weed->Initialise("resources\\models\\Weed\\untitled.obj");
	m_weed->SetScale(glm::vec3(1.0f));
	// Load time pickup model
	m_stopwatch->Initialise("resources\\models\\Stopwatch\\untitled.obj");
	m_stopwatch->SetScale(glm::vec3(135.0f));

	// Load the player mesh
	m_car = make_shared<PlayerPawn>();
	m_car->Initialise("resources\\models\\Jeep\\untitled.obj");
	// Set the position to the beginning of the map
	m_car->SetPosition(m_catmull->GetCenterPoints()[0]);
	// Load barrier model
	m_barrier->Initialise("resources\\models\\Fence\\fence.obj");
	int roadSize = m_catmull->GetLeftPoints().size();
	m_roadSize = roadSize;
	barriersModelMatrices = new glm::mat4[roadSize * 2];
	for (int i = 0; i < roadSize; i++)
	{
		glm::vec3 p = m_catmull->GetLeftPoints()[(i - 1) > 0 ? (i - 1) : 0];
		glm::vec3 pNext = m_catmull->GetLeftPoints()[(i + 1) % roadSize];

		glm::vec3 T = glm::normalize(pNext - p);
		glm::vec3 N = glm::normalize(glm::cross(T, Y_AXIS));
		glm::vec3 B = glm::normalize(glm::cross(N, T));

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, m_catmull->GetLeftPoints()[i]);
		// Road orientation
		model = model * glm::mat4(glm::mat3(T, B, N));
		model = glm::scale(model, glm::vec3(9.5f, 3.5f, 9.5f));
		barriersModelMatrices[i] = model;
	}
	for (int i = 0; i < roadSize; i++)
	{
		glm::vec3 p = m_catmull->GetRightPoints()[(i - 1) > 0 ? i - 1 : 0];
		glm::vec3 pNext = m_catmull->GetRightPoints()[(i + 1) % roadSize];

		glm::vec3 T = glm::normalize(pNext - p);
		glm::vec3 N = glm::normalize(glm::cross(T, Y_AXIS));
		glm::vec3 B = glm::normalize(glm::cross(N, T));

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, m_catmull->GetRightPoints()[i]);
		// Road orientation
		model = model * glm::mat4(glm::mat3(T, B, N));
		model = glm::scale(model, glm::vec3(9.5f, 3.5f, 9.5f));
		barriersModelMatrices[i + roadSize] = model;
	}

	m_startingPoint->Initialise("resources\\models\\Starting Line\\arch.obj");
	m_startingPoint->SetScale(glm::vec3(180.0f, 110.0f, 180.0f));
	m_startingPoint->SetPosition(m_catmull->GetCenterPoints()[1]);
	glm::vec3 T = glm::normalize(m_catmull->GetCenterPoints()[1] - m_catmull->GetCenterPoints()[0]);
	glm::vec3 N = glm::normalize(glm::cross(T, Y_AXIS));
	glm::vec3 B = glm::normalize(glm::cross(N, T));

	glm::mat4 startingOrientation = glm::mat4(glm::mat3(T, B, N));
	m_startingPoint->SetOrientation(startingOrientation);

	// Create random number helps to distrubute weeds and cactus
	for (int i = 0; i < 15; i++)
		m_randomFoliage.push_back(rand() % m_roadSize);

	m_rock->Initialise("resources\\models\\Rock\\newr.obj");
	m_rock->SetScale(glm::vec3(3.0f));

	m_torus->Create("resources\\textures\\", "rockwall.jpg", 4.f, .6f, 1.f, 5);

	glEnable(GL_CULL_FACE);

	// Initialise audio and play background music
	m_pAudio->Initialise();
}

// Render method runs repeatedly in a loop
void Game::RenderScene(int pass)
{

	// Clear the buffers and enable depth testing (z-buffering)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);


	// Set up a matrix stack
	glutil::MatrixStack modelViewMatrixStack;
	modelViewMatrixStack.SetIdentity();

	// Use the main shader program 
	CShaderProgram* pMainProgram = (*m_pShaderPrograms)[0];
	pMainProgram->UseProgram();
	pMainProgram->SetUniform("bUseTexture", true);
	pMainProgram->SetUniform("lightingOn", true);
	pMainProgram->SetUniform("sampler0", 0);
	pMainProgram->SetUniform("sampler1", 1);
	// Note: cubemap and non-cubemap textures should not be mixed in the same texture unit.  Setting unit 10 to be a cubemap texture.
	int cubeMapTextureUnit = 10;
	pMainProgram->SetUniform("CubeMapTex", cubeMapTextureUnit);

	// Set the projection matrix
	pMainProgram->SetUniform("matrices.projMatrix", m_pCamera->GetPerspectiveProjectionMatrix());

	// Call LookAt to create the view matrix and put this on the modelViewMatrix stack. 
	// Store the view matrix and the normal matrix associated with the view matrix for later (they're useful for lighting -- since lighting is done in eye coordinates)
	modelViewMatrixStack.LookAt(m_pCamera->GetPosition(), m_pCamera->GetView(), m_pCamera->GetUpVector());
	glm::mat4 viewMatrix = modelViewMatrixStack.Top();
	glm::mat3 viewNormalMatrix = m_pCamera->ComputeNormalMatrix(viewMatrix);
	// Add lights to the scene
	AddLights(pMainProgram, viewMatrix, viewNormalMatrix);
	// Print finish message and score when the game finishes
	if (gameState == GameStates::FINISH)
	{
		CShaderProgram* fontProgram = (*m_pShaderPrograms)[1];
		fontProgram->UseProgram();
		glDisable(GL_DEPTH_TEST);
		fontProgram->SetUniform("matrices.modelViewMatrix", glm::mat4(1));
		fontProgram->SetUniform("matrices.projMatrix", m_pCamera->GetOrthographicProjectionMatrix());
		fontProgram->SetUniform("vColour", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		m_pFtFont->Render(m_width / 2 - 100, m_height - 92, 20, "You finished the game!");
		m_pFtFont->Render(m_width / 2 - 100, m_height - 140, 20, "Your time: %d", (int)(m_playerTime / 1000));
	}
	else
	{
		// Final rendering with first and second pass already finished
		if (pass == 2)
		{
			modelViewMatrixStack.Push();
			modelViewMatrixStack.SetIdentity();
			modelViewMatrixStack.Translate(m_width / 2, m_height / 2, 0);
			modelViewMatrixStack.Scale(glm::vec3(.5f, .5f, .0f));
			glDisable(GL_DEPTH_TEST);
			pMainProgram->SetUniform("lightingOn", false);
			if (m_car->GetTurboStatus())
				pMainProgram->SetUniform("isBlur", true);
			else
				pMainProgram->SetUniform("isBlur", false);
			pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
			pMainProgram->SetUniform("matrices.projMatrix",
				m_pCamera->GetOrthographicProjectionMatrix());
			m_pFBO1->BindTexture(texture1);
			m_quad->RenderNoTexture();
			modelViewMatrixStack.Pop();
			pMainProgram->SetUniform("isBlur", false);

			/********************	RENDER HUD	  ****************/
			m_hud->Render(&modelViewMatrixStack, pMainProgram, m_pCamera, m_car, m_width, m_height);
			return;
		}
		// Render the skybox and terrain with full ambient reflectance 
		modelViewMatrixStack.Push();
		pMainProgram->SetUniform("renderSkybox", true);
		// Translate the modelview matrix to the camera eye point so skybox stays centred around camera
		glm::vec3 vEye = m_pCamera->GetPosition();
		modelViewMatrixStack.Translate(vEye);
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		m_pSkybox->Render(cubeMapTextureUnit);
		pMainProgram->SetUniform("renderSkybox", false);
		modelViewMatrixStack.Pop();

		// Turn on diffuse + specular materials for road and plane
		pMainProgram->SetUniform("material1.Ma", glm::vec3(0.6f));	// Ambient material reflectance
		pMainProgram->SetUniform("material1.Md", glm::vec3(0.7f));	// Diffuse material reflectance
		pMainProgram->SetUniform("material1.Ms", glm::vec3(0.5f));	// Specular material reflectance

		// Render the planar terrain
		modelViewMatrixStack.Push();
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		m_pPlanarTerrain->Render();
		modelViewMatrixStack.Pop();

		// Render Road
		modelViewMatrixStack.Push();
		pMainProgram->SetUniform("isMultiTexture", true); // turn on texturing
		pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
		pMainProgram->SetUniform("matrices.normalMatrix",
			m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
		m_catmull->RenderTrack();
		modelViewMatrixStack.Pop();
		pMainProgram->SetUniform("isMultiTexture", false); // turn on texturing

		// Turn on diffuse + specular materials
		pMainProgram->SetUniform("material1.Ma", glm::vec3(0.5f));	// Ambient material reflectance
		pMainProgram->SetUniform("material1.Md", glm::vec3(0.5f));	// Diffuse material reflectance
		pMainProgram->SetUniform("material1.Ms", glm::vec3(1.0f));	// Specular material reflectance	

		// Render the car
		m_car->Render(&modelViewMatrixStack, pMainProgram, m_pCamera);

		// Render the road barriers
		m_barrier->RenderInstanced(&modelViewMatrixStack, pMainProgram, m_pCamera, barriersModelMatrices, m_roadSize * 2);
		// Render torus tunnel
		pMainProgram->SetUniform("material1.shininess", 5.0f);		// Shininess material property
		for (int i = 0; i < 30; i++)
		{
			modelViewMatrixStack.Push();
			modelViewMatrixStack.Translate(m_catmull->GetCenterPoints()[30 + i]);
			modelViewMatrixStack.Scale(6.f);
			pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
			pMainProgram->SetUniform("matrices.normalMatrix",
				m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
			m_torus->Render();
			modelViewMatrixStack.Pop();
		}
		pMainProgram->SetUniform("material1.shininess", 15.0f);		// Shininess material property
		// Render rocks
		for (auto rockPosition : m_rocksList)
		{
			m_rock->SetPosition(rockPosition);
			m_rock->Render(&modelViewMatrixStack, pMainProgram, m_pCamera);
		}
		// Render Starting line
		m_startingPoint->Render(&modelViewMatrixStack, pMainProgram, m_pCamera);
		// Render stopwatches
		pMainProgram->SetUniform("isCartoon", true);
		for (auto timePosition : m_timePowerUpList)
		{
			m_stopwatch->SetPosition(timePosition + glm::vec3(0.f, 2.f, 0.f));
			m_stopwatch->SetRotation(Y_AXIS, (float)m_timePassed * 0.005f);
			m_stopwatch->Render(&modelViewMatrixStack, pMainProgram, m_pCamera);
		}
		// Render turbos
		for (auto turboPosition : m_turboPowerUpList)
		{
			modelViewMatrixStack.Push();
			modelViewMatrixStack.Translate(turboPosition + glm::vec3(0.f, 2.f, 0.f));
			modelViewMatrixStack.RotateY((float)(m_timePassed * 0.005));
			pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
			pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));
			m_arrow->Render();
			modelViewMatrixStack.Pop();
		}
		pMainProgram->SetUniform("isCartoon", false);
		// Render cactuses and weeds
		for (int i = 0; i < 15; i++)
		{
			modelViewMatrixStack.Push();
			if (m_randomFoliage[i] % 2 == 1)
				modelViewMatrixStack.Translate(m_catmull->GetLeftPoints()[(i * 30) % m_roadSize] + glm::vec3(3.f, 0.f, 3.f));
			else
				modelViewMatrixStack.Translate(m_catmull->GetRightPoints()[(i * 30) % m_roadSize] + glm::vec3(3.f, 0.f, 3.f));
			pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
			pMainProgram->SetUniform("matrices.normalMatrix", m_pCamera->ComputeNormalMatrix(modelViewMatrixStack.Top()));

			if (m_randomFoliage[i] % 3 == 1)
				m_cactus->Render(&modelViewMatrixStack, pMainProgram, m_pCamera);
			else
				m_weed->Render(&modelViewMatrixStack, pMainProgram, m_pCamera);
			modelViewMatrixStack.Pop();
		}

		if (pass == 1 && (m_cameraState == CCamera::States::FIRST_PERSON))
		{
			modelViewMatrixStack.Push();
			modelViewMatrixStack.SetIdentity();
			modelViewMatrixStack.Translate(m_width / 2, m_height - 70, 0);
			modelViewMatrixStack.Scale(glm::vec3(.22f, .09f, .22f));
			glDisable(GL_DEPTH_TEST);
			pMainProgram->SetUniform("lightingOn", false);
			pMainProgram->SetUniform("matrices.modelViewMatrix", modelViewMatrixStack.Top());
			pMainProgram->SetUniform("matrices.projMatrix",
				m_pCamera->GetOrthographicProjectionMatrix());
			m_pFBO->BindTexture(texture);
			m_quad->RenderNoTexture();
			modelViewMatrixStack.Pop();
		}
	}
}

// Update method runs repeatedly with the Render method
void Game::Update()
{
	// If 3 lap is done finish the game
	if (m_catmull->CurrentLap(distanceTraveled) == 3)
	{
		gameState = GameStates::FINISH;
		return;
	}

	// Change the camera based on the input
	if (GetKeyState(VK_NUMPAD1) & 0x80 || GetKeyState('1') & 0x80)
		m_cameraState = CCamera::States::THIRD_PERSON;
	if (GetKeyState(VK_NUMPAD2) & 0x80 || GetKeyState('2') & 0x80)
		m_cameraState = CCamera::States::FIRST_PERSON;
	if (GetKeyState(VK_NUMPAD3) & 0x80 || GetKeyState('3') & 0x80)
		m_cameraState = CCamera::States::TOP_DOWN;
	if (GetKeyState(VK_NUMPAD4) & 0x80 || GetKeyState('4') & 0x80)
		m_cameraState = CCamera::States::FRONT_VIEW;
	if (GetKeyState(VK_NUMPAD0) & 0x80 || GetKeyState('0') & 0x80)
		m_cameraState = CCamera::States::FREE;

	// Update timers
	m_timePassed += m_dt;
	m_playerTime += m_dt;

	// Car and Rocks collisions
	for (auto rockPosition : m_rocksList)
	{
		if (m_car->Intersects(rockPosition, 5))
		{
			m_car->OnAccident();
		}
	}
	// Car and time powerup collisions
	vector<int> timeRemovals;
	for (int i = 0; i < m_timePowerUpList.size(); i++)
	{
		if (m_car->Intersects(m_timePowerUpList[i], 5))
		{
			timeRemovals.push_back(i);
			m_playerTime -= 5000.f;
		}
	}
	// Car and turbo powerup collisions
	vector<int> turboRemovals;
	for (int i = 0; i < m_turboPowerUpList.size(); i++)
	{
		if (m_car->Intersects(m_turboPowerUpList[i], 5))
		{
			turboRemovals.push_back(i);
			m_car->activateTurbo();
		}
	}

	// Removal of the pickup objects
	for (auto index : timeRemovals)
		m_timePowerUpList.erase(m_timePowerUpList.begin() + index);
	for (auto index : turboRemovals)
		m_turboPowerUpList.erase(m_turboPowerUpList.begin() + index);

	// movement on the spline
	m_currentDistance += m_dt * m_car->GetSpeed();
	glm::vec3 p;
	glm::vec3 pNext;
	m_catmull->Sample(m_currentDistance, p);
	m_catmull->Sample(m_currentDistance + 1.f, pNext);

	glm::vec3 T = glm::normalize(pNext - p);
	glm::vec3 N = glm::normalize(glm::cross(T, Y_AXIS));
	glm::vec3 B = glm::normalize(glm::cross(N, T));

	m_spaceShipOrientation = glm::mat4(glm::mat3(T, B, N));


	// Update car position
	m_car->Update(m_dt);
	if (GetKeyState('D') & 0x80) {
		if (m_car->GetRotationAmountY() > -.45f)
			m_car->SetRotation(Y_AXIS, m_car->GetRotationAmountY() - .001f * m_dt);
		diversionFromCenter += .04f * m_dt;
		// Car and walls collisions
		if (diversionFromCenter > (PATH_WIDTH / 2 - 4.f))
			diversionFromCenter = PATH_WIDTH / 2 - 4.f;
	}
	else
	{
		if (m_car->GetRotationAmountY() < 0.f)
			m_car->SetRotation(Y_AXIS, m_car->GetRotationAmountY() + .003f * m_dt);
	}
	if (GetKeyState('A') & 0x80) {
		if (m_car->GetRotationAmountY() < .45f)
			m_car->SetRotation(Y_AXIS, m_car->GetRotationAmountY() + .001f * m_dt);
		diversionFromCenter -= .04f * m_dt;
		// Car and walls collisions
		if (diversionFromCenter < -(PATH_WIDTH / 2 - 4.f))
			diversionFromCenter = -(PATH_WIDTH / 2 - 4.f);
	}
	else
	{
		if (m_car->GetRotationAmountY() > 0.f)
			m_car->SetRotation(Y_AXIS, m_car->GetRotationAmountY() - .003f * m_dt);
	}
	if (GetKeyState('S') & 0x80) {
		m_car->Break(m_dt);
	}
	if (GetKeyState('W') & 0x80) {
		m_car->Accelerate(m_dt);
	}
	if (GetKeyState('T') & 0x80) {
		if (lightToggleTime < 0)
		{
			lightToggleTime = 700;
			isLightsOn = !isLightsOn;
		}
	}
	if (GetKeyState('N') & 0x80) {
		if (sunToggleTime < 0)
		{
			toggleOffset += M_PI;
			sunToggleTime = 700;
		}
	}
	lightToggleTime -= m_dt;
	sunToggleTime -= m_dt;
	forwardVec = T;
	rightVec = N;
	glm::vec3 newPosition = p + T + N * diversionFromCenter;
	distanceTraveled += glm::distance(newPosition, m_car->GetPosition());
	m_car->SetPosition(newPosition);
	m_car->SetOrientation(m_spaceShipOrientation);

	//m_pAudio->Update();
}

void Game::Render()
{
	m_pFBO->Bind();
	CCamera::States prev = m_cameraState;
	// If we are in free camera view don't capture
	if (prev != CCamera::States::FREE)
	{
		// Capture the scene with rear mirror view applied
		m_pCamera->SetCamera(CCamera::States::REAR_MIRROR, m_dt, forwardVec, rightVec, m_car);
		RenderScene(0);
	}

	// Capture the scene with MIRROR already rendere
	m_pFBO1->Bind();
	m_pCamera->SetCamera(prev, m_dt, forwardVec, rightVec, m_car);
	RenderScene(1);

	// Render scene with BLUR and MIRROR
	// Change to default buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	RenderScene(2);

	// Draw the 2D graphics after the 3D graphics
	if (gameState != GameStates::FINISH)
		DisplayFrameRate();
	// Swap buffers to show the rendered image
	SwapBuffers(m_gameWindow.Hdc());
}

void Game::AddLights(CShaderProgram* shader, glm::mat4 viewMatrix, glm::mat3 viewNormalMatrix)
{
	// Set light and materials in main shader program
	// Light [0] -> sun directional light
	float intensity = glm::cos(m_timePassed * 0.00005f + toggleOffset);
	float xzPlaneRotation = sin(m_timePassed * 0.00005f + toggleOffset);
	glm::vec4 lightPosition1 = glm::vec4(-100 * xzPlaneRotation, 100 * intensity, -100 * xzPlaneRotation, 1); // Position of light source *in world coordinates*
	light = lightPosition1;
	if (lightPosition1.y < 0.f)
	{
		isNight = true;
	}
	else
	{
		isNight = false;
	}
	shader->SetUniform("lights[0].position", viewMatrix * lightPosition1); // Position of light source *in eye coordinates*
	shader->SetUniform("lights[0].La", glm::vec3(glm::max(1.0f * intensity, 0.57f), glm::max(1.0f * intensity, 0.61f), glm::max(0.86f * intensity, 0.57f)));		// Ambient colour of light
	shader->SetUniform("lights[0].Ld", glm::vec3(glm::max(1.0f * intensity, 0.57f), glm::max(1.0f * intensity, 0.61f), glm::max(0.86f * intensity, 0.57f)));		// Diffuse colour of light
	shader->SetUniform("lights[0].Ls", glm::vec3(1.0f));		// Specular colour of light
	// This is for skybox and terrain
	shader->SetUniform("material1.Ma", glm::vec3(1.0f));	// Ambient material reflectance
	shader->SetUniform("material1.Md", glm::vec3(0.0f));	// Diffuse material reflectance
	shader->SetUniform("material1.Ms", glm::vec3(0.0f));	// Specular material reflectance
	shader->SetUniform("material1.shininess", 15.0f);		// Shininess material property

	if (isNight)
	{
		glm::vec4 moonLight = glm::vec4(lightPosition1.x, -lightPosition1.y, lightPosition1.z, 1);
		shader->SetUniform("lights[0].position", viewMatrix * moonLight);	// Position of light source *in eye coordinates*
		shader->SetUniform("lights[0].La", glm::vec3(glm::vec3(				// Ambient colour of light
			0.57f + intensity * (0.57f - .15f),
			0.61f + intensity * (0.61f - .21f),
			0.57f + intensity * (0.57f - .28f))));
		shader->SetUniform("lights[0].Ld", glm::vec3(glm::vec3(				// Diffuse colour of light
			0.57f + intensity * (0.57f - .15f),
			0.61f + intensity * (0.61f - .21f),
			0.57f + intensity * (0.57f - .28f))));
		shader->SetUniform("lights[0].Ls", glm::vec3(.15f, .21f, .28f));	// Specular colour of light
	}

	// light[1] -> right front car spotlight
	shader->SetUniform("lights[1].position", viewMatrix *
		glm::vec4(m_car->GetPosition() + 2.7f * forwardVec + 1.7f * rightVec + 2.5f * Y_AXIS, 1.f));
	shader->SetUniform("lights[1].La", glm::vec3(.01f, .01f, .01f));
	shader->SetUniform("lights[1].Ld", glm::vec3(.4f, .4f, .4f));
	shader->SetUniform("lights[1].Ls", glm::vec3(1.0f, 1.f, 1.f));
	shader->SetUniform("lights[1].direction", glm::normalize(viewNormalMatrix * forwardVec));
	shader->SetUniform("lights[1].exponent", .02f);
	shader->SetUniform("lights[1].cutoff", 20.0f);

	// light[2] -> left front car spotlight
	shader->SetUniform("lights[2].position", viewMatrix *
		glm::vec4(m_car->GetPosition() + 2.7f * forwardVec - 1.7f * rightVec + 2.5f * Y_AXIS, 1.f));
	shader->SetUniform("lights[2].La", glm::vec3(.01f, .01f, .01f));
	shader->SetUniform("lights[2].Ld", glm::vec3(.4f, .4f, .4f));
	shader->SetUniform("lights[2].Ls", glm::vec3(1.0f, 1.f, 1.f));
	shader->SetUniform("lights[2].direction", glm::normalize(viewNormalMatrix * forwardVec));
	shader->SetUniform("lights[2].exponent", .02f);
	shader->SetUniform("lights[2].cutoff", 20.0f);
	if (!isLightsOn)
	{
		shader->SetUniform("lights[1].exponent", .0f);
		shader->SetUniform("lights[1].La", glm::vec3(.0f, .0f, .0f));
		shader->SetUniform("lights[1].Ld", glm::vec3(.0f, .0f, .0f));
		shader->SetUniform("lights[1].Ls", glm::vec3(.0f, .0f, .0f));
		shader->SetUniform("lights[2].exponent", .0f);
		shader->SetUniform("lights[2].La", glm::vec3(.0f, .0f, .0f));
		shader->SetUniform("lights[2].Ld", glm::vec3(.0f, .0f, .0f));
		shader->SetUniform("lights[2].Ls", glm::vec3(.0f, .0f, .0f));
	}

	// light[3] -> right break car spotlight
	shader->SetUniform("lights[3].position", viewMatrix *
		glm::vec4(m_car->GetPosition() + -2.6f * forwardVec + 1.7f * rightVec + 2.8f * Y_AXIS, 1.f));
	shader->SetUniform("lights[3].La", glm::vec3(.0f, .0f, .0f));
	shader->SetUniform("lights[3].Ld", glm::vec3(.0f, .0f, .0f));
	shader->SetUniform("lights[3].Ls", glm::vec3(.0f, .0f, .0f));
	shader->SetUniform("lights[3].direction", glm::normalize(viewNormalMatrix * -forwardVec));
	shader->SetUniform("lights[3].exponent", .0f);
	shader->SetUniform("lights[3].cutoff", 37.0f);

	// light[4] -> left break car spotlight
	shader->SetUniform("lights[4].position", viewMatrix *
		glm::vec4(m_car->GetPosition() + -2.6f * forwardVec - 1.7f * rightVec + 2.8f * Y_AXIS, 1.f));
	shader->SetUniform("lights[4].La", glm::vec3(.0f, .0f, .0f));
	shader->SetUniform("lights[4].Ld", glm::vec3(.0f, .0f, .0f));
	shader->SetUniform("lights[4].Ls", glm::vec3(.0f, .0f, .0f));
	shader->SetUniform("lights[4].direction", glm::normalize(viewNormalMatrix * -forwardVec));
	shader->SetUniform("lights[4].exponent", .0f);
	shader->SetUniform("lights[4].cutoff", 37.0f);
	if (m_car->IsBreaking())
	{
		shader->SetUniform("lights[3].La", glm::vec3(.01f, .0f, .0f));
		shader->SetUniform("lights[3].Ld", glm::vec3(.1f, .0f, .0f));
		shader->SetUniform("lights[3].Ls", glm::vec3(0.8f, .0f, .0f));
		shader->SetUniform("lights[3].exponent", .005f);

		shader->SetUniform("lights[4].La", glm::vec3(.01f, .0f, .0f));
		shader->SetUniform("lights[4].Ld", glm::vec3(.1f, .0f, .0f));
		shader->SetUniform("lights[4].Ls", glm::vec3(0.8f, .0f, .0f));
		shader->SetUniform("lights[4].exponent", .005f);
	}
}

void Game::DisplayFrameRate()
{
	CShaderProgram* fontProgram = (*m_pShaderPrograms)[1];

	// Increase the elapsed time and frame counter
	m_elapsedTime += m_dt;
	m_frameCount++;

	// Now we want to subtract the current time by the last time that was stored
	// to see if the time elapsed has been over a second, which means we found our FPS.
	if (m_elapsedTime > 1000)
	{
		m_elapsedTime = 0;
		m_framesPerSecond = m_frameCount;

		// Reset the frames per second
		m_frameCount = 0;
	}

	if (m_framesPerSecond > 0) {
		// Use the font shader program and render the text
		fontProgram->UseProgram();
		glDisable(GL_DEPTH_TEST);
		fontProgram->SetUniform("matrices.modelViewMatrix", glm::mat4(1));
		fontProgram->SetUniform("matrices.projMatrix", m_pCamera->GetOrthographicProjectionMatrix());
		fontProgram->SetUniform("vColour", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
		m_pFtFont->Render(90, m_height - 35, 20, "LAP: %d/%d", m_catmull->CurrentLap(distanceTraveled), 3);
		m_pFtFont->Render(90, m_height - 92, 20, "Time: %d", (int)(m_playerTime / 1000));
		m_pFtFont->Render(20, m_height - 200, 14, "FPS: %d", m_framesPerSecond);
	}
}

// The game loop runs repeatedly until game over
void Game::GameLoop()
{
	/*
	// Fixed timer
	dDt = pHighResolutionTimer->Elapsed();
	if (dDt > 1000.0 / (double) Game::FPS) {
		pHighResolutionTimer->Start();
		Update();
		Render();
	}
	*/


	// Variable timer
	m_pHighResolutionTimer->Start();
	Update();
	Render();
	m_dt = m_pHighResolutionTimer->Elapsed();
}


WPARAM Game::Execute()
{
	m_pHighResolutionTimer = new CHighResolutionTimer;
	m_gameWindow.Init(m_hInstance);

	if (!m_gameWindow.Hdc()) {
		return 1;
	}

	Initialise();

	m_pHighResolutionTimer->Start();


	MSG msg;

	while (1) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else if (m_appActive) {
			GameLoop();
		}
		else Sleep(200); // Do not consume processor power if application isn't active
	}

	m_gameWindow.Deinit();

	return(msg.wParam);
}

LRESULT Game::ProcessEvents(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
	LRESULT result = 0;

	switch (message) {


	case WM_ACTIVATE:
	{
		switch (LOWORD(w_param))
		{
		case WA_ACTIVE:
		case WA_CLICKACTIVE:
			m_appActive = true;
			m_pHighResolutionTimer->Start();
			break;
		case WA_INACTIVE:
			m_appActive = false;
			break;
		}
		break;
	}

	case WM_SIZE:
		RECT dimensions;
		GetClientRect(window, &dimensions);
		m_gameWindow.SetDimensions(dimensions);
		break;

	case WM_PAINT:
		PAINTSTRUCT ps;
		BeginPaint(window, &ps);
		EndPaint(window, &ps);
		break;

	case WM_KEYDOWN:
		switch (w_param) {
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		case '1':
			m_pAudio->PlayEventSound();
			break;
		case VK_F1:
			m_pAudio->PlayEventSound();
			break;
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		result = DefWindowProc(window, message, w_param, l_param);
		break;
	}

	return result;
}

Game& Game::GetInstance()
{
	static Game instance;

	return instance;
}

void Game::SetHinstance(HINSTANCE hinstance)
{
	m_hInstance = hinstance;
}

LRESULT CALLBACK WinProc(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
	return Game::GetInstance().ProcessEvents(window, message, w_param, l_param);
}

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE, PSTR, int)
{
	Game& game = Game::GetInstance();
	game.SetHinstance(hinstance);

	return int(game.Execute());
}
