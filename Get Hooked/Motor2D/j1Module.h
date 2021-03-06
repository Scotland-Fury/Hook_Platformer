// ----------------------------------------------------
// j1Module.h
// Interface for all engine modules
// ----------------------------------------------------

#ifndef __j1MODULE_H__
#define __j1MODULE_H__

#include "p2SString.h"
#include "PugiXml\src\pugixml.hpp"

class j1App;

struct Collider;
enum class collision_type;

class j1Module
{
public:

	j1Module() : active(false)
	{}

	virtual void Init()
	{
		active = true;
	}

	// Called before render is available
	virtual bool Awake(pugi::xml_node&)
	{
		return true;
	}

	// Called before the first frame
	virtual bool Start()
	{
		return true;
	}

	// Called each loop iteration
	virtual bool PreUpdate()
	{
		return true;
	}

	// Called each frame (logic)
	virtual bool UpdateTick(float dt)
	{
		return true;
	}

	// Called each loop iteration (graphic)
	virtual bool Update()
	{
		return true;
	}

	// Called each loop iteration
	virtual bool PostUpdate()
	{
		return true;
	}

	// Called before quitting
	virtual bool CleanUp()
	{
		return true;
	}

	virtual bool Load(pugi::xml_node&)
	{
		return true;
	}

	virtual bool Save(pugi::xml_node&) const
	{
		return true;
	}

	virtual collision_type OnCollision(Collider*, Collider*) { return (collision_type)-1; }	// @Carles
	virtual void OnAir(bool airborne) {}	//CHANGE/FIX: We might need this for the enemies but who knows

public:

	p2SString	name;
	bool		active;

};

#endif // __j1MODULE_H__