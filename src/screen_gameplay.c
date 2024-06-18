/**********************************************************************************************
*
*   raylib - Advance Game template
*
*   Gameplay Screen Functions Definitions (Init, Update, Draw, Unload)
*
*   Copyright (c) 2014-2022 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#include "raylib.h"
#include <stdlib.h>
#include "math.h"
#include "raymath.h"
#include "screens.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            100
#endif


#define MAX_STARS   100

//typedefs

typedef struct BackgroundStar
{
	Vector2 position;
	Color color;
	float radius;
} BackgroundStar;

typedef struct Particle
{
	Vector2 position;
	Vector2 speed;
	float radius;
	float rotation;
    float alpha;
	Color color;
} Particle;

//planet typedef
typedef struct Planet
{
	Vector2 position;
	float radius;
	Color color;
    float gravity;
} Planet;

//missile typedef
typedef struct Missile
{
	Vector2 position;
	Vector2 speed;
	float radius;
	float rotation;
	float alpha;
	Color color;
} Missile;

//----------------------------------------------------------------------------------
// Module Variables Definition (local)
//----------------------------------------------------------------------------------
static int framesCounter = 0;
static int finishScreen = 0;
static bool drawDebugStrings = true;

//Shader shader = { 0 };

//Player variables
static Vector2 position = { 0.0f, 0.0f };
static Vector2 PlayerTri[] = { {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f} };
static Vector2 speed = { 0.0f, 0.0f };
static float Acceleration = 1.0f;
static float Direction = 0.0f;
static float Radius = 20.0f;
static float RotationRate = 5.0f;
static int Health = 100;
static bool isDead = false;

static float MissileFireRate = 0.5f;
static float MissileCooldown = 0.0f;

//engine exhaust particles
static Particle particles[100];
//planet
static Planet planet;
static Planet moon;
static Vector2 moonSpeed = { 0.0f, 0.0f };
//projectiles
static Missile missiles[10];

static BackgroundStar stars[MAX_STARS];

//we define a very slow speed of light, in order to experience dilation
static float SpeedOfLight = 10;
static float TimeDilation = 1.0f;

static float LightSpeedPercentage = 0.0f;
static float LorentzFactor = 1.0f;

Camera2D camera = { 0 };
Image ImBlank = { 0 };
Texture2D StarFieldTexture = { 0 };
Texture2D NebulaTexture = { 0 };
Shader shader = { 0 };

float  PointA_X = 0.0f;
float  PointA_Y = 0.0f;
float  PointB_X = 0.0f;
float  PointB_Y = 0.0f;
bool isPointA = true;

static float time = 0.0f;
static int timeLoc = 0;
static int worldPosLoc = 0;
static int divisionsLoc = 0;
static Vector2 predictedPosition = { 0.0f, 0.0f };
//----------------------------------------------------------------------------------
// Gameplay Screen Functions Definition
//----------------------------------------------------------------------------------

Image GenImageWhiteNoiseAlphaBG(int width, int height, float factor)
{
    Color* pixels = (Color*)RL_MALLOC(width * height * sizeof(Color));
    Color AlphaColor = { 0.0f, 0.0f, 0.0f, 0.0f };

    for (int i = 0; i < width * height; i++)
    {
        if (GetRandomValue(0, 999) < (int)(factor * 100.0f)) pixels[i] = WHITE;
        else pixels[i] = AlphaColor;
    }

    Image image = {
        .data = pixels,
        .width = width,
        .height = height,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1
    };

    return image;
}


void CreateMewEngineParticle()
{
	for (int i = 0; i < 100; i++)
	{
		if (particles[i].alpha <= 0)
		{
			particles[i].position = (Vector2){ position.x - 20 * cosf(Direction), position.y - GetRandomValue(-10, 10) * sinf(Direction) };
			particles[i].speed = (Vector2){ speed.x, speed.y };
			particles[i].radius = 4.0f;
			particles[i].rotation = 0.0f;
			particles[i].alpha = 1.0f;
			particles[i].color = RED;
			break;
		}
	}
}

void PredictPlayerPositionInTime(float time)
{
	predictedPosition = position;
	predictedPosition.x += speed.x * time;
	predictedPosition.y += speed.y * time;
}

// Gameplay Screen Initialization logic
void InitGameplayScreen(void)
{
    ImBlank = GenImageWhiteNoiseAlphaBG(GetScreenWidth() * 4, GetScreenHeight() * 4, 0.01f);
    //TextureScaleMode(ImBlank, FILTER_BILINEAR);  // Scale texture to enable bilinear filtering
    StarFieldTexture = LoadTextureFromImage(ImBlank);  // Load blank texture to fill on shader
    UnloadImage(ImBlank);

    //ImBlank = GenImagePerlinNoise(GetScreenWidth() * 4, GetScreenHeight() * 4, 50, 50, 1.0f);
    //NebulaTexture = LoadTextureFromImage(ImBlank);  // Load blank texture to fill on shader
    //UnloadImage(ImBlank);

    char* WorkingDir = GetWorkingDirectory();
    //print working dir
    printf("Working Directory: %s\n", WorkingDir);

    // NOTE: Using GLSL 330 shader version, on OpenGL ES 2.0 use GLSL 100 shader version
    //shader = LoadShader(0, TextFormat("star_field.fs", GLSL_VERSION));


    //float time = 0.0f;
    //timeLoc = GetShaderLocation(shader, "uTime");
    //SetShaderValue(shader, timeLoc, &time, SHADER_UNIFORM_FLOAT);

    //divisionsLoc = GetShaderLocation(shader, "uDivisions");

    //worldPosLoc = GetShaderLocation(shader, "uWorldPos");


    camera.target = (Vector2){ position.x + 20.0f, position.y + 20.0f };
    camera.offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    
    // TODO: Initialize GAMEPLAY screen variables here!
    framesCounter = 0;
    finishScreen = 0;

    // Initialize player position
    position = (Vector2){ GetScreenWidth()/2 - 200 , GetScreenHeight()/2 - 200 };
    
    //init planet
    planet.position = (Vector2){ GetScreenWidth()/2, GetScreenHeight()/2 };
    planet.radius = 50.0f;
    planet.color = BLUE;
    planet.gravity = 0.5f;

    //init moon
    moon.position = (Vector2){ GetScreenWidth()/2 + 200, GetScreenHeight()/2 + 200 };
    moon.radius = 20.0f;
    moon.color = GRAY;
    moon.gravity = 0.1f;
    //calculate initial moon speed
    Vector2 direction = Vector2Subtract(planet.position, moon.position);
    direction = Vector2Normalize(direction);
    Vector2 tangent = (Vector2){-direction.y, direction.x};
    moonSpeed = Vector2Scale(tangent, 0.1f);

    int GridDistance = 100;

    int GridLines = 100;

    DisableCursor();    // Disable cursor for better gameplay experience

    //generate background stars
    for(int i = 0; i < 100; i++)
        {
        stars[i].position = (Vector2){(float)GetRandomValue(-6000, 13000), (float)GetRandomValue(-6000, 8000)};
			stars[i].color = WHITE;
			stars[i].radius = (float)GetRandomValue(1, 2);
		}
}

// Gameplay Screen Update logic
void UpdateGameplayScreen(void)
{
    // TODO: Update GAMEPLAY screen variables here!
    double deltaTime = GetFrameTime();
    double DilatedDeltaTime = deltaTime * LightSpeedPercentage;

    if (MissileCooldown > 0) MissileCooldown -= deltaTime;

    float currentZoom = camera.zoom;

    // Camera zoom controls
    camera.zoom += ((float)GetMouseWheelMove() * 0.05f);

    if (camera.zoom > 3.0f) camera.zoom = 3.0f;
    else if (camera.zoom < 0.1f) camera.zoom = 0.1f;

    if (!isDead)
    {
        //player movement
        if (IsKeyDown(KEY_W))
        {
            speed.x += Acceleration * cosf(Direction) * deltaTime;
            speed.y += Acceleration * sinf(Direction) * deltaTime;
            // create new particle and add to array
           CreateMewEngineParticle();
        }
        //if space, apply brakes, use deltatime to slow down
        if (IsKeyDown(KEY_SPACE))
		{
			speed.x -= speed.x * deltaTime;
			speed.y -= speed.y * deltaTime;
		}

		//if left shift, apply boost
		if (IsKeyDown(KEY_LEFT_SHIFT))
		{
			speed.x += Acceleration * 2 * cosf(Direction) * deltaTime;
			speed.y += Acceleration * 2 * sinf(Direction) * deltaTime;
			// create new particle and add to array
			CreateMewEngineParticle();
		}

		//if left control, apply reverse
		if (IsKeyDown(KEY_LEFT_CONTROL))
		{
			speed.x -= Acceleration * cosf(Direction) * deltaTime;
			speed.y -= Acceleration * sinf(Direction) * deltaTime;
			// create new particle and add to array
			CreateMewEngineParticle();
		}

		//if left control, apply reverse
		if (IsKeyDown(KEY_S))
		{
			speed.x -= Acceleration * cosf(Direction) * deltaTime;
			speed.y -= Acceleration * sinf(Direction) * deltaTime;
			// create new particle and add to array
			CreateMewEngineParticle();
		}

		//if left control, apply reverse
		if (IsKeyDown(KEY_S))
		{
			speed.x -= Acceleration * cosf(Direction) * deltaTime;
			speed.y -= Acceleration * sinf(Direction) * deltaTime;
			// create new particle and add to array
			CreateMewEngineParticle();
		}

		//if left control, apply reverse
		if (IsKeyDown(KEY_S))
		{
			speed.x -= Acceleration * cosf(Direction) * deltaTime;
			speed.y -= Acceleration * sinf(Direction) * deltaTime;
			// create new particle and add to array
			CreateMewEngineParticle();
		}

		//if left control, apply reverse
		if (IsKeyDown(KEY_S))
		{
			speed.x -= Acceleration * cosf(Direction) * deltaTime;
			speed.y -= Acceleration * sinf(Direction) * deltaTime;
			// create new particle and add to array
			CreateMewEngineParticle();
		}

		//if left control, apply reverse
		if (IsKeyDown(KEY_S))
		{
			speed.x -= Acceleration * cosf(Direction) * deltaTime;
			speed.y -= Acceleration * sinf(Direction) * deltaTime;
			// create new particle and add
		}

       

    }
   

    // update particles
    for(int i = 0; i < MAX_STARS; i++)
        {   
    
        	if(particles[i].alpha > 0)
			{
				//apply planet gravity
                Vector2 direction = Vector2Subtract(planet.position, particles[i].position);
                direction = Vector2Normalize(direction);
                Vector2 frameNormalizedSpeed = Vector2Scale(particles[i].speed, DilatedDeltaTime);
                particles[i].speed = Vector2Add(frameNormalizedSpeed, Vector2Scale(direction, planet.gravity));


                particles[i].position.x += particles[i].speed.x;
				particles[i].position.y += particles[i].speed.y;
				particles[i].alpha -= 0.01f;
                particles[i].radius += 0.1f;
			}
		}

    //for left right add angle and rotate tri
    if (IsKeyDown(KEY_A)) Direction -= RotationRate * deltaTime;
    if (IsKeyDown(KEY_D)) Direction += RotationRate * deltaTime;

    //add planet gravity influence to speed
    Vector2 direction = Vector2Subtract(planet.position, position);
    direction = Vector2Normalize(direction);

    //distance from planet
    float distance = Vector2Distance(planet.position, position);
    float distanceInverseSquared = 1.0f / (distance * distance);

    speed = Vector2Add(speed, Vector2Scale(direction, planet.gravity * distanceInverseSquared * deltaTime));

    time = (float)GetTime();
    SetShaderValue(shader, timeLoc, &time, SHADER_UNIFORM_FLOAT);


    SetShaderValue(shader, worldPosLoc, &position, SHADER_UNIFORM_VEC2);

    //update moon position
    Vector2 moonDirection = Vector2Subtract(planet.position, moon.position);
    moonDirection = Vector2Normalize(moonDirection);
    moonSpeed = Vector2Add(moonSpeed, Vector2Scale(moonDirection, moon.gravity * DilatedDeltaTime));
    moon.position = Vector2Add(moon.position, moonSpeed);

    position.x += speed.x;
    position.y += speed.y;

    camera.target = (Vector2){ position.x, position.y };

    //wrap around screen
    /*
    if (position.x > GetScreenWidth()) position.x = 0;
    if (position.x < 0) position.x = GetScreenWidth();
    if (position.y > GetScreenHeight()) position.y = 0;
    if (position.y < 0) position.y = GetScreenHeight();
    */
    // calculate the player triangle points surrounding the central position, using the radius and direction
    // point 0 should be aligned with direction

   
        PlayerTri[0].x = position.x + Radius * cosf(Direction);
        PlayerTri[0].y = position.y + Radius * sinf(Direction);
        PlayerTri[1].x = position.x + Radius * cosf(Direction + 2.35619f);
        PlayerTri[1].y = position.y + Radius * sinf(Direction + 2.35619f);
        PlayerTri[2].x = position.x + Radius * cosf(Direction - 2.35619f);
        PlayerTri[2].y = position.y + Radius * sinf(Direction - 2.35619f);

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && MissileCooldown <= 0)
        {
            // create new missile and add to array
            MissileCooldown = MissileFireRate;
            if (isPointA)
            {
                PointA_X = GetMouseX();
                PointA_Y = GetMouseY();
            }
            else
            {
                PointB_X = GetMouseX();
                PointB_Y = GetMouseY();
            }

            isPointA = !isPointA;

            for (int i = 0; i < 10; i++)
            {
                if (missiles[i].alpha <= 0)
                {
                    missiles[i].position = PlayerTri[0];
                    missiles[i].speed = (Vector2){ 10.0f * cosf(Direction), 10.0f * sinf(Direction) };
                    missiles[i].radius = 4.0f;
                    missiles[i].rotation = 0.0f;
                    missiles[i].alpha = 1.0f;
                    missiles[i].color = GREEN;
                    break;
                }
            }
        }



    // check if overlap with planet, if collided with planet, reverse speed
    if (CheckCollisionCircleRec(planet.position, planet.radius, (Rectangle){position.x, position.y, 20, 20}))
	    {
		    speed = Vector2Scale(speed, -1);
            Health -= 50;
	    }

    // check if missiles hit planet
    for (int i = 0; i < 10; i++)
	{
		if (CheckCollisionCircleRec(planet.position, planet.radius, (Rectangle){missiles[i].position.x, missiles[i].position.y, 4, 4}))
		{
			missiles[i].alpha = 0;
		}
	}

    // check if player is dead
    if (Health <= 0 && !isDead)
	{
		isDead = true;
        //finishScreen = 1;
        //draw particles for explosion
        for(int i = 0; i < 100; i++)
            {
            if(particles[i].alpha <= 0)
				{
					particles[i].position = (Vector2){position.x + GetRandomValue(-50,50), position.y + GetRandomValue(-50,50)};
					particles[i].speed = (Vector2){GetRandomValue(-1000, 1000), GetRandomValue(-1000, 1000)};
					particles[i].radius = 4.0f;
					particles[i].rotation = 0.0f;
					particles[i].alpha = 1.0f;
					particles[i].color = RED;
				}
			}
	}

    // Press enter or tap to change to ENDING screen
    if (IsKeyPressed(KEY_ENTER) || IsGestureDetected(GESTURE_TAP))
    {
        //finishScreen = 1;
        PlaySound(fxCoin);
    }

    //calculate time dilation
    LightSpeedPercentage = 1; 
    TimeDilation = 1;

    PredictPlayerPositionInTime(100.0f);
}

// Gameplay Screen Draw logic
void DrawGameplayScreen(void)
{
       // Disable our custom shader, return to default shader


    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);

    //BeginShaderMode(shader);    // Enable our custom shader for next shapes/textures drawings
   // EndShaderMode();


    BeginMode2D(camera);
    BeginBlendMode(BLEND_ALPHA);
   // DrawTexture(NebulaTexture, -2 * GetScreenWidth(), -2 * GetScreenHeight(), PURPLE);  // Drawing BLANK texture, all magic happens on shader
    //DrawTexture

    Vector2 offset = { 20 *  GetScreenWidth() / 2, 20 * GetScreenHeight() / 2 };
    Rectangle source = { 0, 0, GetScreenWidth() * 4, GetScreenHeight() * 4 };
    Rectangle Scalesource = { 0, 0, GetScreenWidth() * 20, GetScreenHeight() * 20 };

    DrawTexturePro(StarFieldTexture, source, Scalesource, offset, 0.0f, WHITE);
    EndBlendMode();

    //draw background grid
    for(int i = -100; i < 100; i++)
		{

        Color color = (i % 5 == 0) ? DARKGRAY : GRAY;
        color = i == 0 ? RED : color;

		DrawLine(i * 100, -10 * GetScreenHeight(), i * 100, 20 * GetScreenHeight(), color);
		DrawLine(- 10 * GetScreenWidth(), i * 100, 20 * GetScreenWidth(), i * 100, color);
		}

    Vector2 PointA = { PointA_X, PointA_Y };
    Vector2 PointB = { PointB_X, PointB_Y };
    DrawLineEx(PointA, PointB, 5, RED);

    //draw line between point a and point b
   // DrawLineEx({ PointA_X, PointA_Y }, { (PointB_X, PointB_Y) }, 5, RED);

    // TODO: Draw GAMEPLAY screen here!

    //BeginBlendMode(BLEND_ADDITIVE);
    //draw particles
    for (int i = 0; i < 100; i++)
    {
        if (particles[i].alpha > 0)
        {
            DrawCircleV(particles[i].position, particles[i].radius, Fade(particles[i].color, particles[i].alpha));
        }
    }
   // EndBlendMode();

    Vector2 pos = { 20, 10 };

    if(!isDead)
	{

       DrawTriangle(PlayerTri[0], PlayerTri[2], PlayerTri[1], WHITE);

       //draw diamont at predicted position
       DrawCircleV(predictedPosition, 10, RED);

	}


    //draw planet
    DrawCircleV(planet.position, planet.radius, planet.color);

    //draw moon
    DrawCircleV(moon.position, moon.radius, moon.color);

    //draw direction text
    //DrawText(TextFormat("Direction: %f", Direction), 20, 20, 20, RED);

    //draw cursor at mouse position
    DrawCircleLines(GetMouseX(), GetMouseY(), 10, RED);

    //draw missiles
    for (int i = 0; i < 10; i++)
    {
        if (missiles[i].alpha > 0)
        {
            DrawCircleV(missiles[i].position, missiles[i].radius, Fade(missiles[i].color, missiles[i].alpha));
            missiles[i].position.x += missiles[i].speed.x;
            missiles[i].position.y += missiles[i].speed.y;
            missiles[i].alpha -= 0.01f;
        }
    }
    EndMode2D();
    if (isDead)
    {
        DrawText("GAME OVER", 20, 10, 40, RED);
    }
    //draw healthbar
    DrawRectangle(20, 40, Health * 2, 20, GREEN);
  

    //Draw mouse grid coordinates
    DrawText(TextFormat("Mouse Grid Integer: %i, %i", GetMouseX() / 100, GetMouseY() / -100), 20, 80, 40, RED);
    //Draw mouse grid float coordinates
    DrawText(TextFormat("Mouse Grid Float: %f, %f", (float)GetMouseX() / 100, (float)GetMouseY() / -100), 20, 140, 40, RED);
    //calculate length of line in grid space and print it
    float length = sqrtf(powf((PointB_X - PointA_X), 2) + powf((PointB_Y - PointA_Y), 2));
    DrawText(TextFormat("Line Length: %f", length / 100), 20, 180, 40, RED);
    //draw position of player in grid space
    DrawText(TextFormat("Player Grid: %f, %f", position.x / 100, position.y / -100), 20, 220, 40, RED);


}

// Gameplay Screen Unload logic
void UnloadGameplayScreen(void)
{
    // TODO: Unload GAMEPLAY screen variables here!
}

// Gameplay Screen should finish?
int FinishGameplayScreen(void)
{
    return finishScreen;
}