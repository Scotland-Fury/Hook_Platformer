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
#include "j1Scene2.h"
#include "j1Player.h"	// @Carles

j1Scene2::j1Scene2() : j1Module()
{
	name.create("scene2");
}

// Destructor
j1Scene2::~j1Scene2()
{}

// Called before render is available
bool j1Scene2::Awake(pugi::xml_node& config)
{
	LOG("Loading Scene");
	bool ret = true;

	cameraSpeed.x = config.child("cameraSpeed").attribute("x").as_float();
	cameraSpeed.y = config.child("cameraSpeed").attribute("y").as_float();

	map.create(config.child("map").attribute("name").as_string());

	return ret;
}

// Called before the first frame
bool j1Scene2::Start()
{
	if (App->scene->active == true) { active = false; }

	if (active)
	{
		App->map->Load(map.GetString());	// SamAlert: Hardcoded map loading, should use a p2SString that copies a string from an xml file
		App->audio->PlayMusic(App->audio->musicMap1.GetString());	// SamAlert: Add map condition for playing music, this always calls the map 1 music
		App->audio->SetMusicVolume();
	}

	return true;
}

// Called each loop iteration
bool j1Scene2::PreUpdate()
{
	return true;
}

// Called each loop iteration
bool j1Scene2::Update(float dt)
{
	AudioInput();

	if (App->player->debugMode == true)
		CameraInput();

	//App->render->Blit(img, 0, 0);
	App->map->Draw();

	return true;
}

// Called each loop iteration
bool j1Scene2::PostUpdate()
{
	bool ret = true;

	if (App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		ret = false;

	return ret;
}

// Called before quitting
bool j1Scene2::CleanUp()
{
	LOG("Freeing scene");

	return true;
}

void j1Scene2::CameraInput()	// @Carles
{
	if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_IDLE) {
		if (App->input->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT)
			App->render->camera.y += cameraSpeed.y;

		if (App->input->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT)
			App->render->camera.y -= cameraSpeed.y;

		if (App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT)
			App->render->camera.x += cameraSpeed.x;

		if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT)
			App->render->camera.x -= cameraSpeed.x;
	}
}

void j1Scene2::AudioInput()	// @Carles
{
	if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) {
		if (App->input->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT && App->audio->masterVolume < 100)
			App->audio->masterVolume++;

		if (App->input->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT && App->audio->masterVolume > 0)
			App->audio->masterVolume--;

		if (App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT && App->audio->musicVolume < 100)
			App->audio->musicVolume++;

		if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT && App->audio->musicVolume > 0)
			App->audio->musicVolume--;

		App->audio->SetMusicVolume();
		App->audio->SetSfxVolume();
	}
}

void j1Scene2::ChangeScene()
{
	App->scene2->active = true;
	App->scene->active = false;
	CleanUp();
	App->fade->FadeToBlack(App->scene, App->scene2);
	App->player->Start();
	App->render->camera = { 0,0 };
	App->scene2->Start();

}