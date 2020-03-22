//
// Created by Niklas on 27.02.2020.
//
#include "controls.h"

#include <engine/graphics.h>
#include <engine/shared/config.h>

#include <game/collision.h>
#include <game/client/render.h>
#include <game/client/components/players.h>

#include <stdio.h>

#include <queue>
#include <vector>
#include <stack>

#define STEP 32         //tilewidth
#define MAX_X 50
#define MAX_Y 50

void CControls::astar_pathfinder() {

    Config()->m_Pathfinder = 1;
    if(!Config()->m_Pathfinder)
        return;

    printf("START PATHFINDER with maxx = %d and maxy = %d\n", MAX_X, MAX_Y);
    vec2 start = m_pClient->m_LocalCharacterPos;
    vec2 end = start + m_MousePos;

    find_path(start, end);
    printf("END PATHFINDER \n");

}

bool CControls::is_destination(float x, float y, vec2 destination) {
    if(x == destination.x && y == destination.y)
        return true;
    return false;
}

bool CControls::is_valid(float x, float y, vec2 start){
    int tile = Collision()->GetCollisionAt(32*x, 32*y);
    if(tile == TILE_AIR){
        if(x < start.x - MAX_X/2 || x >= start.x + MAX_X/2 || y < start.y - MAX_Y/2 || y >= start.y + MAX_Y/2)
            return false;
        return true;
    }
    return false;
}

float CControls::heuristic(float x, float y, vec2 end) {
    //using manhatten distance here, since teeworlds is made of blocks
    float d = abs(end.x - x) + abs(end.y - y);
    return d;
}

void CControls::find_path(vec2 start, vec2 end){

    /// set start and end pos to each tile center so we do not run into funny things

    used_path.clear();

    int xdif = start.x/32;
    int ydif = start.y/32;
    start = vec2(xdif, ydif);
    //vec2 end = m_pClient->m_LocalCharacterPos;
    xdif = end.x/32;
    ydif = end.y/32;
    end = vec2(xdif, ydif);

    printf("Start x: %d, start y: %d\n", round_to_int(start.x), round_to_int(start.y));
    printf("End x: %d, end y: %d\n", round_to_int(end.x), round_to_int(end.y));

    d_start = start;
    d_end = end;

    ///

    if(!is_valid(end.x, end.y, start)){
        return;
    }

    if(is_destination(start.x, start.y, end)){
        return;
    }

    bool closedlist[MAX_X][MAX_Y];

    //initialize the map
    node allmap[MAX_X][MAX_Y];

    for(int x = 0; x < MAX_X; x++){
        for(int y = 0; y < MAX_Y; y++){
            allmap[x][y].fcost = 100000; //very high numbers yees
            allmap[x][y].gcost = 100000;
            allmap[x][y].hcost = 100000;
            allmap[x][y].parent_x = -1;
            allmap[x][y].parent_y = -1;
            allmap[x][y].x = start.x + x - MAX_X/2;
            allmap[x][y].y = start.y + y - MAX_Y/2;

            closedlist[x][y] = false;
        }
    }

    //initialize starting point

    int x = MAX_X/2;
    int y = MAX_Y/2;
    allmap[x][y].fcost = 0.0f;
    allmap[x][y].gcost = 0.0f;
    allmap[x][y].hcost = 0.0f;
    allmap[x][y].parent_x = x;
    allmap[x][y].parent_y = y;

    std::vector<node> openlist;
    openlist.emplace_back(allmap[x][y]);

    while(!openlist.empty()){
        node tile;
        do{
            float temp = 1000000;
            std::vector<node>::iterator itNode;
            for(std::vector<node>::iterator it = openlist.begin(); it != openlist.end(); it = next(it)){
                node n = *it;
                if(n.fcost <= temp){
                    temp = n.fcost;
                    itNode = it;
                }
            }
            tile = *itNode;
            openlist.erase(itNode);
        }while(is_valid(tile.x, tile.y, start) == false && !openlist.empty());

        x = tile.x - start.x + MAX_X/2;
        y = tile.y - start.y + MAX_Y/2;
        closedlist[x][y] = true;

        //for each neighbour starting from northwest to southeast
        for(int newx = -1; newx <= 1; newx++){
            for(int newy = -1; newy <= 1; newy++){
                float gnew, hnew, fnew;
                if(is_destination(allmap[x + newx][y + newy].x, allmap[x + newx][y + newy].y, end)){
                    allmap[x + newx][y + newy].parent_x = tile.x;
                    allmap[x + newx][y + newy].parent_y = tile.y;
                    printf("CHECKPOINT DESTINATION FOUND\n");
                    
                    ///MAKE PATH HERE

                    x = allmap[x + newx][y + newy].x - start.x + MAX_X/2;
                    y = allmap[x + newx][y + newy].y - start.y + MAX_Y/2;
                    std::stack<node> path;

                    while (!(allmap[x][y].parent_x == start.x && allmap[x][y].parent_y == start.y)
                        && x != -1 && y != -1) 
                    {
                        path.push(allmap[x][y]);
                        x = allmap[x][y].parent_x - start.x + MAX_X/2;
                        y = allmap[x][y].parent_y - start.y + MAX_Y/2;
                    }
                    path.push(allmap[x][y]);

                    while(!path.empty()){
                        node top = path.top();
                        used_path.emplace_back(vec2(top.x*32 + 16, top.y*32 + 16));
                        path.pop();
                    }
                    return;
                    ///
                }
                else if(closedlist[x + newx][y + newy] == false){
                    gnew = (newx == newy) ? tile.gcost + 1.141f : tile.gcost + 1.0f;
                    hnew = heuristic(allmap[x + newx][y + newy].x, allmap[x + newx][y + newy].y, end);
                    fnew = gnew + hnew;
                    //Check if new path is better than old one
                    if(allmap[x + newx][y + newy].fcost > fnew){
                        allmap[x + newx][y + newy].fcost = fnew;
                        allmap[x + newx][y + newy].gcost = gnew;
                        allmap[x + newx][y + newy].hcost = hnew;
                        allmap[x + newx][y + newy].parent_x = tile.x;
                        allmap[x + newx][y + newy].parent_y = tile.y;
                        openlist.emplace_back(allmap[x + newx][y + newy]);
                    }
                }
            }
        }
    }
}

void CControls::drawpath(std::vector<vec2> path){

    for(std::vector<vec2>::iterator it = path.begin(); it != path.end(); it = next(it)){
        vec2 pos = *it;
        drawbox(pos, 1, 0, 0);    
    }

}









