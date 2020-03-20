//
// Created by Niklas on 27.02.2020.
//
#include "controls.h"

#include <engine/graphics.h>
#include <engine/shared/config.h>

#include <game/collision.h>
#include <game/client/render.h>
#include <game/client/components/players.h>

#include <queue>
#include <vector>
#include <stack>

using namespace std;
#define STEP 32         //tilewidth
#define MAX_X mapwidth
#define MAX_Y mapheight

struct node{
    int x;
    int y;
    int parent_x;
    int parent_y;
    float gcost;
    float hcost;
    float fcost;
};

typedef struct node node;

inline bool operator < (const node& lhs, const node& rhs){
    return lhs.fcost < rhs.fcost;
}

void CControls::astar_pathfinder() {

    Config()->m_Pathfinder = 1;
    if(!Config()->m_Pathfinder)
        return;

    mapwidth = Collision()->GetWidth();
    mapheight = Collision()->GetHeight();

    printf("START PATHFINDER with maxx = %d and maxy = %d\n", MAX_X, MAX_Y);
    vec2 start = m_pClient->m_LocalCharacterPos;
    vec2 end = start + m_MousePos;

    find_path(start, end);
    printf("END PATHFINDER \n");

}

bool CControls::is_destination(float x, float y, vec2 destination) {
    if(x == destination.x, y == destination.y)
        return true;
    return false;
}

bool CControls::is_valid(float x, float y){
    int tile = Collision()->GetCollisionAt(x, y);
    if(tile == TILE_AIR){
        if(x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y)
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

    int xdif = start.x/32;
    int ydif = start.y/32;
    start = vec2(xdif, ydif);
    //vec2 end = m_pClient->m_LocalCharacterPos;
    xdif = end.x/32;
    ydif = end.y/32;
    end = vec2(xdif, ydif);

    d_start = start;
    d_end = end;

    ///

    printf("CHECKPOINT 0.0\n");

    vector<node> empty;
    if(!is_valid(end.x, end.y)){
        return;
    }

    printf("CHECKPOINT 0.25\n");

    if(is_destination(start.x, start.y, end)){
        return;
    }

    printf("CHECKPOINT 0.5\n");

    bool closedlist[MAX_X][MAX_Y];

    //initialize the map
    printf("CHECKPOINT 0.525\n");
    printf("INITIALIZE ARRAY with maxx = %d and maxy = %d\n", MAX_X, MAX_Y);

    /*node** allmap = new node*[MAX_Y];
    for(int i = 0; i < MAX_Y; i++){
        allmap[i] = new node[MAX_X];
    }*/

    printf("CHECKPOINT 0.55\n");

    int count = 0;
    for(int x = 0; x < MAX_X; x++){
        for(int y = 0; y < MAX_Y; y++){
            allmap[x][y].fcost = 100000; //very high numbers yees
            allmap[x][y].gcost = 100000;
            allmap[x][y].hcost = 100000;
            allmap[x][y].parent_x = -1;
            allmap[x][y].parent_y = -1;
            allmap[x][y].x = x;
            allmap[x][y].y = y;
            count++;

            closedlist[x][y] = false;
        }
    }
    printf("TOTAL LOOPS %d\n", count);

    printf("CHECKPOINT 0.6\n");
    //initialize starting point

    int x = start.x;
    int y = start.y;
    allmap[x][y].fcost = 0.0f;
    allmap[x][y].gcost = 0.0f;
    allmap[x][y].hcost = 0.0f;
    allmap[x][y].parent_x = x;
    allmap[x][y].parent_y = y;

    printf("CHECKPOINT 0.75\n");

    vector<node> openlist;
    openlist.emplace_back(allmap[x][y]);
    bool destinationfound = false;

    while(!openlist.empty()){
        node tile;
        do{
            float temp = 10000000;
            vector<node>::iterator itNode;
            for(vector<node>::iterator it = openlist.begin(); it != openlist.end(); it = next(it)){
                node n = *it;
                if(n.fcost < temp){
                    temp = n.fcost;
                    itNode = it;
                }
            }
            tile = *itNode;
            openlist.erase(itNode);
        }while(is_valid(tile.x, tile.y) == false);

        printf("CHECKPOINT 1\n");

        x = tile.x/STEP;
        y = tile.y/STEP;
        closedlist[x][y] = true;
        printf("CHECKPOINT 2\n");

        //for each neighbour starting from northwest to southeast
        for(int newx = -1; newx <= 1; newx++){
            for(int newy = -1; newy <= 1; newy++){
                float gnew, hnew, fnew;
                printf("CHECKPOINT 3\n");
                if(is_destination(tile.x + newx, tile.y + newy, end)){
                    allmap[x + newx][y + newy].parent_x = x;
                    allmap[x + newx][y + newy].parent_y = y;
                    destinationfound = true;
                    printf("CHECKPOINT DESTINATION FOUND\n");

                    ///MAKE PATH HERE
                }
                else if(closedlist[x + newx/STEP][y + newy/STEP] == false){
                    printf("CHECKPOINT 4\n");
                    gnew = (newx == newy) ? tile.gcost + 1.141f : tile.gcost + 1.0f;
                    hnew = heuristic(tile.x + newx, tile.y, end);
                    fnew = gnew + hnew;
                    //Check if new path is better than old one
                    if(allmap[x + newx][y + newy].gcost > gnew){
                        allmap[x + newx][y + newy].fcost = fnew;
                        allmap[x + newx][y + newy].gcost = gnew;
                        allmap[x + newx][y + newy].hcost = hnew;
                        allmap[x + newx][y + newy].parent_x = x;
                        allmap[x + newx][y + newy].parent_y = y;
                        openlist.emplace_back(allmap[x + newx][y + newy]);
                    }
                }
            }
        }
        if(destinationfound == false){
            return;
        }
    }




}









