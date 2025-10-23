/**
* Author: Nishat Shamiha
* Assignment: Lunar Lander
* Date due: 2025-10-25, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/Entity.h"

// Global Constants
constexpr int SCREEN_WIDTH  = 990,
              SCREEN_HEIGHT = 720,
              FPS           = 60;

constexpr char BG_COLOUR[]    = "#000000";
constexpr Vector2 ORIGIN      = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };

constexpr int   NUMBER_OF_TILES         = 66,
                NUMBER_OF_LANDING       = 12;
constexpr float TILE_DIMENSION          = 15.0f,
                // in m/ms², since delta time is in ms
                ACCELERATION_OF_GRAVITY = 80.0f, // LOWER GRAVITY
                FIXED_TIMESTEP          = 1.0f / 60.0f,
                MAX_FUEL                = 100.0f,
                FUEL_CONSUMPTION_RATE   = 0.1f;

// Global Variables
AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f,
      gTimeAccumulator = 0.0f,
      gFuelRemaining   = MAX_FUEL,
      gBirdTime = 0.0f; // to track the movement

Entity *gBackground = nullptr;
Entity *gBalloon = nullptr;
Entity *gTiles  = nullptr;
Entity *gLandingBlocks = nullptr;
Entity *gBird = nullptr; // obstacle
Entity *gBird2 = nullptr; // second obstacle

bool gHasWon = false;
bool gHasLost = false;

Texture2D texBG,
          texIdleB,
          texLeftB,
          texLeftestB,
          texRightB,
          texRightestB,
          texCrashB,
          texTile,
          texLandingBlock,
          texBirdLeft,
          texBirdRight;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Balloon Lander");

    texBG = LoadTexture("assets/regbgwhill.PNG");
    texIdleB = LoadTexture("assets/idleb.PNG");
    texLeftB = LoadTexture("assets/leftb.PNG");
    texLeftestB = LoadTexture("assets/leftestb.PNG");
    texRightB = LoadTexture("assets/rightb.PNG");
    texRightestB = LoadTexture("assets/rightestb.PNG");
    texCrashB = LoadTexture("assets/crashb.PNG");
    texTile = LoadTexture("assets/tile.PNG");
    texLandingBlock = LoadTexture("assets/landingblock.PNG");
    texBirdLeft = LoadTexture("assets/birdleft.PNG");
    texBirdRight = LoadTexture("assets/birdright.PNG");

    /*
        ----------- BACKGROUND -----------
    */
    gBackground = new Entity(
        ORIGIN,                                         // position
        { (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT },  // size
        texBG,                                          // texture file address
        NONE                                            // type
    );

    /*
        ----------- HOT AIR BALLOON -----------
    */
    gBalloon = new Entity(
        {ORIGIN.x, ORIGIN.y - 200.0f},                  // position
        {135.0f, 150.0f},                               // size
        texIdleB,                                       // texture file address
        PLAYER                                          // type
    );      

    gBalloon->setJumpingPower(400.0f);
    gBalloon->setColliderDimensions({45.0f, 25.0f});    // jus thte basket
    gBalloon->setColliderOffset({0.0f, gBalloon->getScale().y / 2.0f - 12.5f}); // jus thte basket
    gBalloon->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

    /*
        ----------- HILLS -----------
    */
    int tileHeights[NUMBER_OF_TILES] = { // map for the heights where tiles spawn
        15,12,10,10,11,11,11,11,11,10,
        9,7,6,6,7,11,12,14,17,20,22,20,
        18,17,17,17,17,18,19,16,15,12,
        8,7,6,6,7,8,9,11,9,9,10,12,25,
        27,29,32,32,29,27,23,22,18,18,
        18,18,17,13,11,9,6,5,15,17,24
    };

    int totalTileCount = 0;
    for (int i = 0; i < NUMBER_OF_TILES; i++){
        totalTileCount += tileHeights[i];
    }

    gTiles = new Entity[totalTileCount];

    for (int i = 0; i < NUMBER_OF_TILES; i++){ // spawn a tile at the peaks every col
        int height = tileHeights[i];
        
        Vector2 position = { // position
            i * TILE_DIMENSION + TILE_DIMENSION / 2.0f,
            SCREEN_HEIGHT - (height * TILE_DIMENSION + TILE_DIMENSION / 2.0f)
        };

        gTiles[i].setTexture(texTile);
        gTiles[i].setEntityType(PLATFORM);
        gTiles[i].setScale({ TILE_DIMENSION, TILE_DIMENSION });
        float colliderHeight = tileHeights[i] * TILE_DIMENSION;
        gTiles[i].setColliderDimensions({ TILE_DIMENSION, colliderHeight });
        gTiles[i].setColliderOffset({ 0.0f, colliderHeight / 2.0f - TILE_DIMENSION / 2.0f });
        gTiles[i].setPosition(position);
    }

    /*
        ----------- LANDING PADS -----------
    */
    gLandingBlocks = new Entity[NUMBER_OF_LANDING];
    int landingPadIndices[][5] = {
        {4, 5, 6, 7, 8},        // first landing pad
        {23, 24, 25, 26, -1},    // second landing pad (-1 placeholder)
        {54, 55, 56, -1, -1}     // third landing pad (-1 placeholder)
    };

    int landingBlockIndex = 0;
    for (int pad = 0; pad < 3; pad++){
        for (int i = 0; i < 5; i++){
            int blockIndex = landingPadIndices[pad][i];
            if (blockIndex == -1) continue; // for the placeholder

            // tiles psoition
            Vector2 blockPosition = gTiles[blockIndex].getPosition();

            // make tile that's there inactive
            gTiles[blockIndex].deactivate();

            // set landing block
            gLandingBlocks[landingBlockIndex].setTexture(texLandingBlock);
            gLandingBlocks[landingBlockIndex].setEntityType(BLOCK);
            gLandingBlocks[landingBlockIndex].setScale({TILE_DIMENSION, TILE_DIMENSION});
            // tiny increase in collider dimensions height bcuz overlap w/ tile 
            gLandingBlocks[landingBlockIndex].setColliderDimensions({TILE_DIMENSION, TILE_DIMENSION + 1.0f});
            gLandingBlocks[landingBlockIndex].setPosition(blockPosition);

            landingBlockIndex++;
        }
    }

    /*
    ----------- MOVING BIRD (OBSTACLE) -----------
    */
    gBird = new Entity(
        { TILE_DIMENSION * 7.0f, SCREEN_HEIGHT / 3.0f },  // starting around tile 7ish
        { 2*TILE_DIMENSION, 2*TILE_DIMENSION },           // size
        texBirdRight,                           // starts of right
        PLATFORM                                          // entity type
    );

    gBird->setColliderDimensions({ 2*TILE_DIMENSION, 2*TILE_DIMENSION });

    /*
    ----------- SECOND MOVING BIRD (OBSTACLE) -----------
    */
    gBird2 = new Entity(
        { SCREEN_WIDTH - TILE_DIMENSION * 7.0f, SCREEN_HEIGHT * 0.3f }, // starts right side
        { 2*TILE_DIMENSION, 2*TILE_DIMENSION },             // size
        texBirdLeft,                              // starts facing left
        PLATFORM                                            // entity type
    );

    gBird2->setColliderDimensions({ 2 * TILE_DIMENSION, 2 * TILE_DIMENSION });

    SetTargetFPS(FPS);
}

void processInput() 
{
    Vector2 acceleration = gBalloon->getAcceleration();
    acceleration.y = ACCELERATION_OF_GRAVITY;

    if (gFuelRemaining > 0.0f && (!gHasWon && !gHasLost)){ // no moving after winning/losing
        if (IsKeyDown(KEY_A)){ // animation and movement to the left
            acceleration.x -= 75.0f;  // accelerate to the left
            gFuelRemaining -= FUEL_CONSUMPTION_RATE;
            if (gBalloon->getVelocity().x < -50.0f)
                gBalloon->setTexture(texRightestB);
            else
                gBalloon->setTexture(texRightB);
        }
        else if (IsKeyDown(KEY_D)){ // animation and movement to the right
            acceleration.x += 75.0f;  // accelerate to the right
            gFuelRemaining -= FUEL_CONSUMPTION_RATE;
            if (gBalloon->getVelocity().x > 50.0f)
                gBalloon->setTexture(texLeftestB);
            else
                gBalloon->setTexture(texLeftB);
        }

        if (IsKeyDown(KEY_W) && !(gBalloon->isCollidingBottom())){ 
            acceleration.y += -120.0f;
            gFuelRemaining -= FUEL_CONSUMPTION_RATE;
        }
        if (gFuelRemaining < 0.0f) gFuelRemaining = 0.0f; // cant have neg fuel
    }
    else 
        gBalloon->setTexture(texCrashB);
    
    gBalloon->setAcceleration(acceleration);

    if (IsKeyPressed(KEY_SPACE) && (gHasWon || gHasLost)){ // reset the balloon and game
        gBalloon->setPosition({ ORIGIN.x, ORIGIN.y - 200.0f });
        gBalloon->setAcceleration({ 0.0f, ACCELERATION_OF_GRAVITY });
        gBalloon->setTexture(texIdleB);
        gHasWon = false;
        gHasLost = false;
        gFuelRemaining = MAX_FUEL;
    }

    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
}

void update() 
{
    // Delta time
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    // Fixed timestep
    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP){
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP){
        gBalloon->update(FIXED_TIMESTEP, gTiles, NUMBER_OF_TILES, gLandingBlocks, NUMBER_OF_LANDING);

        if (!gHasLost){
            for (int i = 0; i < NUMBER_OF_LANDING; i++){ // win condition for landing on pads
                if (gBalloon->getLastBottomCollision() == &gLandingBlocks[i]){
                    gHasWon = true;
                    gBalloon->setTexture(texIdleB);
                    gBalloon->setAcceleration({0.0f, 0.0f});
                    break;
                }
            }
        }

        if (!gHasWon){
            for (int i = 0; i < NUMBER_OF_TILES; i++){
                if (!gTiles[i].isActive()) continue;
                // landed on a tile
                if (gBalloon->getLastBottomCollision() == &gTiles[i]){
                    gHasLost = true;
                    gBalloon->setAcceleration({0.0f, 0.0f});
                    gBalloon->setTexture(texCrashB);
                    break;
                }
            }
            // out of fuel while floating
            // TODO: DOUBLE CHECK BOTH FUEL CASES
            if (gFuelRemaining == 0){ // not sure to add !gBalloon->isCollidingBottom() or not
                gHasLost = true;
                gBalloon->setTexture(texCrashB);
            }
        }

        Vector2 balloonPos = gBalloon->getPosition();
        Vector2 balloonScale = gBalloon->getScale();

        float halfWidth  = balloonScale.x / 2.0f;
        float halfHeight = balloonScale.y / 2.0f;

        if (balloonPos.x - halfWidth < 0){ // bounds for side walls
            balloonPos.x = halfWidth;
        }
        else if (balloonPos.x + halfWidth > SCREEN_WIDTH){
            balloonPos.x = SCREEN_WIDTH - halfWidth;
        }
        if (balloonPos.y - halfHeight < 0){ // bounds for top and bottom walls
            balloonPos.y = halfHeight;
        }
        else if (balloonPos.y + halfHeight > SCREEN_HEIGHT){
            balloonPos.y = SCREEN_HEIGHT - halfHeight;
        }

        gBalloon->setPosition(balloonPos);

        for (int i = 0; i < NUMBER_OF_LANDING; i++){
            gLandingBlocks[i].update(FIXED_TIMESTEP, nullptr, 0, nullptr, 0);
        }
        for (int i = 0; i < NUMBER_OF_TILES; i++){
            gTiles[i].update(FIXED_TIMESTEP, nullptr, 0, nullptr, 0);
        }
        deltaTime -= FIXED_TIMESTEP;
    }

    // bird obstacle
    gBirdTime += FIXED_TIMESTEP;

    // move as a sin wave
    float birdAmplitude = 40.0f; // vertical up and down
    float birdSpeed = 1.5f; // speed left and right
    float birdFrequency = 3.0f; // bounciness

    // stay between the 1st and 14th tiles and halfway down the screen
    float birdRange = (TILE_DIMENSION * 15.0f) - (TILE_DIMENSION * 1.0f);
    float birdCenterX = ((TILE_DIMENSION * 15.0f) + (TILE_DIMENSION * 1.0f)) / 2.0f;
    float birdX = birdCenterX + (birdRange / 2.0f) * sin(birdSpeed * gBirdTime);
    float birdY = (SCREEN_HEIGHT / 2.0f) + birdAmplitude * sin(birdFrequency * gBirdTime);

    // changing the texture based on direction
    static float lastX = birdX;
    if (birdX > lastX)
        gBird->setTexture(texBirdRight);
    else if (birdX < lastX)
        gBird->setTexture(texBirdLeft);
    lastX = birdX;

    gBird->setPosition({ birdX, birdY }); // position

    if (gBalloon->isColliding(gBird) && !gHasWon && !gHasLost){ // losing when hitting the bird
        gHasLost = true;
        gBalloon->setTexture(texCrashB);
    }

    // between the last 16 tiles around 50-66
    float bird2Range = (TILE_DIMENSION * 66.0f) - (TILE_DIMENSION * 50.0f);
    float bird2CenterX = ((TILE_DIMENSION * 66.0f) + (TILE_DIMENSION * 50.0f)) / 2.0f;
    float bird2X = bird2CenterX - (birdRange / 2.0f) * sin(birdSpeed * gBirdTime);
    float bird2Y = SCREEN_HEIGHT * 0.3f + birdAmplitude * sin(birdFrequency * gBirdTime);

    // changing texture based on direction
    static float lastX2 = bird2X;
    if (bird2X > lastX2)
        gBird2->setTexture(texBirdRight);
    else if (bird2X < lastX2)
        gBird2->setTexture(texBirdLeft);
    lastX2 = bird2X;

    gBird2->setPosition({ bird2X, bird2Y });

    if (gBalloon->isColliding(gBird2) && !gHasWon && !gHasLost){ // losing when hitting the bird
        gHasLost = true;
        gBalloon->setTexture(texCrashB);
    }

}

void render()
{
    BeginDrawing();
    ClearBackground(BLACK);
    gBackground->render();

    for (int i = 0; i < NUMBER_OF_TILES; i++){
        gTiles[i].render();
        // gTiles[i].renderCollider(); // for debugging collider dims
    }

    for (int i = 0; i < NUMBER_OF_LANDING; i++){
        gLandingBlocks[i].render();
        // gLandingBlocks[i].renderCollider(); // for debugging collider dims
    }

    gBalloon->render();
    // gBalloon->renderCollider(); // for debugging collider dims

    gBird->render();
    // gBird->renderCollider(); // for debugging collider dims

    gBird2->render();
    // gBird2->renderCollider(); // for debugging collider dims

    char fuelText[64];
    sprintf(fuelText, "Fuel: %.1f", gFuelRemaining);
    DrawText(fuelText, 10, 10, 25, BLACK);

    Vector2 accel = gBalloon->getAcceleration();
    
    char accelText[128];
    sprintf(accelText, "X Acceleration: %.2f m/ms²\nY Acceleration: %.2f m/ms²", accel.x, accel.y);

    int fontSize = 20;
    int margin = 10;
    int textWidth = MeasureText("X Acceleration: -0000.00", fontSize);

    DrawText(accelText,
             SCREEN_WIDTH - textWidth - margin - 30,
             margin,
             fontSize,
             BLACK);
    
    Vector2 vel = gBalloon->getVelocity();

    char velText[128];
    sprintf(velText, "X Velocity: %.2f px/ms\nY Velocity: %.2f px/ms", vel.x, vel.y);

    DrawText(velText,
            SCREEN_WIDTH - textWidth - margin - 30,
            margin + 50,
            fontSize,
            BLACK);

    int messageFontSize = 40;
    int messageYOffset  = 85; // fix the margin from bottom

    if (gHasWon){
        const char* winMsg = "YOU LANDED SAFELY!";
        DrawText(
            winMsg,
            SCREEN_WIDTH / 2 - MeasureText(winMsg, messageFontSize) / 2,
            SCREEN_HEIGHT - messageYOffset,
            messageFontSize,
            BLACK
        );
    } 
    else if (gHasLost){
        const char* loseMsg = "YOU CRASHED!";
        DrawText(
            loseMsg,
            SCREEN_WIDTH / 2 - MeasureText(loseMsg, messageFontSize) / 2,
            SCREEN_HEIGHT - messageYOffset,
            messageFontSize,
            BLACK
        );
    }

    if (gHasWon || gHasLost){
        DrawText("press [SPACE] to try again",
                SCREEN_WIDTH / 2 - MeasureText("Press SPACE to try again", 20) / 2,
                SCREEN_HEIGHT - 40,
                20,
                BLACK);
    }

    EndDrawing();
}

void shutdown() 
{
    delete gBackground;
    delete gBalloon;
    delete[] gTiles;
    delete[] gLandingBlocks;
    delete gBird;
    delete gBird2;
    UnloadTexture(texBG);
    UnloadTexture(texIdleB);
    UnloadTexture(texLeftB);
    UnloadTexture(texLeftestB);
    UnloadTexture(texRightB);
    UnloadTexture(texRightestB);
    UnloadTexture(texCrashB);
    UnloadTexture(texTile);
    UnloadTexture(texLandingBlock);
    UnloadTexture(texBirdLeft);
    UnloadTexture(texBirdRight);
    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}