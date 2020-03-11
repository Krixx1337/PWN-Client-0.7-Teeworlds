//
// Created by Niklas on 25.02.2020.
//

#include "controls.h"

#include <math.h>

#include <engine/shared/config.h>

#include <game/collision.h>
#include <game/client/gameclient.h>
#include <engine/keys.h>
#include <engine/input.h>

//variables
#define local_id m_pClient->m_LocalClientID
#define local_position m_pClient->m_aClients[m_pClient->m_LocalClientID].m_Predicted.m_Pos
#define local_velocity m_pClient->m_aClients[m_pClient->m_LocalClientID].m_Predicted.m_Vel

void CControls::Flybot_Gores(){

    static int y = 0;
    static int x = 0;
    static bool has_hooked = false;

    if(!Config()->m_Flybot_Gores) {
        y = local_position.y;
        x = local_position.x;
        if(has_hooked){
            m_InputData.m_Hook = 0;
            has_hooked = false;
        }
        return;
    }

    has_hooked = true; // reset hookinput

    if(Input()->KeyIsPressed(KEY_D))
        x += 5;
    if(Input()->KeyIsPressed(KEY_A))
        x -= 5;
    if(Input()->KeyIsPressed(KEY_W))
        y -= 5;
    if(Input()->KeyIsPressed(KEY_S))
        y += 5;

    //search hookable
    /*
     * for(float i = 1.5*pi; i > -0.5*pi; i-=pi/180){
        float degree = 90 + i * 180/pi;
        if(degree > 45 && degree < 315)
            continue;
    */
    vec2 nonpredpos = vec2(m_pClient->m_Snap.m_aCharacters[local_id].m_Cur.m_X, m_pClient->m_Snap.m_aCharacters[local_id].m_Cur.m_Y);
    vec2 hookpos = vec2(0, 0);

    int range = m_pClient->m_Tuning.m_HookLength;
    int nx = (int)nonpredpos.x + range;
    int ny = (int)nonpredpos.y + range;
    int index = Collision()->GetPureMapIndex(nx, ny);
    for(int i = 0; i < index; i++){
        if(Collision()->GetTileIndex(i) != TILE_SOLID)
            continue;
        vec2 tilepos = Collision()->GetPos(i);
        if(tilepos.y > local_position.y)
            continue;
        if(distance(local_position, tilepos) > range)
            continue;
        float angle = GetAngle(tilepos - local_position);
        float degrees = 90 + angle*180/pi;
        if(degrees > 45 && degrees < 315)
            continue;
        if(Collision()->IntersectLine(local_position, tilepos, 0, 0) == TILE_NOHOOK)
            continue;
        range = distance(local_position, tilepos);

        hookpos = tilepos;
    }

    if(hookpos == vec2(0, 0)) {
        m_InputData.m_Hook = 0;
        return;
    }

    if(local_position.x + local_velocity.x - x > 1)
        m_InputData.m_Direction = -1;
    else if(x - local_position.x + local_velocity.x > 1)
        m_InputData.m_Direction = 1;

    float Priority = abs(local_position.y - y)/(0.5f*13.37f);
    if(Priority < 1.0f)
        Priority = 1.0f;

    float vmod = 0.0f;

    if(y < local_position.y)
        vmod = -0.55f*Priority;
    else if(y > local_position.y)
        vmod = 0.55f*Priority;
    else
        vmod = 0.0f;

    if(local_velocity.y > vmod)
    {
        m_pClient->m_pControls->m_InputData.m_TargetX = hookpos.x - local_position.x;
        m_pClient->m_pControls->m_InputData.m_TargetY = hookpos.y - local_position.y;
        m_pClient->m_pControls->m_InputData.m_Hook = 1;
    }
    else
        m_pClient->m_pControls->m_InputData.m_Hook = 0;





}