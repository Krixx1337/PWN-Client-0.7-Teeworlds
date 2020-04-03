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

using namespace std;

#define STEP 32         //tilewidth
#define MAX_X 50
#define MAX_Y 50

void CControls::astar_pathfinder() {

    Config()->m_Pathfinder = 1;
    if(!Config()->m_Pathfinder)
        return;

    vec2 start = m_pClient->m_LocalCharacterPos;
    vec2 end = start + m_MousePos;

    find_path(start, end);

}

bool CControls::is_destination(float x, float y, vec2 destination) {
    if(x/32 == destination.x/32 && y/32 == destination.y/32)
        return true;
    return false;
}

bool CControls::is_valid(float x, float y, vec2 start){
    int tile = Collision()->GetPureMapIndex(x, y);
    tile = Collision()->GetTileIndex(tile);
    if(tile != TILE_SOLID && tile != TILE_NOHOOK && tile != TILE_FREEZE){
        if(x < start.x - 32*MAX_X/2 || x >= start.x + 32*MAX_X/2 || y < start.y - 32*MAX_Y/2 || y >= start.y + 32*MAX_Y/2)
            return false;
        return true;
    }
    return false;
}

float CControls::heuristic(float x, float y, vec2 end) {
    //using manhatten distance here, since teeworlds is made of blocks
    float d = abs(end.x/32 - x/32) + abs(end.y/32 - y/32);
    round_to_int(d);
    return d;
}

void CControls::find_path(vec2 start, vec2 end){

    /// set start and end pos to each tile center so we do not run into funny things

    static vec2 s_start = vec2(0,0);
    static vec2 s_end = vec2(0,0);

    int xdif = start.x/32;
    int ydif = start.y/32;
    start = vec2(xdif*32, ydif*32);
    //vec2 end = m_pClient->m_LocalCharacterPos;
    xdif = end.x/32;
    ydif = end.y/32;
    end = vec2(xdif*32, ydif*32);

    if(s_start == start && s_end == end)
        return;

    s_start = start;
    s_end = end;

    used_path.clear();

    //get tile mid
    end.x += 16; end.y += 16;
    start.x += 16; start.y += 16;

    d_start = start;
    d_end = end;

    ///

    if(!is_valid(end.x, end.y, start))
        return;

    if(!is_valid(start.x, start.y, start))
        return;

    if(is_destination(start.x, start.y, end)){
        return;
    }

    bool closedlist[MAX_X][MAX_Y];

    //initialize the map
    node map[MAX_X][MAX_Y];
    vec2 camefrom[MAX_X][MAX_Y];

    for(int x = 0; x < MAX_X; x++){
        for(int y = 0; y < MAX_Y; y++){
            map[x][y].fcost = 1000; //very high numbers yees
            map[x][y].gcost = 1000;
            map[x][y].hcost = 1000;
            map[x][y].x = start.x - 32*MAX_X/2 + 32*x;
            map[x][y].y = start.y - 32*MAX_Y/2 + 32*y;
            map[x][y].x_array = x;
            map[x][y].y_array = y;

            map[x][y].neighbours[0] = vec2(map[x][y].x_array-1, map[x][y].y_array);
            map[x][y].neighbours[1] = vec2(map[x][y].x_array+1, map[x][y].y_array);
            map[x][y].neighbours[2] = vec2(map[x][y].x_array, map[x][y].y_array-1);
            map[x][y].neighbours[3] = vec2(map[x][y].x_array, map[x][y].y_array+1);

            camefrom[x][y].x = -1;
            camefrom[x][y].y = -1;

            closedlist[x][y] = false;
        }
    }

    //initialize starting point

    int x = MAX_X/2; 
    int y = MAX_Y/2;
    map[x][y].fcost = 0.0f;
    map[x][y].gcost = 0.0f;
    map[x][y].hcost = 0.0f;


    priority_queue<node, vector<node>, greater<node>> openlist;
    openlist.push(map[x][y]);

    while(!openlist.empty()){
        node tile = openlist.top();
        openlist.pop();

        int x = tile.x_array;
        int y = tile.y_array;
        closedlist[x][y] = true;

        if(is_destination(tile.x, tile.y, end)){
            printf("Destination Found\n");
            ///we do stuff here later

            int lx = x, ly = y;
            while(camefrom[x][y].x != -1 && camefrom[x][y].y != -1){
                vec2 temp = vec2(map[x][y].x, map[x][y].y);
                used_path.emplace_back(temp);
                x = camefrom[x][y].x;
                y = camefrom[x][y].y;

                /* we do a workaround here, because for NO REASON, there are diagonal nodes in the final path
                    we are just going to find the node between them which is valid and hope there is one
                */

                if(x != lx && y != ly){
                    if(is_valid(map[lx][y].x, map[lx][y].y, start))
                        used_path.emplace_back(vec2(map[lx][y].x, map[lx][y].y));
                    else if(is_valid(map[x][ly].x, map[x][ly].y, start))
                        used_path.emplace_back(vec2(map[x][ly].x, map[x][ly].y));
                }

                lx = x; ly = y;
            }

            return;
        }

        ///here we loop through each neighbour and check which we can use
        int alength = 4;//we know its 4 so lets not fuck around with: sizeof(openlist[x][y].neighbours)/sizeof(vec2);
        for(int i = 0; i < alength; i++){

            int dx = map[x][y].neighbours[i].x;
            int dy = map[x][y].neighbours[i].y;

            if(dx < 0 || dx >= MAX_X || dy < 0 || dy >= MAX_Y)
                continue;

            if(!is_valid(map[dx][dy].x, map[dx][dy].y, start))
                continue;
            
            float gnew, hnew, fnew;
            gnew = tile.gcost + 1.0f;
            hnew = heuristic(map[dx][dy].x, map[dx][dy].y, end);
            fnew = gnew + hnew;
            if(map[dx][dy].fcost > fnew){
                map[dx][dy].gcost = gnew;
                map[dx][dy].hcost = hnew;
                map[dx][dy].fcost = fnew;
                camefrom[dx][dy] = vec2(x, y);

                if(closedlist[dx][dy] == false){
                    openlist.push(map[dx][dy]);
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









/*
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
    
    */