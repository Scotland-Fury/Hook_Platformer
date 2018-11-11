#ifndef __j1SCENE_H__
#define __j1SCENE_H__

#include "j1Module.h"

struct SDL_Texture;

class j1Scene : public j1Module
{
public:

	j1Scene();

	// Destructor
	virtual ~j1Scene();

	// Called before render is available
	bool Awake(pugi::xml_node&);

	// Called before the first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each frame (logic)
	bool UpdateTick(float dt);

	// Called each loop iteration (graphic)
	bool Update();

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	// Changes to next scene
	void ChangeScene();


public:	// @Carles
	// Inputs
	void CameraInput(float dt);
	void AudioInput();
	bool active = true;

	fPoint playerPos;
private:
	fPoint cameraSpeed;	// @Carles
	

	p2SString map;
};

#endif // __j1SCENE_H__