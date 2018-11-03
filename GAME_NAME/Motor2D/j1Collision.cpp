#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Input.h"
#include "j1Render.h"
#include "j1Collision.h"
#include "j1Player.h"

j1Collision::j1Collision()
{
	name.create("collision");
}

j1Collision::~j1Collision()
{}

// Called before render is available
bool j1Collision::Awake(pugi::xml_node& config)
{
	bool ret = true;

	pugi::xml_node childList = config.child("matrix").first_child();
	pugi::xml_attribute attributeList = childList.first_attribute();

	for (int i = COLLIDER_WALL; i < COLLIDER_MAX; ++i) {	// @Carles, automatically allocate collider matrix using config.xml
		for (int j = COLLIDER_WALL; j < COLLIDER_MAX; ++j) {

			matrix[i][j] = attributeList.as_bool();
			attributeList = attributeList.next_attribute();
		}

		childList = childList.next_sibling();
		attributeList = childList.first_attribute();
	}

	mustDebugDraw = config.child("mustDebugDraw").attribute("value").as_bool();

	return ret;
}

// Called each loop iteration
bool j1Collision::PreUpdate()
{
	bool ret = true;

	// Remove all colliders scheduled for deletion
	for (uint i = 0; i < colliders.count(); ++i) {
		if (colliders[i] != nullptr && colliders[i]->to_delete == true) {
			delete colliders[i];
			colliders[i] = nullptr;
		}
	}

	// Calculate collisions
	Collider* c1;
	Collider* c2;

	for (uint i = 0; i < colliders.count(); ++i) {
		// skip empty colliders
		if (colliders[i] == nullptr)
			continue;

		c1 = colliders[i];

		// avoid checking collisions already checked
		for (uint k = i + 1; k < colliders.count(); ++k) {
			// skip empty colliders
			if (colliders[k] == nullptr)
				continue;

			c2 = colliders[k];
			if (c2 != nullptr) {

				if (c1->CheckCollision(c2->rect) == true) {
					if (matrix[c1->type][c2->type] && c1->callback)
						c1->callback->OnCollision(c1, c2);

					if (matrix[c2->type][c1->type] && c2->callback)
						c2->callback->OnCollision(c2, c1);
				}
			}
		}
	}

	return ret;

	// CHECK_ERIC: PREVIOUS CODE
	/*
	bool ret = true;

	// Calculate collisions
	Collider* c1;
	Collider* c2;

	for (uint i = 0; i < colliders.count(); ++i)
	{
		// skip empty colliders
		if (colliders[i] == nullptr)
			continue;	

		c1 = colliders[i];

		// avoid checking collisions already checked
		for (uint k = 0; k < colliders.count(); ++k)
		{
			// skip empty colliders
			if (colliders[k] == nullptr)
				continue;

			c2 = colliders[k];

			if (c1->CheckCollision(c2->rect) == true)
			{
				if (matrix[c1->type][c2->type] && c1->callback)
					c1->callback->OnCollision(c1, c2);

				if (matrix[c2->type][c1->type] && c2->callback)
					c2->callback->OnCollision(c2, c1);
			}
		}
	}

	return ret;
	*/
}

bool j1Collision::Update(float dt)
{
	bool ret = true;

	DebugDraw();

	return ret;
}

// Called before quitting
bool j1Collision::CleanUp()
{
	LOG("Freeing all colliders");

	for (uint i = 0; i < colliders.count(); ++i)
	{
		if (colliders[i] != nullptr)
		{
			delete colliders[i];
			colliders[i] = nullptr;
		}
	}

	colliders.clear();	// @Carles, empty collider list of nullptr

	return true;
}

// Save and Load
bool j1Collision::Load(pugi::xml_document& map_file)
{
	LOG("Loading Colliders");
	bool ret = true;
	//Load all collider info

	pugi::xml_node collider;

	for (collider = map_file.child("map").child("objectgroup"); collider && ret; collider = collider.next_sibling("objectgroup"))
	{
		for (pugi::xml_node object = collider.child("object"); object && ret; object = object.next_sibling("object"))
		{
			Collider* col = new Collider();
			col->rect.x = object.attribute("x").as_uint();
			col->rect.y = object.attribute("y").as_uint();
			col->rect.w = object.attribute("width").as_uint();
			col->rect.h = object.attribute("height").as_uint();
			col->type = (COLLIDER_TYPE)object.attribute("name").as_uint();

			colliders.add(col);
		}
	}

	return ret;
}

//bool Save(pugi::xml_node&) const;

void j1Collision::DebugDraw()
{
	if (mustDebugDraw == false) {
		return;
	}

	Uint8 alpha = 80;
	Uint8 alphaHard = 130;
	for (uint i = 0; i < colliders.count(); ++i)
	{
		if (colliders[i] == nullptr)
			continue;

		/* COLORS
		Red: 255, 0, 0
		Green: 0, 255, 0
		Blue: 0, 0, 255
		Yellow: 255, 255, 0
		Magenta: 255, 0, 255
		Cyan: 0, 255, 255
		White: 255, 255, 255
		Black: 0, 0, 0
		Brown: 160, 128, 96
		*/

		switch (colliders[i]->type)
		{
		case COLLIDER_NONE: // white
			App->render->DrawQuad(colliders[i]->rect, 255, 255, 255, alpha);
			break;
		case COLLIDER_WALL: // green
			App->render->DrawQuad(colliders[i]->rect, 0, 255, 0, alpha);
			break;
		case COLLIDER_PLATFORM: // magenta
			App->render->DrawQuad(colliders[i]->rect, 255, 0, 255, alpha);	// SamAlert: Unreadable collider types
			break;
		case COLLIDER_FALLING_PLATFORM: // brown
			App->render->DrawQuad(colliders[i]->rect, 160, 128, 96, alpha);	// SamAlert: Unreadable collider types
			break;
		case COLLIDER_PLAYER: // blue
			App->render->DrawQuad(colliders[i]->rect, 0, 0, 255, alpha);
			break;
		case COLLIDER_PLAYER_ATTACK: // cyan
			App->render->DrawQuad(colliders[i]->rect, 0, 255, 255, alpha);
			break;
		case COLLIDER_ENEMY: // red
			App->render->DrawQuad(colliders[i]->rect, 255, 0, 0, alpha);
			break;
		case COLLIDER_ENEMY_ATTACK: // yellow
			App->render->DrawQuad(colliders[i]->rect, 255, 255, 0, alpha);
			break;
		}
	}
}

Collider* j1Collision::AddCollider(Collider* colliderPtr, SDL_Rect rect, COLLIDER_TYPE type, j1Module* callback)	//@Carles
{
	Collider* ret = nullptr;

	colliderPtr->rect = rect;
	colliderPtr->type = type;
	colliderPtr->callback = callback;
	colliders.add(colliderPtr);
	ret = colliderPtr;

	return ret;
}

// -----------------------------------------------------

bool Collider::CheckCollision(const SDL_Rect& r) const
{
	return !(rect.y + rect.h < r.y || rect.y > r.y + r.h || rect.x + rect.w < r.x || rect.x > r.x + r.w);
}

fPoint Collider::AvoidCollision(fPoint speed, Collider& collider) {

	fPoint new_speed = speed;
	Collider c1 = collider;
	c1.rect.x += speed.x;
	c1.rect.y += speed.y;

	//onGround = false;
	
	for (uint i = 0; i < App->collision->colliders.count(); ++i)
	{
		// skip empty colliders
		if (App->collision->colliders[i] == nullptr)
			continue;

		Collider* c2 = App->collision->colliders[i];

		if (c1.CheckCollision(c2->rect) == true)
		{
			if (c2->getType() == 0) {
				new_speed = CollisionSpeed(&c1.rect, &c2->rect, new_speed);
				if (speed.y == new_speed.y)
					App->player->state = player_state::FALLING;
			}
			c1.rect.y -= (speed.y - new_speed.y);
			c1.rect.x -= (speed.x - new_speed.x);
		}

	}

	//collider.callback->setGround(onGround,isFalling);

	return new_speed;
}

fPoint Collider::CollisionSpeed(SDL_Rect* collider1, SDL_Rect* collider2, fPoint new_speed) {
	SDL_Rect overlay;
	SDL_IntersectRect(collider1, collider2, &overlay);

	if (new_speed.y >= 0) {
		if (collider1->y + collider1->h >= collider2->y) {
			if (new_speed.x > 0) {
				if (overlay.h < overlay.w && collider1->y + collider1->h > collider2->y && collider1->y < collider2->y) {
					new_speed.y -= overlay.h;		//Ground
					App->player->state = player_state::IDLE;
				}
				else if (overlay.h < overlay.w && collider1->y < collider2->y + collider2->h) {
					new_speed.y += overlay.h;		//Corners
				}
				else if (overlay.w <= overlay.h && collider1->x < collider2->x + collider2->w && collider1->x > collider2->x) {
					new_speed.x += overlay.w;		//Corners
				}
				else if (overlay.w <= overlay.h && collider1->x + collider1->w > collider2->x) {
					new_speed.x -= overlay.w;		//Lateral Wall
				}
			}
			else if (new_speed.x < 0) {
				if (overlay.h < overlay.w && collider1->y + collider1->h > collider2->y && collider1->y < collider2->y) {
					new_speed.y -= overlay.h;		//Ground
					App->player->state = player_state::IDLE;
				}
				else if (overlay.h < overlay.w && collider1->y < collider2->y + collider2->h) {
					new_speed.y += overlay.h;		//Corners
				}
				else if (overlay.w <= overlay.h && collider1->x + collider1->w > collider2->x && collider1->x < collider2->x) {
					new_speed.x -= overlay.w;		//Corners
				}
				else if (overlay.w <= overlay.h && collider1->x < collider2->x + collider2->w) {
					new_speed.x += overlay.w;		//Lateral Wall
				}
			}
			else {
				new_speed.y -= overlay.h;
				App->player->state = player_state::IDLE;
			}
		}
	}
	else if (new_speed.y < 0) {
		if (collider1->y < collider2->y + collider2->h) {
			if (new_speed.x > 0) {
				if (overlay.h < overlay.w && collider1->y + collider1->h > collider2->y && collider1->y < collider2->y) {
					new_speed.y -= overlay.h;		//Ground
				}
				else if (overlay.h < overlay.w && collider1->y < collider2->y + collider2->h) {
					new_speed.y += overlay.h;		//Corners
				}
				else if (overlay.w <= overlay.h && collider1->x < collider2->x + collider2->w && collider1->x > collider2->x) {
					new_speed.x += overlay.w;		//Corners
				}
				else if (overlay.w <= overlay.h && collider1->x + collider1->w > collider2->x) {
					new_speed.x -= overlay.w;		//Lateral Wall
				}
			}
			else if (new_speed.x < 0) {
				if (overlay.h < overlay.w && collider1->y + collider1->h > collider2->y && collider1->y < collider2->y) {
					new_speed.y -= overlay.h;		//Ground
				}
				else if (overlay.h < overlay.w && collider1->y < collider2->y + collider2->h) {
					new_speed.y += overlay.h;		//Corners
				}
				else if (overlay.w <= overlay.h && collider1->x + collider1->w > collider2->x && collider1->x < collider2->x) {
					new_speed.x -= overlay.w;		//Corners
				}
				else if (overlay.w <= overlay.h && collider1->x < collider2->x + collider2->w) {
					new_speed.x += overlay.w;		//Lateral Wall
				}
			}
			else {
				new_speed.y += overlay.h;
				App->player->state = player_state::FALLING;
			}
		}
	}

	return new_speed;
}