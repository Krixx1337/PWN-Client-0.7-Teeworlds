//
// Created by Niklas on 27.02.2020.
//
#include "controls.h"
#include <game/collision.h>

struct CPathTile
{
    vec2 pos;

    int d_start;        //distance from the starttile
    int d_end;          //distance to the end (without obstacles)
    int heuristic;      //educated guess which tile makes most sense

    vec2 neighbours[4];
};

void CControls::Pathfinder(vec2 pos1, vec2 pos2) {

    /*
     *
     *
     *
     * */
    return;
}