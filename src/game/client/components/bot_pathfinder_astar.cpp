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

#define x_max Config()->m_X_MAX
#define y_max Config()->m_Y_MAX

using namespace std;

/*
 *
 *
 *
 *
 * */

void CControls::astar_pathfinder() {

    Config()->m_Pathfinder = 1;
    if(!Config()->m_Pathfinder)
        return;

    vec2 start = m_pClient->m_aClients[m_pClient->m_LocalClientID].m_Predicted.m_Pos;
    vec2 end = start + m_MousePos;

    find_path(start, end);

}

bool CControls::is_destination(int x, int y, vec2 destination) {
    if(abs(destination.x - x) < 32 && abs(destination.y - y) < 32)
        return true;
    return false;
}

bool CControls::is_valid(float x, float y, vec2 start){
    int index = Collision()->GetCollisionAt(x, y);
    if(index == TILE_AIR){
        if(x < start.x - x_max
        || x >= start.x + x_max
        || y < start.y - y_max
        || y >= start.y + y_max)
            return false;
        return true;
    }
    return false;
}

float CControls::heuristic(int x, int y, vec2 end) {
    //using manhatten distance here, since teeworlds is made of blocks
    float d = abs(end.x - x) + abs(end.y - y);
    return d;
}

void CControls::find_path(vec2 start, vec2 end){

    /// set start and end pos to each tile center so we do not run into funny things

    int xdif = (int)start.x/32;
    int ydif = (int)start.y/32;
    start = vec2(xdif*32 + 16, ydif*32 + 16);
    xdif = (int)end.x/32;
    ydif = (int)end.y/32;
    end = vec2(xdif*32 + 16, ydif*32 + 16);

    d_start = start;
    d_end = end;

    ///

    while(!end_path.empty()){
        end_path.pop_back();
    }

    //initialise our map

    float MAX_DIST = x_max + y_max;
    path_tile map[x_max][y_max];
    bool closed_list[x_max][y_max];

    int i = 0;
    for(int x = 0; x < x_max; x++){
        for(int y = 0; y < y_max; y++){
            map[x][y].f_cost = MAX_DIST;        //g + h
            map[x][y].h_cost = MAX_DIST;        //heuristic - cost to end
            map[x][y].g_cost = MAX_DIST;        //cost from start
            map[x][y].y_parent = -1;
            map[x][y].x_parent = -1;
            map[x][y].x_parent_pos = start.x - 32*x_max/2  +  32*x;
            map[x][y].y_parent_pos = start.y - 32*y_max/2  +  32*y;
            map[x][y].ax = x;
            map[x][y].ay = y;

            map[x][y].came_from = -1;

            map[x][y].x = start.x - 32*x_max/2  +  32*x;
            map[x][y].y = start.y - 32*y_max/2  +  32*y;

            closed_list[x][y] = false;
        }
    }

    // our [0][0] is techincally x_max/2 and y_max/2 so we start at vec2 start

    int x = x_max/2;
    int y = y_max/2;

    map[x][y].g_cost = 0;
    map[x][y].f_cost = 0;
    map[x][y].h_cost = 0;
    map[x][y].x_parent = -x_max/2  +  x;
    map[x][y].y_parent = -y_max/2  +  y;

    priority_queue<path_tile> open_set;
    open_set.push(map[x][y]);
    path_tile came_from[i];

    while(!open_set.empty()){

        path_tile current = open_set.top();

        if (is_destination(map[current.ax][current.ay].x, map[current.ax][current.ay].y, end)) {

            while(i>0) {
                current = came_from[i];
                end_path.push_back(current);
                printf("FOUND DESTINATION!\n");
                i--;
            }

            /// we wanna draw the path to be sure it works as intended, so we wanna have something we are able to pass to players.cpp to draw xd



            ///
            return;
        }

        open_set.pop();

        closed_list[current.ax][current.ay] = true;

        for(int nx = -1; nx <= 1; nx++){
            for(int ny = -1; ny <= 1; ny++){
                if(is_valid(current.x + nx*32, current.y + ny*32, start)) {
                    float d = (nx == ny) ? 32 * 1.141f : 32.0f;
                    float tentative_g_cost = current.g_cost + d;

                    if (tentative_g_cost < map[current.ax + nx][current.ay + ny].g_cost) {
                        came_from[i] = current;
                        i++;
                        map[current.ax + nx][current.ay + ny].g_cost = tentative_g_cost;
                        map[current.ax + nx][current.ay + ny].f_cost = map[current.ax + nx][current.ay + ny].g_cost + heuristic(current.x, current.y, end);
                        if (closed_list[current.ax + nx][current.ay + ny] == false) {
                            //map[current.ax + nx][current.ay + ny].x_parent = current.ax;
                            //map[current.ax + nx][current.ay + ny].y_parent = current.ay;
                            open_set.push(map[current.ax + nx][current.ay + ny]);
                        }
                    }
                }
            }
        }
    }
}

void CControls::draw_path(vector<path_tile> path) {

    for(vector<path_tile>::iterator it = path.begin(); it != path.end(); ++it){
        drawbox(vec2(it->x, it->y), 1, 0, 0);
    }

}








