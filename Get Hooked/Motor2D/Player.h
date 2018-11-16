#ifndef __PLAYER_H__
#define __PLAYER_H__	// @CarlesHoms

#include "Entity.h"

//Components
enum class player_state {	// @Carles, enum that groups all possible player states that will decide how the player behaves
	IDLE,
	CROUCHING,
	RUNNING,
	JUMPING,
	FALLING,
	SLIDING,
	HURT,
	//HOOK
};

class Player : public Entity
{
public:

	Player();

	virtual ~Player();

	// Called before render is available
	bool Awake(pugi::xml_node&);

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool PreUpdate();
	bool UpdateTick(float dt);
	bool Update();

	// Called before quitting
	bool CleanUp();

	// Called when colliding
	collision_type OnCollision(Collider* c1, Collider* c2);	// @Carles
	collision_type WallCollision(Collider* c1, Collider* c2);

	// Save and Load
	bool Load(pugi::xml_node&);
	bool Save(pugi::xml_node&) const;

public:
	bool CameraFree() const {
		return freeCamera;
	}

private:	// @Carles
	void ImportAllStates(pugi::xml_node&);													// Import all state data from config.xml
	void ImportAllSprites(pugi::xml_node&);													// Import all sprite data using the above function for each animation
	void AllocAllAnimations();																// Allocate all animations with previously recieved sprite data
	
	// Player actions
	void Jump();			// Add Y speed and jump state when requested
	void Fall(float dt);	// Add acceleration to Y speed
	void LateralStop();		// Stop X speed
	void Land();			// Stop Y speed
	void StandUp();			// Return to normal acceleration and reset slide values
	void Hurt();	// Stop and move slightly up and opposite of current direction, player state changes to HURT
	void PlayerReset();
	//void Hook();

	// Debug update
	void DebugInput();

	// Player update
	void CheckInput();		// Check player input
	//void CheckMovement();	// Check player current movement
	void CheckState();		// Check player state
	void ApplyState();		// Add state effects like movement restrictions, animation and sounds
	void Move(float dt);	// Move player position and decide/calculate other movement related factors
	void UpdateHitbox();	// Transform player collider depending on new position and state

	// Check possible new states in each state and other changes in the player's status
	player_state IdleMoveCheck();
	player_state CrouchingMoveCheck();
	player_state RunningMoveCheck();
	player_state JumpingMoveCheck();
	player_state FallingMoveCheck();
	player_state SlidingMoveCheck();
	player_state HurtMoveCheck();
	
	// Apply effects of each state
	void IdleEffects();
	void CrouchingEffects();
	void RunningEffects();
	void JumpingEffects();
	void FallingEffects();
	void SlidingEffects();
	void HurtEffects();
	void DeadEffects();

	// Movement and final changes
	fPoint GodModeMovement(float dt);
	fPoint NormalMovement(float dt);
	fPoint LimitSpeed();

public:
	bool debugMode;		// Flag that marks if debug functionalities are available

private:
	player_state status;

	fPoint lastGroundPosition;
	fPoint respawnPosition;

	fPoint hurtSpeed;	// Speed that the player adopts when getting hurt/killed

	float normalAcceleration;
	float slideAcceleration;
	float gravity;

	// Character status flags and directly related data
	bool airborne;			// Flag to mark if player is on air (not colliding with anything)
	bool somersaultUsed;	// Flag for somersault usage

	uint deadTimer = 0;		// Timer used for player death
	ushort deathDelay;		// Time delay between death and start FadeToBlack

	bool fading = false;	// Flag used to mark fade starting
	float fadeDelay;		// FadeToBlack duration

	bool mustReset = false;		// Flag used to restart animations when needed (skipping workflow steps)
	bool godMode;				// Flag used to mark player invencibility
	bool freeCamera;			// Flag used to mark free camera movement

	//Character sprites
	sprite_data idleSprite;

	sprite_data runSprite;
	sprite_data slideSprite;
	sprite_data crouchSprite;

	sprite_data jumpSprite;
	sprite_data somersaultSprite;
	sprite_data fallSprite;

	sprite_data hurtSprite;
	sprite_data deadSprite;

	// Audio
	uint runSfxTimer = 0;	// Timer to mark time between run sounds
	int runSfxDelay;		// Time between run sounds
	bool playedSlideSfx;	// Flag to mark slide sfx played
	bool playedHurtSfx;		// Flag to mark hurt sfx played
};

#endif //__PLAYER_H__

// All posible character animations for future use
	//Animation idle;
	//Animation idle_sword;

	//Animation draw_sword;
	//Animation sheathe_sword;

	//Animation run;
	//Animation slide;
	//Animation slide_up;
	//Animation crouch;

	//Animation jump;
	//Animation somersault;
	//Animation fall;

	//Animation attack_1;
	//Animation attack_2;
	//Animation attack_3;

	//Animation air_attack_1;
	//Animation air_attack_2;
	//Animation air_attack_3;
	//Animation air_attack_fall;
	//Animation air_attack_3_end;

	//Animation cast;
	//Animation cast_loop;

	//Animation use_item;

	//Animation corner_grab;
	//Animation corner_climb;
	//Animation ladder_climb;
	//Animation wall_slide;
	//Animation wall_jump;

	//Animation hurt;
	//Animation die;

// 2nd list for definitions
	//idle;
	//idle_sword;

	//draw_sword;
	//sheathe_sword;

	//run;
	//slide;
	//slide_up;
	//crouch;

	//jump;
	//somersault;
	//fall;

	//attack_1;
	//attack_2;
	//attack_3;

	//air_attack_1;
	//air_attack_2;
	//air_attack_3;
	//air_attack_3_fall;
	//air_attack_3_end;

	//cast;
	//cast_loop;

	//use_item;

	//corner_grab;
	//corner_climb;
	//ladder_climb;
	//wall_slide;
	//wall_jump;

	//hurt;
	//die;