/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_CONTROLS_H
#define GAME_CLIENT_COMPONENTS_CONTROLS_H
#include <base/vmath.h>
#include <game/client/component.h>
#include <generated/protocol.h>
#include <game/collision.h>

#define MAX_PREDICTION_TICKS 100
//#define T_WIDTH Collision()->GetWidth()
//#define T_HEIGHT Collision()->GetHeight()
#define MAX_PATH_VALUE 2048

class CControls : public CComponent
{
public:

	vec2 m_MousePos;
	vec2 m_TargetPos;

	CNetObj_PlayerInput m_InputData;
	CNetObj_PlayerInput m_LastData;
	int m_InputDirectionLeft;
	int m_InputDirectionRight;

	CControls();

	virtual void OnReset();
	virtual void OnRelease();
	virtual void OnRender();
	virtual void OnMessage(int MsgType, void *pRawMsg);
	virtual bool OnMouseMove(float x, float y);
	virtual void OnConsoleInit();
	virtual void OnPlayerDeath();

	int SnapInput(int *pData);
	void ClampMousePos();

	void Predict(CNetObj_Character *pCharacter, int t);
    /// we use these variables since they tell us how long a grenade "lives", we will unlikely ever gonna need more
	vec2 m_PredPositionArray[MAX_PREDICTION_TICKS]; // m_pClient->m_Tuning.m_GrenadeLifetime (2) + SERVER_TICK_SPEED (50)

	///bot_aimbot_grenade.cpp
    void GrenadeAimbot();
	vec2 GetPlayer(double *RecAngle);
	int GetTimeOfFlight(double angle, vec2 destination);
	double CalculateAngleLow(vec2 player_position);
    double CalculateAngleHigh(vec2 player_position);
    bool CanReachDest(double angle, int t, vec2 destination);

    ///bot_flybot_gores.cpp
    void Flybot_Gores();

    ///bot_balance.cpp
    void Balance();

    ///bot_pathfinder_astar.cpp
    void Pathfinder(vec2 pos1, vec2 pos2);
    vec2 final_path[MAX_PATH_VALUE];

    //a few functions so we know what we need to keep track of
    bool IsValid(); //need to know if start and dest are within our grid and no solid or freeze themselves (!= TILE_AIR - basically ( we dont have TILE_FREEZE on vanilla ))



};
#endif
