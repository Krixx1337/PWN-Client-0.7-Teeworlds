//
// Created by Niklas on 26.02.2020.
//

#include "controls.h"

#include <engine/shared/config.h>

void CControls::Balance()
{
    if(!Config()->m_Balance)
        return;

    int LocalID = m_pClient->m_LocalClientID;
    vec2 Localpos = m_pClient->m_PredictedChar.m_Pos + m_pClient->m_PredictedChar.m_Vel;
    int ID = -1;
    int Range = 401;
    for(int i = 0; i < MAX_CLIENTS; i++)
    {
        if(i == LocalID)
            continue;
        vec2 Playerpos = m_pClient->m_aClients[i].m_Predicted.m_Pos;
        int Dist = distance(Playerpos, Localpos);
        if(!m_pClient->m_Snap.m_aCharacters[i].m_Active)
            continue;
        if(Localpos.y == Playerpos.y)
            continue;
        if(Dist < Range)
        {
            ID = i;
            Range = Dist;
        }
    }
    if(!m_pClient->m_pControls->m_InputData.m_Direction && ID != -1)
    {
        vec2 Playerpos = m_pClient->m_aClients[ID].m_Predicted.m_Pos;
        if(Localpos.y != Playerpos.y)
        {
            if(Localpos.x - Playerpos.x > 1)
                m_pClient->m_pControls->m_InputData.m_Direction = -1;
            if(Playerpos.x - Localpos.x > 1)
                m_pClient->m_pControls->m_InputData.m_Direction = 1;
        }
    }
}