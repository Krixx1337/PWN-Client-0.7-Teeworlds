/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_PLAYERS_H
#define GAME_CLIENT_COMPONENTS_PLAYERS_H
#include <game/client/component.h>

class CPlayers : public CComponent
{
	CTeeRenderInfo m_aRenderInfo[MAX_CLIENTS];
	void RenderPlayer(
		const CNetObj_Character *pPrevChar,
		const CNetObj_Character *pPlayerChar,
		const CNetObj_PlayerInfo *pPrevInfo,
		const CNetObj_PlayerInfo *pPlayerInfo,
		int ClientID
	);
	void RenderHook(
		const CNetObj_Character *pPrevChar,
		const CNetObj_Character *pPlayerChar,
		const CNetObj_PlayerInfo *pPrevInfo,
		const CNetObj_PlayerInfo *pPlayerInfo,
		int ClientID
	);

public:
	virtual void OnRender();
    void drawline(vec2 p0, vec2 p1, float r, float g, float b);
    void drawbox(vec2 p0, float r, float g, float b);
};

#endif
