#ifndef __j1COLLISION_H__
#define __j1COLLISION_H__	// @Carles

#include "j1Module.h"
#include "SDL\include\SDL_rect.h"

class PhysicalElement;

enum collider_type {	// @Carles, collider types
	COLLIDER_NONE = -1,
	COLLIDER_WALL,
	COLLIDER_PLATFORM,
	COLLIDER_ITEM,
	COLLIDER_PLAYER,
	COLLIDER_PLAYER_ATTACK,
	COLLIDER_ENEMY,
	COLLIDER_ENEMY_ATTACK,
	COLLIDER_WIN,

	COLLIDER_MAX
};

enum class collision_type {	//@Carles, enumerates collision types
	NONE = -1,
	UNDEFINED,
	ON_LEFT,
	ON_RIGHT,
	ON_TOP,
	ON_BOTTOM
};

struct Collider
{
	SDL_Rect rect;
	bool active = true;	//CHANGE/FIX: IMPLEMENT COLLIDER DETECTION RADIUS
	bool to_delete = false;
	collider_type type;
	PhysicalElement* callback = nullptr;

	Collider() :
		rect({ 0,0 }),
		type(collider_type::COLLIDER_NONE),
		callback(nullptr)
	{}

	Collider(SDL_Rect rectangle, collider_type type, PhysicalElement* callback = nullptr) :
		rect(rectangle),
		type(type),
		callback(callback)
	{}

	collider_type GetType() const { return type; }

	void SetPos(int x, int y)
	{
		rect.x = x;
		rect.y = y;
	}

	bool CheckCollision(const SDL_Rect& r) const;
};

class j1Collision : public j1Module
{
public:
	j1Collision();

	virtual ~j1Collision();

	// Called before render is available
	bool Awake(pugi::xml_node&);

	// Called each loop iteration
	bool PreUpdate();
	
	// Called each loop iteration (graphic)
	bool Draw();
	void DebugDraw();

	// Called before quitting
	bool CleanUp();

	// Save and Load
	bool Load(pugi::xml_document& map_file);
	//bool Save(pugi::xml_node&) const;

	Collider* AddCollider(SDL_Rect rect, collider_type type, PhysicalElement* callback);	//@Carles
	void DestroyCollider(Collider* collider);

public:	// @Carles
	bool CheckGroundCollision(Collider* hitbox) const;

public:
	bool mustDebugDraw;

private:
	bool matrix[COLLIDER_MAX][COLLIDER_MAX];
	p2List<Collider*> colliders;
	SDL_Rect screen;
};

#endif	//__j1COLLISION_H__