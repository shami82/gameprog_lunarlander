#include "CS3113/Entity.h"

// Global Constants
constexpr int SCREEN_WIDTH  = 990,
              SCREEN_HEIGHT = 720,
              FPS           = 120;

constexpr char BG_COLOUR[]    = "#000000";
constexpr Vector2 ORIGIN      = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };

constexpr int   NUMBER_OF_TILES         = 66,
                NUMBER_OF_LANDING       = 11;
constexpr float TILE_DIMENSION          = 15.0f,
                // in m/ms², since delta time is in ms
                ACCELERATION_OF_GRAVITY = 100.0f, // LOWER GRAVITY
                FIXED_TIMESTEP          = 1.0f / 60.0f;

// Global Variables
AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f,
      gTimeAccumulator = 0.0f;

Entity *gBackground = nullptr;
Entity *gBalloon = nullptr;
Entity *gTiles  = nullptr;
Entity *gLandingBlocks = nullptr;

bool gHasWon = false;
bool gHasLost = false;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Balloon Lander");

    /*
        ----------- BACKGROUND -----------
    */
    gBackground = new Entity(
        ORIGIN,                                         // position
        { (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT },  // size
        "assets/bgnohill.PNG",                          // texture file address
        NONE                                            // type
    );

    /*
        ----------- HOT AIR BALLOON -----------
    */
    gBalloon = new Entity(
        {ORIGIN.x, ORIGIN.y - 200.0f},                  // position
        {135.0f, 150.0f},                               // size
        "assets/idleb.PNG",                             // texture file address
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
        9, 7, 6, 6, 7,11,12,14,17,20,
        22,20,18,17,17,17,17,18,19,16,
        15,12, 8, 7, 6, 6, 7, 8, 9,11,
        9, 9,10,12,25,27,29,32,32,29,
        27,23,22,18,18,18,18,17,13,11,
        9, 6, 5,15,17,24
    };

    gTiles = new Entity[NUMBER_OF_TILES];

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

        gTiles[i].setTexture("assets/tile.PNG");
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
    int landingPadIndices[][4] = {
        {5, 6, 7, 8},        // first landing pad
        {23, 24, 25, 26},    // second landing pad
        {54, 55, 56, -1}     // third landing pad (-1 placeholder)
    };

    int landingBlockIndex = 0;
    for (int pad = 0; pad < 3; pad++) {
        for (int i = 0; i < 4; i++) {
            int blockIndex = landingPadIndices[pad][i];
            if (blockIndex == -1) continue; // for the placeholder

            // tiles psoition
            Vector2 blockPosition = gTiles[blockIndex].getPosition();

            // make tile that's there inactive
            gTiles[blockIndex].deactivate();

            // set landing block
            gLandingBlocks[landingBlockIndex].setTexture("assets/landingblock.PNG");
            gLandingBlocks[landingBlockIndex].setEntityType(BLOCK);
            gLandingBlocks[landingBlockIndex].setScale({TILE_DIMENSION, TILE_DIMENSION});
            gLandingBlocks[landingBlockIndex].setColliderDimensions({TILE_DIMENSION, TILE_DIMENSION});
            gLandingBlocks[landingBlockIndex].setPosition(blockPosition);

            landingBlockIndex++;
        }
    }

    SetTargetFPS(FPS);
}

void processInput() 
{
    Vector2 acceleration = gBalloon->getAcceleration();
    acceleration.y = ACCELERATION_OF_GRAVITY;

    if (IsKeyDown(KEY_A)){ // animation and movement to the left
        acceleration.x -= 75.0f;  // accelerate to the left
        if (gBalloon->getVelocity().x < -150.0f)
            gBalloon->setTexture("assets/rightestb.PNG");
        else
            gBalloon->setTexture("assets/rightb.PNG");
    }
    else if (IsKeyDown(KEY_D)){ // animation and movement to the right
        acceleration.x += 75.0f;  // accelerate to the right
        if (gBalloon->getVelocity().x > 150.0f)
            gBalloon->setTexture("assets/leftestb.PNG");
        else
            gBalloon->setTexture("assets/leftb.PNG");
    }

    if (IsKeyDown(KEY_W) && !(gBalloon->isCollidingBottom())) acceleration.y += -120.0f;;

    gBalloon->setAcceleration(acceleration);

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

        for (int i = 0; i < NUMBER_OF_LANDING; i++){ // win condiiton for landing
            if (!gLandingBlocks[i].isActive()) {
                gHasWon = true;
                break;
            }
        }

        for (int i = 0; i < NUMBER_OF_TILES; i++) { // lose condition for landing on tiles
            if (!gTiles[i].isActive()) continue;
            if (gBalloon->isCollidingTop() && gBalloon->isColliding(&gTiles[i])) {
                gHasLost = true;
                break;
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

        // TODO: MAKE SURE THE BALLOON STAYS WITHIN FRAME

        for (int i = 0; i < NUMBER_OF_LANDING; i++){
            gLandingBlocks[i].update(FIXED_TIMESTEP, nullptr, 0, nullptr, 0);
        }
        for (int i = 0; i < NUMBER_OF_TILES; i++){
            gTiles[i].update(FIXED_TIMESTEP, nullptr, 0, nullptr, 0);
        }
        deltaTime -= FIXED_TIMESTEP;
    }

    // ADD THE LOGIC FOR EWNDING THE GAME ------------------------------------
}

void render()
{
    BeginDrawing();
    ClearBackground(BLACK);
    gBackground->render();

    for (int i = 0; i < NUMBER_OF_TILES; i++){
        gTiles[i].render();
        gTiles[i].renderCollider();
    }

    for (int i = 0; i < NUMBER_OF_LANDING; i++){
        gLandingBlocks[i].render();
        gLandingBlocks[i].renderCollider();
    }

    gBalloon->render();
    gBalloon->renderCollider();

    if (gHasWon){
        DrawText("YOU LANDED SAFELY!", SCREEN_WIDTH / 2 - MeasureText("YOU LANDED SAFELY!", 30) / 2,
                SCREEN_HEIGHT - 40, 30, ColorFromHex("0x280000"));
    } 
    else if (gHasLost){
        DrawText("YOU CRASHED!", SCREEN_WIDTH / 2 - MeasureText("YOU CRASHED!", 30) / 2,
                SCREEN_HEIGHT - 40, 30, ColorFromHex("0x280000"));
    }

    EndDrawing();
}

void shutdown() 
{
    delete gBackground;
    delete gBalloon;
    delete[] gTiles;
    delete[] gLandingBlocks;
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