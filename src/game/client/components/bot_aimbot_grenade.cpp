//
// Created by Niklas on 12.02.2020.
//

#include "controls.h"

#include <math.h>

#include <engine/shared/config.h>

#include <game/collision.h>
#include <game/client/gameclient.h>
#include <cstdio>

#define lifetime m_pClient->m_Tuning.m_GrenadeLifetime              // 2.0f
#define curvature m_pClient->m_Tuning.m_GrenadeCurvature            // 7.0f
#define speed m_pClient->m_Tuning.m_GrenadeSpeed                    // 1000.0f

#define local_id m_pClient->m_LocalClientID                         // so we dont try to shoot for ourselves
#define local_position m_pClient->m_LocalCharacterPos

#define targetdist length(m_MousePos)                               // just to make sure the vec isnt too small to be used in game and mousepos saves us against detection

void CControls::GrenadeAimbot() {

    if((!Config()->m_GrenadeAimbot || m_pClient->m_Snap.m_aCharacters[local_id].m_Cur.m_Weapon != WEAPON_GRENADE))
        return;

    double angle = 0;
    vec2 player = GetPlayer(&angle);
    if(player == vec2(0, 0))
        return;
    vec2 shoot_at = normalize(GetDir(angle)) * targetdist;


    if(m_InputData.m_Fire&1){                       // i wish we could just go m_Fire += 2 but the zcatch server is vpn protected and i dont wanna expose my ip as a botter ip before 0.7 even really got started
        m_InputData.m_TargetX = shoot_at.x;
        m_InputData.m_TargetY = shoot_at.y;
    }

    if(Config()->m_GrenadeAutofire){
        m_InputData.m_TargetX = shoot_at.x;
        m_InputData.m_TargetY = shoot_at.y;
        m_InputData.m_Fire += 2;
    }

    if(Config()->m_HideBotting) {               // moves mousepos so we dont look so suspicious
        m_MousePos.x = m_InputData.m_TargetX;
        m_MousePos.y = m_InputData.m_TargetY;
    }

}

vec2 CControls::GetPlayer(double *RecAngle){ // now we add the Highangles to this function

    // first we want to get just any player to shoot at to get the calculation right, after that we think about predicting /*****DONE*****/

    // int dist = speed*lifetime; //nades wont fly much further unless they fall and accelerate but even then we most likely wont get the player info cause they will be too far away for us to even render
    // ^ Plus we dont want to distinguish players by body-to-body distance but by the angle we are aiming at, so it somewhat looks intended when we shoot players we couldnt even see before
    // we eliminate mouseflicking to a certain point and bypass possible detections

    double range = Config()->m_AimbotRange/2;                                       //we look for players within a cone as wide as AimbotRange tells us
    double mouse_angle = GetAngle(m_MousePos);
    vec2 final_player_position = vec2(0, 0);

    int iterations = MAX_CLIENTS+MAX_CLIENTS;
    for(int j = 0; j < iterations; j++) { // we want two loops: one for lowangles and one for high angles, we want to loop through all the possible low angles, since they are easier ones to actually hit
        int i = (j >= MAX_CLIENTS) ? (j-MAX_CLIENTS) : j;
        if (i == local_id || !m_pClient->m_Snap.m_aCharacters[i].m_Active || (m_pClient->m_aClients[i].m_Team == m_pClient->m_aClients[local_id].m_Team && m_pClient->m_GameInfo.m_GameFlags&GAMEFLAG_TEAMS)){
            continue;
        }

        /*
         * now how to we start predicting every player
         * we basically want to calculate an angle for every player and predict used player for the amount of time t the projectile flies
         *
         * we COULD just loop through every of the 100 positions and calculate the time t and compare it with the position inside the array
         * -> we NEED time t to be the same as how long we predict a player
         * ^ bad idea, can be done better but backup plan if everything else fails ( need to see the performance of looping so much )
         *
         * IDEA: we can recalculate the angle to shoot a player and predict for the time t over and over again until its the same
         * we get closer to the best angle with every loop and it shouldnt take more than 5-10 loops ( instead of up to 100 )
         *
         * EDIT: instead of checking for time t = predictiontime we can just check when the predicted position doesnt change anymore
         *       that's when we got the point we want to shoot at
         */

        CNetObj_Character player_char = m_pClient->m_Snap.m_aCharacters[i].m_Cur;   // we do this to fill the predictedpositionarray given in controls.h -> we will save performance on the long term
        Predict(&player_char, MAX_PREDICTION_TICKS);                                // we will always predict 100 ticks for each player for now

        vec2 player_position = m_PredPositionArray[0];                              // we just start calculating for the current player position
        int flight_time = 0;
        double aim_at_angle = 0.0;

        /**************************************************************************************************************/ // LOW ANGLES
        if(j < MAX_CLIENTS) {
            for (int x = 0; x < 50; x++) {                                               // iterations we go through to get a better prediction and 20 should be more than enough
                //while(last_flight_time != flight_time)                                    // <-somehow this crashes on me altho it should rarely ever go beyond 10 iterations, tell me if u find out why it crashes
                if (flight_time == -1)                                                  //code crashed when run because m_PredPositionArray doesnt have a -1st entry that flight_time can return
                    break;
                player_position = m_PredPositionArray[flight_time];

                aim_at_angle = CalculateAngleLow(player_position);
                flight_time = GetTimeOfFlight(aim_at_angle, player_position);
            }
        }
        /**************************************************************************************************************/ // HIGH ANGLES
        else if(j >= MAX_CLIENTS && final_player_position == vec2(0, 0)) {        //only loop when we havent found anyone to shoot at yet
            for (int x = 0; x < 50; x++) {                                               // iterations we go through to get a better prediction and 20 should be more than enough
                //while(last_flight_time != flight_time)                                    // <-somehow this crashes on me altho it should rarely ever go beyond 10 iterations, tell me if u find out why it crashes
                if (flight_time == -1)                                                  //code crashed when run because m_PredPositionArray doesnt have a -1st entry that flight_time can return
                    break;
                player_position = m_PredPositionArray[flight_time];

                aim_at_angle = CalculateAngleHigh(player_position);
                flight_time = GetTimeOfFlight(aim_at_angle, player_position);
            }
        }
        else
            continue;

        /* make sure we seek the desired player in relation to our current angle of aim */
        double angle_difference = pi - abs(abs(aim_at_angle - mouse_angle) - pi);  // formula to get the smaller angle between two

        //we use this if for both anglethingies
        if(angle_difference > range*(pi/180) || !CanReachDest(aim_at_angle, flight_time, player_position) || flight_time == -1) {
            continue;
        }

        range = angle_difference*(180/pi);

        final_player_position = player_position;
        *RecAngle = aim_at_angle;

    }

    if(final_player_position != vec2(0 ,0))
        return final_player_position;

    return vec2(0, 0);
}

int CControls::GetTimeOfFlight(double angle, vec2 destination){
    vec2 dir = normalize(GetDir(angle));
    vec2 start_projectile_position = m_pClient->m_LocalCharacterPos + dir*28.0f*0.75f;
    vec2 last_projectile_position = start_projectile_position;

    // we try to get the time the nades flying in the air
    for (float i = 0.0f; i < lifetime*SERVER_TICK_SPEED; i++) { // t so we know how long we have to check the trajectory // 100
        vec2 current_projectile_position = CalcPos(start_projectile_position, dir, curvature, speed, i/SERVER_TICK_SPEED);
        /*
         * to check how long a nade flew
         *
         * i/SERVER_TICK_SPEED is further used with *= speed so we get a range of 0.0 to 2.0 * 1000(=speed) -> 0.0ms to 2000.0ms which is 0 to 2 secs or 200 ticks (which is what we use)
         *
         * */

        vec2 closest_point = closest_point_on_line(current_projectile_position, last_projectile_position, destination);//This is a good way to ensure not getting -1 returned
        //Since we basically loop just 100 times we do not get every EXACT point, so we connect them
        last_projectile_position = current_projectile_position;

        if (distance(closest_point, destination) <= 14/*Half a tee*/)
            return i;
        /*
         *  okay i honestly dont know why we have to return i here, i thought we'd need 100*i/SERVER_TICK_SPEED
         *  but it predicts too far
         *  but apparently i is the right call, tell me why if u can xd
         *  we only need the array to be 100 entries big now
         * */
    }

    return -1;
}

bool CControls::CanReachDest(double angle, int t, vec2 destination){ //TODO: add playerevasion
//returns a time we can use to choose our target
    // Here we just loop through the trajectory our projectile and check if it will intersect with a wall or a teammate and if it can reach its target

    vec2 dir = normalize(GetDir(angle));
    vec2 start_projectile_position = m_pClient->m_LocalCharacterPos + dir*28.0f*0.75f;
    vec2 previous_projectile_position = start_projectile_position;

    for (float i = 0.0f; i < t; i++){ // t so we know how long we have to check the trajectory
        vec2 current_projectile_position = CalcPos(start_projectile_position, dir, curvature, speed, i/SERVER_TICK_SPEED);//SERVER_TICK_SPEED*lifetime gives us the actual time a grenade lives before it explodes
        if (Collision()->IntersectLine(previous_projectile_position, current_projectile_position, nullptr, nullptr))
            return false;
        previous_projectile_position = current_projectile_position;
    }

    return true;

}

double CControls::CalculateAngleLow(vec2 player_position) { // There are 2 angles we can get using the formula given below

    //Here we want to Calculate the angle we have to use to hit a target with our grenade projectile

    // the formula would be "angle = atan((pow(gspeed, 2) - /* + */ sqrt(pow(gspeed, 4) - gcurvature*(gcurvature * pow(pos.x, 2) + 2 * pos.y * pow(gspeed, 2))))/(gcurvature * pos.x));"
    // we just have to make it work, cuz teeworlds' angles goddamn suck
    // edit: copy Nobys GetAngle and GetDirection to gamecore.h and steal it ( ty noby )

    //The formula is right but the values dont match, even CalcPos shows that Curvature is divided by 10000 before being used, this cannot be applied here just like that

    /*
     *
     * for g;
     *
     * Look at this shit - gamecore.h calcpos()
     *
     * the function it uses to calculate the y position of a projectile is supposed to be -1/2g * t^2 + v0 * t + y0 where Curvature/10000 is used
     *
     * -1/2g is given as curvature/10000
     * solving the forumla gives g = -curvature/5000
     *
     * and for SOME reason it does "Time *= Speed;"
     *
     * since we go from t0 and want the initial speed we just use speed*speed
     *
     * source https://en.wikipedia.org/wiki/Projectile_motion
     *
     *
     * */

    player_position = player_position - local_position;

    float v = speed;
    float g = -curvature/5000 * (v*v);

    double angle = atan((pow(v, 2) -/* + */ sqrt(pow(v, 4) - g * (g * pow(player_position.x, 2) + 2 * player_position.y * pow(v, 2))))/(g * player_position.x));

    if(player_position.x < 0)
        angle+=pi;

    return angle;

}

double CControls::CalculateAngleHigh(vec2 player_position) {

    // we want to use low angles first, since they will be much easier to hit

    player_position = player_position - local_position;

    float v = speed;
    float g = -curvature/5000 * (v*v);

    double angle = atan((pow(v, 2) +/* - */ sqrt(pow(v, 4) - g * (g * pow(player_position.x, 2) + 2 * player_position.y * pow(v, 2))))/(g * player_position.x));

    if(player_position.x < 0)
        angle+=pi;

    return angle;

}
