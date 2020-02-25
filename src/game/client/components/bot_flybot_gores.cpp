//
// Created by Niklas on 25.02.2020.
//

#include "controls.h"

#include <math.h>

#include <engine/shared/config.h>

#include <game/collision.h>
#include <game/client/gameclient.h>

//variables
#define local_position m_pClient->m_LocalCharacterPos
#define local_id m_pClient->m_LocalClientID

void CControls::Flybot_Gores(){

    static int y = 0;

    if(!Config()->m_Flybot_Gores) {
        y = local_position.y;
        return;
    }

    m_InputData.m_TargetX = 0;
    m_InputData.m_TargetY = -200; // aim straight up

    //get distance to wall above us
    vec2 collpos = vec2(0, 0);
    Collision()->IntersectLine(local_position, vec2(local_position.x, local_position.y-m_pClient->m_Tuning.m_HookLength), &collpos, 0x0);
    int dist = distance(collpos, local_position);
    int time_hookfly = round_to_int(dist/m_pClient->m_Tuning.m_HookFireSpeed);
    time_hookfly = (time_hookfly > 1) ? time_hookfly : 1;

    CNetObj_Character local_char = m_pClient->m_Snap.m_aCharacters[local_id].m_Cur;
    Predict(&local_char, 100);

    int vel_0 = abs(m_pClient->m_aClients[local_id].m_Predicted.m_Vel.y)/m_pClient->m_Tuning.m_Gravity;

    if(m_PredPositionArray[vel_0].y > y)
        m_InputData.m_Hook = 1;
    else //if(m_PredPositionArray[vel_0].y < y)
        m_InputData.m_Hook = 0;





}