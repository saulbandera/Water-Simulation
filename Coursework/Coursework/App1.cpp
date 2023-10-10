#include "App1.h"
#include <fstream>
#include <sstream> 

App1::App1()
{
	water = nullptr;


	// Initialise shallow water simulation parameters

	spatialStepSize = 0.2f;  // 0.2f
	constexpr int GRID_SIZE = 660; // 100

	gridSizeX = GRID_SIZE; 
	stepSizeX = spatialStepSize;

	gridSizeY = GRID_SIZE; 
	stepSizeY = spatialStepSize;

	gravity = 9.8f;
	n = 0.9f; 
	timeStepSize = 0.001f; // 0.001  

	DTDXDY = timeStepSize / stepSizeX;

	firstPass = true;

}


///////////////////////////           [ INITIALISATION ]           ///////////////////////////                     
void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN)
{
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Initialise scene 3D models
	water = new Water(renderer->getDevice(), renderer->getDeviceContext(),hwnd, textureMgr, gridSizeY);


	// Initialise simulation grids
	predictedGrid = new SimulationGrid2D(gridSizeX, gridSizeY);
	correctedGrid = new SimulationGrid2D(gridSizeX, gridSizeY);

	// Initialise simulation render pass objects: 
	predictionShader = new PredictionShader(renderer->getDevice(), renderer->getDeviceContext(), hwnd, gridSizeX);
	correctionShader = new CorrectionShader(renderer->getDevice(), renderer->getDeviceContext(), hwnd, gridSizeX);

	predictionGridRTA = new RenderTexture(renderer->getDevice(), gridSizeX, gridSizeY, 0.1f, 100.0f);
	predictionGridRTB = new RenderTexture(renderer->getDevice(), gridSizeX, gridSizeY, 0.1f, 100.0f);
	correctionGridRTA = new RenderTexture(renderer->getDevice(), gridSizeX, gridSizeY, 0.1f, 100.0f);
	correctionGridRTB = new RenderTexture(renderer->getDevice(), gridSizeX, gridSizeY, 0.1f, 100.0f);

	orthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight, 0.0f, 0.0f);

	renderTargetsA[0] = predictionGridRTA->getRenderTargetView();
	renderTargetsA[1] = correctionGridRTA->getRenderTargetView();
	renderTargetsB[0] = predictionGridRTB->getRenderTargetView();
	renderTargetsB[1] = correctionGridRTB->getRenderTargetView();


	// Setup the viewport for simulation
	viewport.Width = (float)gridSizeX;
	viewport.Height = (float)gridSizeX;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	initLight();


	toggleGerstner = false;
	toggleSWE = false;


}
 



App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.
	if (water) {
		delete water;
	}

	if (light) {
		delete light;
	}

}

///////////////////////////           [ FRAME ]           /////////////////////////// 
bool App1::frame()
{

	bool result;
	time.frame();

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}

	// Render the graphics.
	result = render();

	if (!result)
	{
		return false;
	}


	return true;
}

void App1::initLight()
{
	int sceneWidth = 100;
	int sceneHeight = 100;

	// directional light definition 
	light = new Light();
	light->setAmbientColour(0.3f, 0.3f, 0.3f, 1.0f);
	light->setDiffuseColour(0.5f, 0.5f, 0.5f, 1.0f);
	light->setPosition(50.0f, 70.0f, 25.0f);
	light->setDirection(0.0f, -1.0f, -0.7f);
	light->setSpecularColour(0.3f, 0.3f, 0.4f, 1.0f);
	light->setSpecularPower(50.0f);
	light->generateOrthoMatrix((float)sceneWidth, (float)sceneHeight, 0.1f, 100.f);
}




void App1::trackFrameRate()
{
	static int sampleCount = 0; // Number of samples taken
	static float elapsedTime = 0; // Elapsed time since the last sample
	static float totalFPS = 0; // Total sum of FPS samples
	static float preTrackingTime = 0; // Time before starting to track FPS

	// Increment preTrackingTime until it reaches 2 seconds
	if (preTrackingTime < 5.0f) {
		preTrackingTime += time.getTime();
		return; // Exit the function if not enough pre-tracking time has passed
	}

	// Increment elapsed time
	elapsedTime += time.getTime();

	// Check if 10 seconds have passed
	if (elapsedTime >= 1.0f && timer->getFPS() > 0.0f)
	{
		// Get the current FPS
		float fps = timer->getFPS();

		// Increment the total FPS sum and sample count
		totalFPS += fps;
		sampleCount++;

		// Check if 5 samples have been taken
		if (sampleCount >= 5)
		{
			// Calculate the average FPS
			float averageFPS = totalFPS / sampleCount;

			// Convert float to string using a period as the decimal point
			std::stringstream ss;
			ss << averageFPS;
			std::string fpsString = ss.str();

			// Replace the period with a comma
			std::replace(fpsString.begin(), fpsString.end(), '.', ',');

			// Write the average FPS to the text file
			std::ofstream file("frameRateSamples.txt");
			if (file.is_open())
			{
				file << fpsString << "\n";
				file.close();
			}
			else
			{
				// Handle error in opening file
				//std::cerr << "Failed to open file for writing framerate samples.\n";
			}

			// Reset variables for next time
			sampleCount = 0;
			totalFPS = 0;
			elapsedTime = 0;

			// Optional: Pause or return
			system("PAUSE");
			//return;
		}
		else
		{
			// Reset elapsed time
			elapsedTime = 0;
		}
	}
}



///////////////////////////           [ RENDER ]           ///////////////////////////                     
bool App1::render()
{

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();

	camera->update();
	renderer->beginScene(0.5f, 0.5f, 0.5f, 1.0f);


	timeVar += time.getTime(); // Increase time counter for sine and Gerstner waves

	if (toggleSWE) {
		// Advance shallow water simulation
		simulationTime += time.getTime();
	}


	if (toggleSWE) {
		// Performing the shallow water simulation render passes

		if (simulationTime >= timeStepSize) {

			predictionStep(worldMatrix, orthoViewMatrix, orthoMatrix);
			correctionStep(worldMatrix, orthoViewMatrix, orthoMatrix);
			simulationTime -= timeStepSize;

			// The first render pass is used to pass the simulation grid values to the output render textures
			if (counter >= 1 && firstPass) {
				firstPass = false;
			}
		}
	}

	if (renderWater)
	{
		// Render the water mesh to be manipulated by either Gerstner waves, sine waves or shallow water simulation
		water->render(viewMatrix, worldMatrix, renderer->getProjectionMatrix(), light, camera->getPosition(), timeVar, correctionGridRTA->getShaderResourceView(), toggleGerstner, toggleSWE);
	}

	// Render GUI
	gui();

	if(toggleSWE)
	{
		counter++;
	}
	// Present the rendered scene to the screen.
	renderer->endScene();
	return true;
}



///////////////////////////           [ USER INTERFACE ]           ///////////////////////////                     
void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox(" Render water", &renderWater);
	ImGui::Checkbox(" Toggle Gerstner waves", &toggleGerstner);
	ImGui::Checkbox(" Toggle SWE water", &toggleSWE);


	// Water settings 
	water->GUI();

	// Custom ImGui theme
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 0.0f;
	style.FrameRounding = 4.0f;
	style.ScrollbarRounding = 4.0f;
	style.GrabRounding = 4.0f;
	style.Alpha = 0.9f;

	// Dark grey and black colors
	ImVec4 dark_grey(0.3f, 0.3f, 0.3f, 1.f);
	ImVec4 black(0.1f, 0.1f, 0.1f, 0.84f);

	// Collapsing header colors
	style.Colors[ImGuiCol_Header] = dark_grey;
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

	// Background, borders, and title colors
	style.Colors[ImGuiCol_WindowBg] = black;
	style.Colors[ImGuiCol_Border] = dark_grey;
	style.Colors[ImGuiCol_FrameBg] = dark_grey;
	style.Colors[ImGuiCol_TitleBg] = dark_grey;
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.0f);

	// Button colors
	style.Colors[ImGuiCol_Button] = dark_grey;
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

	// Slider colors
	style.Colors[ImGuiCol_FrameBg] = dark_grey;
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

	// Text color
	style.Colors[ImGuiCol_Text] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void App1::predictionStep(XMMATRIX world, XMMATRIX view, XMMATRIX proj)
{

	// Set the render targets A, used to store predicted and corrected grids. This
	// is to perform a ping ponged animation so that changes to the grids performed
	// on the GPU are maintained as the simulation advances.
	renderer->getDeviceContext()->OMSetRenderTargets(2, renderTargetsA, nullptr);
	renderer->getDeviceContext()->RSSetViewports(1, &viewport);
	// Clear the render targets that will store the grids
	predictionGridRTA->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 0.0f);
	correctionGridRTA->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 0.0f);

	orthoMesh->sendData(renderer->getDeviceContext());
	predictionShader->setSimulationParameters(gravity, n, timeStepSize, cr);

	// Send the render targets B containing the grids to the predictor step shader
	predictionShader->setShaderParameters(world, view, proj, DTDXDY, predictionGridRTB->getShaderResourceView(), correctionGridRTB->getShaderResourceView(), firstPass, predictedGrid, correctedGrid);
	predictionShader->render(renderer->getDeviceContext(), orthoMesh->GetIndexCount()); // Execute the prediction step of the simulation

	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();




}

void App1::correctionStep(XMMATRIX world, XMMATRIX view, XMMATRIX proj)
{

	// Set the render targets B, used to store predicted and corrected grids.
	renderer->getDeviceContext()->OMSetRenderTargets(2, renderTargetsB, nullptr);
	renderer->getDeviceContext()->RSSetViewports(1, &viewport);
	// Clear the render targets that will store the grids
	predictionGridRTB->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 0.0f);
	correctionGridRTB->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 0.0f, 0.0f);
	orthoMesh->sendData(renderer->getDeviceContext());
	correctionShader->setSimulationParameters(gravity, n, timeStepSize, cr);
	// Send the render targets A containing the grids to the corrector step shader
	correctionShader->setShaderParameters(world, view, proj, DTDXDY, predictionGridRTA->getShaderResourceView(), correctionGridRTA->getShaderResourceView(), firstPass, predictedGrid, correctedGrid);
	correctionShader->render(renderer->getDeviceContext(), orthoMesh->GetIndexCount()); // Execute the correction step of the simulation

	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}
