#include "Entity.h"

Entity::Entity() : mPosition {0.0f, 0.0f}, mMovement {0.0f, 0.0f}, 
                   mVelocity {0.0f, 0.0f}, mAcceleration {0.0f, 0.0f},
                   mScale {DEFAULT_SIZE, DEFAULT_SIZE},
                   mColliderDimensions {DEFAULT_SIZE, DEFAULT_SIZE}, 
                   mTexture {NULL}, mTextureType {SINGLE}, mAngle {0.0f},
                   mSpriteSheetDimensions {}, mDirection {RIGHT}, 
                   mAnimationAtlas {{}}, mAnimationIndices {}, mFrameSpeed {0},
                   mEntityType {NONE} { }

Entity::Entity(Vector2 position, Vector2 scale, Texture2D texture, 
        EntityType entityType, Vector2 colliderOffset) 
        : mPosition {position}, mVelocity {0.0f, 0.0f}, 
        mAcceleration {0.0f, 0.0f}, mScale {scale}, mMovement {0.0f, 0.0f}, 
        mColliderDimensions {scale}, mColliderOffset {colliderOffset}, 
        mTexture {texture}, 
        mTextureType {SINGLE}, mDirection {RIGHT}, mAnimationAtlas {{}}, 
        mAnimationIndices {}, mFrameSpeed {0}, mSpeed {DEFAULT_SPEED}, 
        mAngle {0.0f}, mEntityType {entityType} { }

Entity::Entity(Vector2 position, Vector2 scale, Texture2D texture, 
        TextureType textureType, Vector2 spriteSheetDimensions, 
        std::map<Direction, std::vector<int>> animationAtlas, 
        EntityType entityType, Vector2 colliderOffset) 
        : mPosition {position}, mVelocity {0.0f, 0.0f}, 
        mAcceleration {0.0f, 0.0f}, mMovement { 0.0f, 0.0f }, mScale {scale},
        mColliderDimensions {scale}, mColliderOffset {colliderOffset}, 
        mTexture {texture}, 
        mTextureType {ATLAS}, mSpriteSheetDimensions {spriteSheetDimensions},
        mAnimationAtlas {animationAtlas}, mDirection {RIGHT},
        mAnimationIndices {animationAtlas.at(RIGHT)}, 
        mFrameSpeed {DEFAULT_FRAME_SPEED}, mAngle { 0.0f }, 
        mSpeed { DEFAULT_SPEED }, mEntityType {entityType} { }

Entity::~Entity() { UnloadTexture(mTexture); };

void Entity::setTexture(Texture2D texture){
    mTexture = texture;
}

/**
 * Iterates through a list of collidable entities, checks for collisions with
 * the player entity, and resolves any vertical overlap by adjusting the 
 * player's position and velocity accordingly.
 * 
 * @param collidableEntities An array of pointers to `Entity` objects that 
 * represent the entities that the current `Entity` instance can potentially
 * collide with. The `collisionCheckCount` parameter specifies the number of
 * entities in the `collidableEntities` array that need to be checked for
 * collision.
 * @param collisionCheckCount The number of entities that the current entity
 * (`Entity`) should check for collisions with. This parameter specifies how
 * many entities are in the `collidableEntities` array that need to be checked
 * for collisions with the current entity.
 */
void Entity::checkCollisionY(Entity *collidableEntities, int collisionCheckCount)
{
    for (int i = 0; i < collisionCheckCount; i++)
    {
        Entity *collidableEntity = &collidableEntities[i];
        
        if (isColliding(collidableEntity))
        {
            Vector2 thisCenter = { // added the offsets to this
                mPosition.x + mColliderOffset.x,
                mPosition.y + mColliderOffset.y
            };
            Vector2 otherCenter = {
                collidableEntity->mPosition.x + collidableEntity->mColliderOffset.x,
                collidableEntity->mPosition.y + collidableEntity->mColliderOffset.y
            };

            float yDistance = fabs(thisCenter.y - otherCenter.y);
            float yOverlap = fabs(yDistance - (mColliderDimensions.y / 2.0f) - 
                                            (collidableEntity->mColliderDimensions.y / 2.0f));
            
            if (mVelocity.y >= 0){
                mPosition.y -= yOverlap;
                mVelocity.y  = 0;
                mIsCollidingBottom = true;
                mLastBottomCollision = collidableEntity;  // added to track the last collision
            } 
            else if (mVelocity.y < 0){
                mPosition.y += yOverlap;
                mVelocity.y  = 0;
                mIsCollidingTop = true;
            }
        }
    }
}

void Entity::checkCollisionX(Entity *collidableEntities, int collisionCheckCount)
{
    for (int i = 0; i < collisionCheckCount; i++)
    {
        Entity *collidableEntity = &collidableEntities[i];
        
        if (isColliding(collidableEntity))
        {            
            Vector2 thisCenter = { // added the offsets to fix detection
                mPosition.x + mColliderOffset.x,
                mPosition.y + mColliderOffset.y
            };
            Vector2 otherCenter = {
                collidableEntity->mPosition.x + collidableEntity->mColliderOffset.x,
                collidableEntity->mPosition.y + collidableEntity->mColliderOffset.y
            };

            float yDistance = fabs(thisCenter.y - otherCenter.y);
            float yOverlap = fabs(yDistance - (mColliderDimensions.y / 2.0f) - 
                                             (collidableEntity->mColliderDimensions.y / 2.0f));

            if (yOverlap < Y_COLLISION_THRESHOLD) continue;

            float xDistance = fabs(mPosition.x - collidableEntity->mPosition.x);
            float xOverlap  = fabs(xDistance - (mColliderDimensions.x / 2.0f) - (collidableEntity->mColliderDimensions.x / 2.0f));

            if (mVelocity.x > 0) {
                mPosition.x     -= xOverlap;
                mVelocity.x      = 0;

                mIsCollidingRight = true;
            } 
            else if (mVelocity.x < 0) {
                mPosition.x    += xOverlap;
                mVelocity.x     = 0;
 
                mIsCollidingLeft = true;
            }
        }
    }
}

/**
 * Checks if two entities are colliding based on their positions and collider 
 * dimensions.
 * 
 * @param other represents another Entity with which you want to check for 
 * collision. It is a pointer to the Entity class.
 * 
 * @return returns `true` if the two entities are colliding based on their
 * positions and collider dimensions, and `false` otherwise.
 */
bool Entity::isColliding(Entity *other) const 
{
    // added the collider offsets to it to fix detection
    Vector2 thisColliderCenter = {
        mPosition.x + mColliderOffset.x,
        mPosition.y + mColliderOffset.y
    };

    Vector2 otherColliderCenter = {
        other->mPosition.x + other->mColliderOffset.x,
        other->mPosition.y + other->mColliderOffset.y
    };

    float xDistance = fabs(thisColliderCenter.x - otherColliderCenter.x) - 
        ((mColliderDimensions.x + other->mColliderDimensions.x) / 2.0f);
    float yDistance = fabs(thisColliderCenter.y - otherColliderCenter.y) - 
        ((mColliderDimensions.y + other->mColliderDimensions.y) / 2.0f);

    if (xDistance < 0.0f && yDistance < 0.0f) return true;

    return false;
}

/**
 * Updates the current frame index of an entity's animation based on the 
 * elapsed time and frame speed.
 * 
 * @param deltaTime represents the time elapsed since the last frame update.
 */
void Entity::animate(float deltaTime)
{
    mAnimationIndices = mAnimationAtlas.at(mDirection);

    mAnimationTime += deltaTime;
    float framesPerSecond = 1.0f / mFrameSpeed;

    if (mAnimationTime >= framesPerSecond)
    {
        mAnimationTime = 0.0f;

        mCurrentFrameIndex++;
        mCurrentFrameIndex %= mAnimationIndices.size();
    }
}

void Entity::update(float deltaTime, Entity *collidableEntities, 
    int collisionCheckCount,Entity *collidableEntities2, 
    int collisionCheckCount2)
{
    if(mEntityStatus == INACTIVE) return;

    resetColliderFlags();

    mVelocity.x = mMovement.x * mSpeed;

    mVelocity.x += mAcceleration.x * deltaTime;
    mVelocity.y += mAcceleration.y * deltaTime;

    // ––––– JUMPING ––––– //
    if (mIsJumping)
    {
        mIsJumping = false;
        mVelocity.y -= mJumpingPower;
    }

    mPosition.y += mVelocity.y * deltaTime;
    checkCollisionY(collidableEntities, collisionCheckCount);

    mPosition.x += mVelocity.x * deltaTime;
    checkCollisionX(collidableEntities, collisionCheckCount);

    // BLOCKS COLLISION CHECKING -----------------------
    checkCollisionY(collidableEntities2, collisionCheckCount2);
    checkCollisionX(collidableEntities2, collisionCheckCount2);

    if (mTextureType == ATLAS && GetLength(mMovement) != 0 && mIsCollidingBottom) 
        animate(deltaTime);
}

void Entity::render()
{
    if(mEntityStatus == INACTIVE) return;

    Rectangle textureArea;

    switch (mTextureType)
    {
        case SINGLE:
            // Whole texture (UV coordinates)
            textureArea = {
                // top-left corner
                0.0f, 0.0f,

                // bottom-right corner (of texture)
                static_cast<float>(mTexture.width),
                static_cast<float>(mTexture.height)
            };
            break;
        case ATLAS:
            textureArea = getUVRectangle(
                &mTexture, 
                mAnimationIndices[mCurrentFrameIndex], 
                mSpriteSheetDimensions.x, 
                mSpriteSheetDimensions.y
            );
        
        default: break;
    }

    // Destination rectangle – centred on gPosition
    Rectangle destinationArea = {
        mPosition.x,
        mPosition.y,
        static_cast<float>(mScale.x),
        static_cast<float>(mScale.y)
    };

    // Origin inside the source texture (centre of the texture)
    Vector2 originOffset = {
        static_cast<float>(mScale.x) / 2.0f,
        static_cast<float>(mScale.y) / 2.0f
    };

    // Render the texture on screen
    DrawTexturePro(
        mTexture, 
        textureArea, destinationArea, originOffset,
        mAngle, WHITE
    );

}

void Entity::renderCollider(Color color){ // to be able to see the collision dimensions
    if (!isActive()) return;

    Vector2 worldPos = getPosition();
    Vector2 colliderPos = {
        worldPos.x + mColliderOffset.x,
        worldPos.y + mColliderOffset.y
    };

    Rectangle collider = {
        colliderPos.x - mColliderDimensions.x / 2.0f,
        colliderPos.y - mColliderDimensions.y / 2.0f,
        mColliderDimensions.x,
        mColliderDimensions.y
    };

    DrawRectangleLinesEx(collider, 1.5f, color);
}