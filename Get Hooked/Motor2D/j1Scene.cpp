#include "Brofiler/Brofiler.h"
#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Input.h"
#include "j1Textures.h"
#include "j1Audio.h"
#include "j1Render.h"
#include "j1Window.h"
#include "j1Map.h"
#include "j1FadeScene.h"
#include "j1Scene.h"
#include "j1Collision.h"
#include "j1Timer.h"
#include "j1EntityManager.h"
#include "Player.h"
#include "Enemy.h"
#include "Item.h"
#include "j1Fonts.h"
#include "j1UserInterface.h"
#include "Image.h"
#include "Text.h"
#include "Button.h"
#include "ActionBox.h"

//Button actions	//CHANGE/FIX: Locate somewhere else, having this laying around is quite dirty, but putting them in a header creates wierd problems (Difficulty level: Rick didn't find the issue, and neither did I).
void StartGame()
{
	App->fade->FadeToBlack(App->fade->GetDelay(), fade_type::START_GAME);
}

void GoToSettings()
{
	App->fade->FadeToBlack(App->fade->GetDelay(), fade_type::SETTINGS);
}

void GoToMenu()
{
	App->win->scale = App->win->origScale;
	App->fade->FadeToBlack(App->fade->GetDelay(), fade_type::MAIN_MENU);
}

void GoToCredits()
{
	App->fade->FadeToBlack(App->fade->GetDelay(), fade_type::CREDITS);
}

void CloseGame()
{
	App->mustShutDown = true;
}

void SaveGame()
{
	if (App->scene->loadButton != nullptr) {
		App->scene->loadButton->Enable();
	}
	App->SaveGame();
}

void LoadGame()
{
	if (App->fade->GetStep() == fade_step::NONE)
		App->fade->FadeToBlack(App->fade->GetDelay(), fade_type::LOAD_GAME);
}

void SwitchValue(bool* value)	//IMPROVE: This should be the function that a CheckBox calls with it's own saved value as the parameter
{
	*value = !*value;
}

void OpenSettings()
{
	App->scene->gamePaused = true;

	if (App->scene->settingsWindow != nullptr)
		App->scene->settingsWindow->Activate();
}

void CloseSettings()
{
	App->scene->gamePaused = false;

	if (App->scene->settingsWindow != nullptr)
		App->scene->settingsWindow->Deactivate();
}

void OpenWebpage()
{
	ShellExecuteA(NULL, "open", "https://ch0m5.github.io/Get_Hooked/", NULL , NULL , SW_SHOWNORMAL);
}

// ------------------------------------------------------------------------------

//Constructor
j1Scene::j1Scene() : j1Module()
{
	name.create("scene");
	loading = true;

	button = new SDL_Rect[4];		//CHANGE/FIX: Could be improved, probably with a list of structs that have an id.
	checkButton = new SDL_Rect[3];
	exit = new SDL_Rect[4];
	shutDown = new SDL_Rect[4];
	settings = new SDL_Rect[4];
	back = new SDL_Rect[4];
	webpage = new SDL_Rect[4];
}

// Destructor
j1Scene::~j1Scene()
{
	RELEASE_ARRAY(button);
	RELEASE_ARRAY(checkButton);
	RELEASE_ARRAY(exit);
	RELEASE_ARRAY(shutDown);
	RELEASE_ARRAY(settings);
	RELEASE_ARRAY(back);
	RELEASE_ARRAY(webpage);
}

// Called before render is available
bool j1Scene::Awake(pugi::xml_node& config)
{
	LOG("Loading Scene");
	bool ret = true;

	cameraSpeed.x = config.child("cameraSpeed").attribute("x").as_float();
	cameraSpeed.y = config.child("cameraSpeed").attribute("y").as_float();

	debugMode = config.child("debugMode").attribute("value").as_bool();
	scene = (scene_type)config.child("scene").attribute("start").as_int();
	
	pugi::xml_node item;
	for (item = config.child("maps").first_child(); item != NULL; item = item.next_sibling()) {
		maps.add(item.attribute("file").as_string());
	}

	menuBackgroundTex.create(config.child("ui").child("menuBackground").child_value());	//CHANGE/FIX: Improvised, not well done

	//UI Data Awake
	item = config.child("ui").child("panel");
	panel = { item.attribute("x").as_int(), item.attribute("y").as_int(), item.attribute("w").as_int(), item.attribute("h").as_int() };
	
	item = config.child("ui").child("window");
	window = { item.attribute("x").as_int(), item.attribute("y").as_int(), item.attribute("w").as_int(), item.attribute("h").as_int() };
	
	RegisterButtonData(config.child("ui").child("button"), button);
	
	item = config.child("ui").child("checkButton");
	checkButton[0] = { item.attribute("x1").as_int(), item.attribute("y1").as_int(), item.attribute("w1").as_int(), item.attribute("h1").as_int() };
	checkButton[1] = { item.attribute("x2").as_int(), item.attribute("y2").as_int(), item.attribute("w2").as_int(), item.attribute("h2").as_int() };
	checkButton[2] = { item.attribute("x3").as_int(), item.attribute("y3").as_int(), item.attribute("w3").as_int(), item.attribute("h3").as_int() };

	RegisterButtonData(config.child("ui").child("exit"), exit);
	RegisterButtonData(config.child("ui").child("shutDown"), shutDown);
	RegisterButtonData(config.child("ui").child("settings"), settings);
	RegisterButtonData(config.child("ui").child("back"), back);
	RegisterButtonData(config.child("ui").child("webpage"), webpage);

	item = config.child("ui").child("healthBar");
	healthBar = { item.attribute("x").as_int(), item.attribute("y").as_int(), item.attribute("w").as_int(), item.attribute("h").as_int() };

	item = config.child("ui").child("healthChunck");
	healthChunck = { item.attribute("x").as_int(), item.attribute("y").as_int(), item.attribute("w").as_int(), item.attribute("h").as_int() };

	item = config.child("ui").child("sliderBar");
	sliderBar = { item.attribute("x").as_int(), item.attribute("y").as_int(), item.attribute("w").as_int(), item.attribute("h").as_int() };

	item = config.child("ui").child("sliderGrab");
	sliderGrab = { item.attribute("x").as_int(), item.attribute("y").as_int(), item.attribute("w").as_int(), item.attribute("h").as_int() };

	return ret;
}

void j1Scene::RegisterButtonData(pugi::xml_node& node, SDL_Rect* button)
{
	button[0] = { node.attribute("x1").as_int(), node.attribute("y1").as_int(), node.attribute("w1").as_int(), node.attribute("h1").as_int() };
	button[1] = { node.attribute("x2").as_int(), node.attribute("y2").as_int(), node.attribute("w2").as_int(), node.attribute("h2").as_int() };
	button[2] = { node.attribute("x3").as_int(), node.attribute("y3").as_int(), node.attribute("w3").as_int(), node.attribute("h3").as_int() };
	button[3] = { node.attribute("x4").as_int(), node.attribute("y4").as_int(), node.attribute("w4").as_int(), node.attribute("h4").as_int() };
}

// Called before the first frame
bool j1Scene::Start()
{
	bool ret = true;

	pugi::xml_document doc;
	App->LoadConfig(doc);
	pugi::xml_node config = doc.child("config");

	UIElement* parent;
	_TTF_Font* gameText = App->font->textFont;

	//IMPROVE: All the stuff "skipped" by cases, could be cleaner overall
	pugi::xml_document saveDoc;
	pugi::xml_parse_result result = saveDoc.load_file("save_game.xml");

	switch (scene) {	//CHANGE/FIX: All of these should be functions
	case scene_type::MAIN_MENU:	//CHANGE/FIX: Lots of magic numbers and hardcoding, but making it based on the screen size gives a lot of problems (TYPE/int)
		// iPoint screenCenter = { App->win->GetWindowSize().w / (2 * App->win->GetScale()),  App->win->GetWindowSize().h / (2 * App->win->GetScale()) };
		App->entityManager->player->active = false;
		
		backgroundTexPtr = App->tex->Load(menuBackgroundTex.GetString());
		App->ui->CreateImage({ 1024 / 4, 768 / 4 }, { 0, 0, 0, 0 }, backgroundTexPtr, false);

		App->ui->CreateText({ 1024 / 4, 100 }, "Get Hooked", DEFAULT_COLOR, App->font->titleFont, false);
		parent = App->ui->CreateActionBox(&StartGame, { 1024 / 4, 180 }, button, NULL, false);
		App->ui->CreateText(DEFAULT_POINT, "Start", DEFAULT_COLOR, gameText, false, parent);
		parent = App->ui->CreateActionBox(&LoadGame, { 1024 / 4, 225 }, button, NULL, false);
		App->ui->CreateText(DEFAULT_POINT, "Continue", DEFAULT_COLOR, gameText, false, parent);
		loadButton = (ActionBox<void>*)parent;
		parent = App->ui->CreateActionBox(&GoToSettings, { 1024 / 4, 270 }, button, NULL, false);
		App->ui->CreateText(DEFAULT_POINT, "Settings", DEFAULT_COLOR, gameText, false, parent);
		parent = App->ui->CreateActionBox(&GoToCredits, { 1024 / 4, 315 }, button, NULL, false);
		App->ui->CreateText(DEFAULT_POINT, "Credits", DEFAULT_COLOR, gameText, false, parent);
		App->ui->CreateActionBox(&CloseGame, { 20, 20 }, shutDown, NULL, false);
		App->ui->CreateActionBox(&OpenWebpage, { 55, 20 }, webpage, NULL, false);

		if (result == NULL) {
			loadButton->Disable();
		}
		
		App->audio->PlayMusic(App->audio->musicMainMenu.GetString());
		break;
	case scene_type::SETTINGS:
		App->entityManager->player->active = false;

		backgroundTexPtr = App->tex->Load(menuBackgroundTex.GetString());
		App->ui->CreateImage({ 1024 / 4, 768 / 4 }, { 0, 0, 0, 0 }, backgroundTexPtr, false);

		parent = App->ui->CreateImage({ 1024 / 4, 200 }, window, NULL, false);
		App->ui->CreateActionBox(&GoToMenu, { 353, 59 }, back, NULL, false, parent);
		App->ui->CreateText({ 1024 / 4, 58 }, "Settings", DEFAULT_COLOR, gameText, false, parent);
		App->ui->CreateActionBox(&CloseGame, { 20, 20 }, shutDown, NULL, false);

		CreateVolumeSliders();

		//App->ui->CreateImage({ 1024 / 4, 300 }, panel, NULL, false);

		break;
	case scene_type::CREDITS:
		App->entityManager->player->active = false;

		backgroundTexPtr = App->tex->Load(menuBackgroundTex.GetString());
		App->ui->CreateImage({ 1024 / 4, 768 / 4 }, { 0, 0, 0, 0 }, backgroundTexPtr, false);

		parent = App->ui->CreateImage({ 1024 / 4, 200 }, window, NULL, false);
		App->ui->CreateText({ 1024 / 4, 58 }, "Credits", DEFAULT_COLOR, gameText, false, parent);	//CHANGE/FIX: Implement line jumping, this is bad

		SetupCredits(parent);

		App->ui->CreateActionBox(&GoToMenu, { 353, 59 }, back, NULL, false, parent);
		App->ui->CreateActionBox(&CloseGame, { 20, 20 }, shutDown, NULL, false);
		break;
	case scene_type::LEVEL_1:
		App->map->Load(maps.At(0)->data.GetString());
		playerStart = { 608, 250 };		//start = App->map->data.checkpoints.start->data;	//CHANGE/FIX: Get points close to the ground
		playerFinish = { 500, 500 };	//finish = App->map->data.checkpoints.end->data;
		
		SetupLevel(result, config);

		App->audio->PlayMusic(App->audio->musicMap1.GetString());
		break;
	case scene_type::LEVEL_2:
		App->map->Load(maps.At(1)->data.GetString());
		playerStart = { 720, -400 };	//start = App->map->data.checkpoints.start->data;
		playerFinish = { 500, 500 };	//finish = App->map->data.checkpoints.end->data;
		
		SetupLevel(result, config);

		App->audio->PlayMusic(App->audio->musicMap2.GetString());
		break;
	/*case scene_type::LEVEL_3:
		App->map->Load(mapList.At(2)->data.name.GetString());
		break;
	case scene_type::LEVEL_4:
		break;
	case scene_type::LEVEL_5:
		break;*/
	default:
		ret = false;
	}

	App->audio->SetMusicVolume();

	return ret;
}

void j1Scene::SetupCredits(UIElement* window)	//CHANGE/FIX: Would be a lot better to have a block of text that cuts on set edges
{
	App->ui->CreateText({ 1024 / 4, 88 }, "MIT License", DEFAULT_COLOR, NULL, false);
	App->ui->CreateText({ 1024 / 4, 100 }, "Copyright (c) 2018 ch0m5", DEFAULT_COLOR, NULL, false);
	//App->ui->CreateText({ 1024 / 4, 112 }, "nothing", DEFAULT_COLOR, NULL, false, window);
	App->ui->CreateText({ 1024 / 4, 124 }, "The software is provided <as is>,", DEFAULT_COLOR, NULL, false);
	App->ui->CreateText({ 1024 / 4, 136 }, "without warranty of any kind,", DEFAULT_COLOR, NULL, false);
	App->ui->CreateText({ 1024 / 4, 148 }, "express or implied, including", DEFAULT_COLOR, NULL, false);
	App->ui->CreateText({ 1024 / 4, 160 }, "but not limited to the warranties", DEFAULT_COLOR, NULL, false);
	App->ui->CreateText({ 1024 / 4, 172 }, "of merchantability, fitness for a", DEFAULT_COLOR, NULL, false);
	App->ui->CreateText({ 1024 / 4, 184 }, "particular purpose and", DEFAULT_COLOR, NULL, false);
	App->ui->CreateText({ 1024 / 4, 196 }, "noninfringement in no event shall", DEFAULT_COLOR, NULL, false);
	App->ui->CreateText({ 1024 / 4, 208 }, "the authors or copyright holders be", DEFAULT_COLOR, NULL, false);
	App->ui->CreateText({ 1024 / 4, 220 }, "liable for any claim, damages or", DEFAULT_COLOR, NULL, false);
	App->ui->CreateText({ 1024 / 4, 232 }, "other liability, whether in an action", DEFAULT_COLOR, NULL, false);
	App->ui->CreateText({ 1024 / 4, 244 }, "of contract, tort ot otherwise,", DEFAULT_COLOR, NULL, false);
	App->ui->CreateText({ 1024 / 4, 256 }, "arising from, out of or in connection", DEFAULT_COLOR, NULL, false);
	App->ui->CreateText({ 1024 / 4, 268 }, "with the software or the use or", DEFAULT_COLOR, NULL, false);
	App->ui->CreateText({ 1024 / 4, 280 }, "other dealings in the software.", DEFAULT_COLOR, NULL, false);

	//App->ui->CreateText({ 1024 / 4, 292 }, "nothing", DEFAULT_COLOR, NULL, false, window);
	App->ui->CreateText({ 1024 / 4, 306 }, "Made by:", DEFAULT_COLOR, NULL, false);
	App->ui->CreateText({ 1024 / 4, 318 }, "Carles Homs Puchal", DEFAULT_COLOR, NULL, false);
	/*
	App->ui->CreateText({ 1024 / 4, 292 }, "software.", DEFAULT_COLOR, NULL, false, window);
	App->ui->CreateText({ 1024 / 4, 304 }, "an action of contract, tort or otherwise, arising from,", DEFAULT_COLOR, NULL, false, window);
	App->ui->CreateText({ 1024 / 4, 316 }, "out of or in connection with the software or the use or", DEFAULT_COLOR, NULL, false, window);
	App->ui->CreateText({ 1024 / 4, 316 }, "other dealings in the software.", DEFAULT_COLOR, NULL, false, window);*/
}

void j1Scene::CreateVolumeSliders()
{
	_TTF_Font* gameText = App->font->textFont;

	UIElement* parent;
	Image* barPtr;
	Image* grabPtr;

	parent = App->ui->CreateImage({ 1024 / 4, 100 }, panel, NULL, false);
	App->ui->CreateText(DEFAULT_POINT, "Master Volume", DEFAULT_COLOR, gameText, false, parent);
	parent = App->ui->CreateImage({ 1024 / 4, 135 }, sliderBar, NULL, false);
	masterSlider = App->ui->CreateImage({ 1024 / 4, 135 }, sliderGrab, NULL, true);
	barPtr = (Image*)parent;
	grabPtr = (Image*)masterSlider;
	grabPtr->SetSlider(barPtr->GetPosition().x, barPtr->GetPosition().x + barPtr->GetSize().x, App->audio->GetMasterVolume());

	parent = App->ui->CreateImage({ 1024 / 4, 170 }, panel, NULL, false);
	App->ui->CreateText(DEFAULT_POINT, "Music Volume", DEFAULT_COLOR, gameText, false, parent);
	parent = App->ui->CreateImage({ 1024 / 4, 205 }, sliderBar, NULL, false);
	musicSlider = App->ui->CreateImage({ 1024 / 4, 205 }, sliderGrab, NULL, true);
	barPtr = (Image*)parent;
	grabPtr = (Image*)musicSlider;
	grabPtr->SetSlider(barPtr->GetPosition().x, barPtr->GetPosition().x + barPtr->GetSize().x, App->audio->GetMusicVolume());

	parent = App->ui->CreateImage({ 1024 / 4, 240 }, panel, NULL, false);
	App->ui->CreateText(DEFAULT_POINT, "SFX Volume", DEFAULT_COLOR, gameText, false, parent);
	parent = App->ui->CreateImage({ 1024 / 4, 275 }, sliderBar, NULL, false);
	sfxSlider = App->ui->CreateImage({ 1024 / 4, 275 }, sliderGrab, NULL, true);
	barPtr = (Image*)parent;
	grabPtr = (Image*)sfxSlider;
	grabPtr->SetSlider(barPtr->GetPosition().x, barPtr->GetPosition().x + barPtr->GetSize().x, App->audio->GetSfxVolume());
}

void j1Scene::SetupLevel(pugi::xml_parse_result& result, pugi::xml_node& config)
{
	UIElement* parent;
	_TTF_Font* gameText = App->font->textFont;

	std::string str;
	p2SString p2Str;

	App->entityManager->player->SetSpawn(playerStart);
	App->entityManager->player->active = true;
	SpawnEntities(scene, config);

	App->ui->CreateImage({ 390, 35 }, healthBar, NULL, false);
	App->ui->CreateActionBox(&OpenSettings, { 20, 20 }, settings, NULL, false);

	str = std::to_string(App->entityManager->player->GetScore());
	p2Str.create("      %s", str.c_str());											//CHANGE/FIX: I should add some "reference point" that depends on parent, adding spaces is dirty
	parent = App->ui->CreateImage({ 150, 25 }, panel, NULL, false);
	App->ui->CreateText(DEFAULT_POINT, "Score       ", DEFAULT_COLOR, gameText, false, parent);
	score = App->ui->CreateText(DEFAULT_POINT, p2Str.GetString(), DEFAULT_COLOR, gameText, false, parent);

	str = std::to_string(App->entityManager->player->GetRetry());
	p2Str.create("      %s", str.c_str());
	parent = App->ui->CreateImage({ 150, 60 }, panel, NULL, false);
	App->ui->CreateText(DEFAULT_POINT, "Lifes x     ", DEFAULT_COLOR, gameText, false, parent);
	retry = App->ui->CreateText(DEFAULT_POINT, p2Str.GetString(), DEFAULT_COLOR, gameText, false, parent);
	
	parent = App->ui->CreateImage({ 150, 95 }, panel, NULL, false);
	timerText = App->ui->CreateText(DEFAULT_POINT, "0", DEFAULT_COLOR, gameText, false, parent);

	settingsWindow = App->ui->CreateImage({ 1024 / 4, 200 }, window, NULL, false);
	App->ui->CreateText({ 1024 / 4, 58 }, "Settings", DEFAULT_COLOR, gameText, false, settingsWindow);
	App->ui->CreateActionBox(&GoToMenu, { 153, 59 }, back, NULL, false, settingsWindow);
	App->ui->CreateActionBox(&CloseSettings, { 353, 59 }, exit, NULL, false, settingsWindow);
	App->ui->CreateActionBox(&SaveGame, { 1024 / 4, 100 }, button, NULL, false, settingsWindow);
	App->ui->CreateText({ 1024 / 4, 100 }, "Save", DEFAULT_COLOR, gameText, false, settingsWindow);
	loadButton = (ActionBox<void>*)App->ui->CreateActionBox(&LoadGame, { 1024 / 4, 140 }, button, NULL, false, settingsWindow);
	App->ui->CreateText({ 1024 / 4, 140 }, "Load", DEFAULT_COLOR, gameText, false, settingsWindow);

	//Volume widgets
	Image* barPtr;
	Image* grabPtr;

	parent = App->ui->CreateImage({ 1024 / 4, 180 }, panel, NULL, false, settingsWindow);
	App->ui->CreateText({ 1024 / 4, 180 }, "Master Volume", DEFAULT_COLOR, gameText, false, settingsWindow);
	parent = App->ui->CreateImage({ 1024 / 4, 205 }, sliderBar, NULL, false, settingsWindow);
	masterSlider = App->ui->CreateImage({ 1024 / 4, 205 }, sliderGrab, NULL, true, settingsWindow);
	barPtr = (Image*)parent;
	grabPtr = (Image*)masterSlider;
	grabPtr->SetSlider(barPtr->GetPosition().x, barPtr->GetPosition().x + barPtr->GetSize().x, App->audio->GetMasterVolume());

	parent = App->ui->CreateImage({ 1024 / 4, 235 }, panel, NULL, false, settingsWindow);
	App->ui->CreateText({ 1024 / 4, 235 }, "Music Volume", DEFAULT_COLOR, gameText, false, settingsWindow);
	parent = App->ui->CreateImage({ 1024 / 4, 255 }, sliderBar, NULL, false, settingsWindow);
	musicSlider = App->ui->CreateImage({ 1024 / 4, 255 }, sliderGrab, NULL, true, settingsWindow);
	barPtr = (Image*)parent;
	grabPtr = (Image*)musicSlider;
	grabPtr->SetSlider(barPtr->GetPosition().x, barPtr->GetPosition().x + barPtr->GetSize().x, App->audio->GetMusicVolume());

	parent = App->ui->CreateImage({ 1024 / 4, 285 }, panel, NULL, false, settingsWindow);
	App->ui->CreateText({ 1024 / 4, 285 }, "SFX Volume", DEFAULT_COLOR, gameText, false, settingsWindow);
	parent = App->ui->CreateImage({ 1024 / 4, 305 }, sliderBar, NULL, false, settingsWindow);
	sfxSlider = App->ui->CreateImage({ 1024 / 4, 305 }, sliderGrab, NULL, true, settingsWindow);
	barPtr = (Image*)parent;
	grabPtr = (Image*)sfxSlider;
	grabPtr->SetSlider(barPtr->GetPosition().x, barPtr->GetPosition().x + barPtr->GetSize().x, App->audio->GetSfxVolume());

	if (result == NULL) {
		loadButton->Disable();
	}

	settingsWindow->Deactivate();

	levelTimer.Start();
}

void j1Scene::HealthToUI(int life)
{
	int i = 0;
	if (i < life) {	//CHANGE/FIX: A loop iteration is obvious, yet "ilegal int to const TYPE" won't let me, debug and fix
		health[0] = (App->ui->CreateImage({ 340, 35 }, healthChunck, NULL, false));
		i++;
	}
	if (i < life) {
		health[1] = (App->ui->CreateImage({ 390, 35 }, healthChunck, NULL, false));
		i++;
	}
	if (i < life) {
		health[2] = (App->ui->CreateImage({ 440, 35 }, healthChunck, NULL, false));
		i++;
	}

	playerLife = i;
}

void j1Scene::HurtUI()
{
	App->ui->DestroyElement(health[playerLife - 1]);	//CHANGE/FIX: Really wierd implementation, but a list created problems
	health[playerLife - 1] = nullptr;
	playerLife--;
};

// Called each loop iteration
bool j1Scene::PreUpdate()	//IMPROVE: Full debug input here?
{
	BROFILER_CATEGORY("Module Scene PreUpdate", Profiler::Color::DarkOrange);

	if (App->fade->GetStep() == fade_step::NONE) {
		if (debugMode == true) {
			DebugInput();	// Check debug input
		}
		if (App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN && scene > scene_type::CREDITS) {
			if (settingsWindow->active == false) {
				App->scene->gamePaused = true;
				App->scene->settingsWindow->Activate();
			}
			else {
				App->scene->gamePaused = false;
				App->scene->settingsWindow->Deactivate();
			}
		}

		if (scene > scene_type::CREDITS || scene == scene_type::SETTINGS) {
			
			Image* sliderImg;
			if (masterSlider != nullptr) {
				sliderImg = (Image*)masterSlider;
				sliderImg->SliderToValue();
				sliderImg->LimitSlide();
			}
			if (musicSlider != nullptr) {
				sliderImg = (Image*)musicSlider;
				sliderImg->SliderToValue();
				sliderImg->LimitSlide();
			}
			if (sfxSlider != nullptr) {
				sliderImg = (Image*)sfxSlider;
				sliderImg->SliderToValue();
				sliderImg->LimitSlide();
			}
		}
	}
	if (scene > scene_type::CREDITS) {
		std::string str;
		p2SString p2Str;
		
		str = std::to_string((int)levelTimer.ReadSec());
		p2Str.create("%s", str.c_str());

		if (timerText != nullptr) {
			Text* timerPtr = (Text*)timerText;
			timerPtr->ChangeContent(p2Str.GetString());
		}
	}

	return true;
}

// Called each frame (logic)
bool j1Scene::UpdateTick(float dt)
{
	BROFILER_CATEGORY("Module Scene UpdateTick", Profiler::Color::OrangeRed);

	AudioInput();

	if (debugMode == true && scene > scene_type::CREDITS) {
		CameraInput(dt);
	}

	return true;
}

// Called each loop iteration (graphic)
bool j1Scene::Update()
{
	BROFILER_CATEGORY("Module Scene Update", Profiler::Color::Orange);

	if (App->entityManager->player != nullptr && App->entityManager->player->CameraFree() == false) {
		LimitCameraPos(App->entityManager->player->GetPosition());	// Limit camera position
	}

	// All blitting in order
	App->map->Draw();
	App->entityManager->Draw();
	App->collision->Draw();
	App->ui->Draw();

	return true;
}

// Called each loop iteration
bool j1Scene::PostUpdate()
{
	BROFILER_CATEGORY("Module Scene PostUpdate", Profiler::Color::DarkOrange);

	bool ret = true;
															// @Carles
	if (App->fade->GetStep() == fade_step::FULLY_FADED) {	// When game is fully faded, start game load and disable all entities for the next frame, then enable them.
		App->entityManager->active = false;					// This prevents the entities to use the massive dt value of the after-load frame on their calculations.
		App->ui->active = false;

		switch (App->fade->GetType()) {	//CHANGE/FIX: This should be a function
		case fade_type::MAIN_MENU:
			if (backgroundTexPtr != nullptr) {
				App->tex->UnLoad(backgroundTexPtr);
				backgroundTexPtr = nullptr;
			}
			App->ui->CleanUp();
			ChangeScene(scene_type::MAIN_MENU);
			break;
		case fade_type::SETTINGS:
			if (backgroundTexPtr != nullptr) {
				App->tex->UnLoad(backgroundTexPtr);
				backgroundTexPtr = nullptr;
			}
			App->ui->CleanUp();
			ChangeScene(scene_type::SETTINGS);
			break;
		case fade_type::CREDITS:
			if (backgroundTexPtr != nullptr) {
				App->tex->UnLoad(backgroundTexPtr);
				backgroundTexPtr = nullptr;
			}
			App->ui->CleanUp();
			ChangeScene(scene_type::CREDITS);
			break;
		case fade_type::START_GAME:
			if (backgroundTexPtr != nullptr) {
				App->tex->UnLoad(backgroundTexPtr);
				backgroundTexPtr = nullptr;
			}
			if (firstStart) {
				App->entityManager->player->retryLeft = 2;
				firstStart = false;
			}
			else {
				//App->entityManager->player->ResetRetry();
				App->entityManager->player->EraseScore();
			}
			App->ui->CleanUp();
			ChangeScene(scene_type::LEVEL_1);
			break;
		case fade_type::LOAD_GAME:
			if (backgroundTexPtr != nullptr) {
				App->tex->UnLoad(backgroundTexPtr);
				backgroundTexPtr = nullptr;
			}
			App->ui->CleanUp();
			App->LoadGame();
			break;
		case fade_type::NEXT_LEVEL:
			NextLevel();
			break;
		case fade_type::RESTART_LEVEL:
			App->entityManager->player->ResetScore();
			RestartLevel();
			break;
		case fade_type::RESTART_GAME:
			App->entityManager->player->ResetRetry();
			App->entityManager->player->EraseScore();
			ChangeScene(scene_type::LEVEL_1);
			break;
		case fade_type::LEVEL_1:
			scene = scene_type::LEVEL_1;
			CleanUp();
			App->entityManager->player->CleanUp();
			App->entityManager->CleanEntities();
			Start();
			App->entityManager->player->LoadStart();
			break;
		case fade_type::LEVEL_2:
			scene = scene_type::LEVEL_2;
			CleanUp();
			App->entityManager->player->CleanUp();
			App->entityManager->CleanEntities();
			Start();
			App->entityManager->player->LoadStart();
			break;
		default:
			break;
		}

		loading = true;
	}
	else if (loading) {
		App->entityManager->active = true;
		loading = false;
	}
	else if (App->fade->GetStep() == fade_step::UNFADING && App->fade->GetType() == fade_type::LOAD_GAME) {	//CHANGE/FIX: God knows why a wall collision resets the player Y pos. but this workaround fixes it for now.
		App->fade->ResetType();
		App->LoadGame();
	}

	if (App->ui->active == false && App->fade->GetStep() == fade_step::NONE) {	//CHANGE/FIX: Avoids bugs, but could be improved
		App->fade->ResetType();
		App->ui->active = true;
	}

	return ret;
}

// Called before quitting
bool j1Scene::CleanUp()	//IMPROVE: When changing scene a lot of new memory is allocated. Memory leaks?
{
	LOG("Freeing scene");
	App->map->CleanUp();
	App->collision->CleanUp();

	health[0] = health[1] = health[2] = nullptr;

	if (settingsWindow != nullptr) {
		App->ui->DestroyElement(settingsWindow);
		settingsWindow = nullptr;
	}

	/*if (score != nullptr) {
		App->ui->DestroyElement(score);
		score = nullptr;
	}*/
	timerText = nullptr;
	score = nullptr;
	retry = nullptr;
	loadButton = nullptr;

	masterSlider = nullptr;
	musicSlider = nullptr;
	sfxSlider = nullptr;

	return true;
}

// Save and Load
bool j1Scene::Load(pugi::xml_node& config)
{
	bool ret = true;

	scene_type current = scene;

	pugi::xml_node tmpNode;
	scene = (scene_type)config.child("scene").attribute("current").as_int();

	if (saveFix == false) {
		CleanUp();
		App->entityManager->player->CleanUp();
		App->entityManager->CleanEntities();
		Start();
		App->entityManager->player->LoadStart();
		saveFix = true;
	}
	else {
		saveFix = false;
	}

	return ret;
}

bool j1Scene::Save(pugi::xml_node& config) const
{
	bool ret = true;

	pugi::xml_node tmpNode;

	tmpNode = config.append_child("scene");
	tmpNode.append_attribute("current") = (int)scene;

	return ret;
}

void j1Scene::DebugInput()
{
	if (scene > scene_type::CREDITS) {
		if (App->fade->GetStep() == fade_step::NONE) {
			if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN) {
				App->fade->FadeToBlack(App->fade->GetDelay(), fade_type::RESTART_GAME);
			}

			if (App->input->GetKey(SDL_SCANCODE_F2) == KEY_DOWN) {
				App->fade->FadeToBlack(App->fade->GetDelay(), fade_type::RESTART_LEVEL);
			}

			if (App->input->GetKey(SDL_SCANCODE_F3) == KEY_DOWN) {
				App->fade->FadeToBlack(App->fade->GetDelay(), fade_type::NEXT_LEVEL);
			}

			if (App->input->GetKey(SDL_SCANCODE_P) == KEY_DOWN) {
				gamePaused = !gamePaused;
			}
		}

		if (App->input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN && App->entityManager->player->IsDead() == false) {	// Save game
			if (loadButton != nullptr) {
				loadButton->Enable();
			}
			App->SaveGame();
		}

		if (App->input->GetKey(SDL_SCANCODE_F6) == KEY_DOWN && App->entityManager->player->IsDead() == false) {	// Load game
			if (App->fade->GetStep() == fade_step::NONE)
				App->fade->FadeToBlack(App->fade->GetDelay(), fade_type::LOAD_GAME);
		}

		if (App->input->GetKey(SDL_SCANCODE_F8) == KEY_DOWN && App->ui->mustDebugDraw == false) {	// UI logic drawing
			App->ui->mustDebugDraw = true;
		}
		else if (App->input->GetKey(SDL_SCANCODE_F8) == KEY_DOWN && App->ui->mustDebugDraw == true) {
			App->ui->mustDebugDraw = false;
		}

		if (App->input->GetKey(SDL_SCANCODE_F9) == KEY_DOWN && App->collision->mustDebugDraw == false) {	// Logic drawing
			App->collision->mustDebugDraw = true;
		}
		else if (App->input->GetKey(SDL_SCANCODE_F9) == KEY_DOWN && App->collision->mustDebugDraw == true) {
			App->collision->mustDebugDraw = false;
		}

		// Change scale
		if (App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN) {
			App->win->scale = 1;
		}
		if (App->input->GetKey(SDL_SCANCODE_2) == KEY_DOWN) {
			App->win->scale = 2;
		}
		if (App->input->GetKey(SDL_SCANCODE_3) == KEY_DOWN) {
			App->win->scale = 3;
		}
		if (App->input->GetKey(SDL_SCANCODE_4) == KEY_DOWN) {
			App->win->scale = 4;
		}
		if (App->input->GetKey(SDL_SCANCODE_5) == KEY_DOWN) {
			App->win->scale = 5;
		}
	}
	else {
		if (App->input->GetKey(SDL_SCANCODE_F8) == KEY_DOWN && App->ui->mustDebugDraw == false) {	// UI logic drawing
			App->ui->mustDebugDraw = true;
		}
		else if (App->input->GetKey(SDL_SCANCODE_F8) == KEY_DOWN && App->ui->mustDebugDraw == true) {
			App->ui->mustDebugDraw = false;
		}
	}
}

void j1Scene::CameraInput(float dt)	// @Carles
{
	if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_IDLE) {
		if (App->input->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT)
			App->render->camera.y += (int)ceil(cameraSpeed.y * dt);

		if (App->input->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT)
			App->render->camera.y -= (int)ceil(cameraSpeed.y * dt);

		if (App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT)
			App->render->camera.x += (int)ceil(cameraSpeed.x * dt);

		if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT)
			App->render->camera.x -= (int)ceil(cameraSpeed.x * dt);
	}
}

void j1Scene::AudioInput()	// @Carles
{
	if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) {
		if (App->input->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT && App->audio->masterVolume < 100)
			App->audio->masterVolume++;

		if (App->input->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT && App->audio->masterVolume > 0)
			App->audio->masterVolume--;

		if (App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT && App->audio->musicVolume > 0)
			App->audio->musicVolume--;

		if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT && App->audio->musicVolume < 100)
			App->audio->musicVolume++;

		App->audio->SetMusicVolume();
		App->audio->SetSfxVolume();
	}
}

void j1Scene::NextLevel()
{
	scene = (scene_type)((int)scene + 1);

	if (scene == scene_type::MAX_SCENES) {	//Restart Game
		App->entityManager->player->ResetRetry();
		App->entityManager->player->EraseScore();
		ChangeScene(scene_type::MAIN_MENU);
	}
	else {
		ChangeScene(scene);
	}
}

void j1Scene::RestartLevel()	//IMPROVE: Only reload entities, not the full map
{
	CleanUp();
	App->entityManager->player->CleanUp();
	App->entityManager->CleanEntities();
	Start();
	App->entityManager->player->LifeToStart();	//CHANGE/FIX: A little bit buggy currently, let's just reset the HP to max for now (also it makes the game easier, which is appreciated)
	App->entityManager->player->Start();

	//OLD
	/*App->entityManager->player->DeadReset();
	App->entityManager->RestartEnemies();
	App->entityManager->player->CleanUp();
	App->entityManager->player->LifeToStart();
	App->entityManager->player->Start();*/
}

void j1Scene::ChangeScene(scene_type scene)
{
	this->scene = scene;

	CleanUp();
	App->entityManager->player->CleanUp();
	App->entityManager->CleanEntities();
	Start();
	App->entityManager->player->LifeToMax();
	App->entityManager->player->Start();
}

SDL_Rect j1Scene::LimitCameraPos(fPoint playerPos)
{
	if (App->render->camera.x < (int)-(playerPos.x * App->win->GetScale() - 350)/* && mapRightLimit is not crossed*/) {	//left	// Improve: Map limits & eliminate magic numbers
		App->render->camera.x = (int)-(playerPos.x * App->win->GetScale() - 350);
	}
	else if (App->render->camera.x > (int)-(playerPos.x * App->win->GetScale() - 500)/* && mapLeftLimit is not crossed*/) {	//right
		App->render->camera.x = (int)-(playerPos.x * App->win->GetScale() - 500);
	}

	if (App->render->camera.y < (int)-(playerPos.y * App->win->GetScale() - 300)/* && mapRightLimit is not crossed*/) {	//left
		App->render->camera.y = (int)-(playerPos.y * App->win->GetScale() - 300);
	}
	else if (App->render->camera.y > (int)-(playerPos.y * App->win->GetScale() - 400)/* && mapLeftLimit is not crossed*/) {	//right
		App->render->camera.y = (int)-(playerPos.y * App->win->GetScale() - 400);
	}

	return App->render->camera;
}

bool j1Scene::SpawnEntities(scene_type level, pugi::xml_node& config)
{
	bool ret = true;

	pugi::xml_node spawns = config.child("scene").child("maps");
	pugi::xml_node entities = config.child("entityManager").child("entities");

	switch (level)
	{
	case scene_type::LEVEL_1:
		spawns = spawns.child("level_1").child("spawns");
		break;
	case scene_type::LEVEL_2:
		spawns = spawns.child("level_2").child("spawns");
		break;
	default:
		return false;
	}

	SpawnLevelEntities(entities, spawns);

	return ret;
}

bool j1Scene::SpawnLevelEntities(pugi::xml_node& entitiesNode, pugi::xml_node& spawns)
{
	bool ret = true;

	pugi::xml_node lastChild;

	Enemy* enemyPtr;
	lastChild = spawns.child("enemiesEnd");
	for (pugi::xml_node spawnPoints = spawns.first_child(); spawnPoints != lastChild; spawnPoints = spawnPoints.next_sibling()) {
		enemyPtr = (Enemy*)App->entityManager->CreateEntity((entity_type)spawnPoints.attribute("type").as_int(), entitiesNode);
		enemyPtr->Spawn(spawnPoints.attribute("xSpawn").as_int(), spawnPoints.attribute("ySpawn").as_int());
	}

	Item* itemPtr;
	lastChild = spawns.child("itemsEnd");
	for (pugi::xml_node spawnPoints = spawns.child("enemiesEnd").next_sibling(); spawnPoints != lastChild; spawnPoints = spawnPoints.next_sibling()) {
		itemPtr = (Item*)App->entityManager->CreateEntity((entity_type)spawnPoints.attribute("type").as_int(), entitiesNode);
		itemPtr->Spawn(spawnPoints.attribute("xSpawn").as_int(), spawnPoints.attribute("ySpawn").as_int());
		itemPtr->UpdateHitbox();
	}


	return ret;
}