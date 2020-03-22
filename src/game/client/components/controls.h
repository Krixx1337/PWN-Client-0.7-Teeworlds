/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_CONTROLS_H
#define GAME_CLIENT_COMPONENTS_CONTROLS_H
#include <base/vmath.h>
#include <game/client/component.h>
#include <generated/protocol.h>
#include <game/collision.h>
#include <vector>

#define MAX_PREDICTION_TICKS 100

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
    //std::vector<struct path_tile> best_path;
    vec2 d_start;
    vec2 d_end;

    std::vector<vec2> used_path;

    int mapwidth;
    int mapheight;

    void astar_pathfinder();
    void find_path(vec2 pstart, vec2 pend);
    float heuristic(float x, float y, vec2 end);
    static bool is_destination(float x, float y, vec2 destination);
    bool is_valid(float x, float y, vec2 start);
    void drawpath(std::vector<vec2> path);

    //drawing

    void drawline(vec2 p0, vec2 p1, float r, float g, float b);
    void drawbox(vec2 p0, float r, float g, float b);
};

struct node{
    int x;
    int y;
    int parent_x;
    int parent_y;
    float gcost;
    float hcost;
    float fcost;
};

inline bool operator < (const node& lhs, const node& rhs){
    return lhs.fcost < rhs.fcost;
}

#endif
