//
// Created by Lucas N. Ferreira on 28/09/23.
//

#include "AnimatorComponent.h"
#include "../Actors/Actor.h"
#include "../Game.h"
#include "../Json.h"
#include "../Renderer/Texture.h"
#include <fstream>

AnimatorComponent::AnimatorComponent(class Actor *owner, const std::string &texPath, const std::string &dataPath,
                                     int width, int height, int drawOrder)
    : DrawComponent(owner, drawOrder)
      , mAnimTimer(0.0f)
      , mIsPaused(false)
      , mWidth(width)
      , mHeight(height)
      , mTextureFactor(1.0f) {
    mSpriteTexture = owner->GetGame()->GetRenderer()->GetTexture(texPath);

    LoadSpriteSheetData(dataPath);
}

AnimatorComponent::~AnimatorComponent() {
    mAnimations.clear();
    mSpriteSheetData.clear();
}

bool AnimatorComponent::LoadSpriteSheetData(const std::string &dataPath) {
    // Load sprite sheet data and return false if it fails
    std::ifstream spriteSheetFile(dataPath);

    if (!spriteSheetFile.is_open()) {
        SDL_Log("Failed to open sprite sheet data file: %s", dataPath.c_str());
        return false;
    }

    nlohmann::json spriteSheetData = nlohmann::json::parse(spriteSheetFile);

    if (spriteSheetData.is_null()) {
        SDL_Log("Failed to parse sprite sheet data file: %s", dataPath.c_str());
        return false;
    }

    auto textureWidth = static_cast<float>(spriteSheetData["meta"]["size"]["w"].get<int>());
    auto textureHeight = static_cast<float>(spriteSheetData["meta"]["size"]["h"].get<int>());

    for (const auto &frame: spriteSheetData["frames"]) {
        int x = frame["frame"]["x"].get<int>();
        int y = frame["frame"]["y"].get<int>();
        int w = frame["frame"]["w"].get<int>();
        int h = frame["frame"]["h"].get<int>();

        mSpriteSheetData.emplace_back(static_cast<float>(x) / textureWidth, static_cast<float>(y) / textureHeight,
                                      static_cast<float>(w) / textureWidth, static_cast<float>(h) / textureHeight);
    }

    return true;
}

void AnimatorComponent::Draw(Renderer *renderer) {
    if (!mIsVisible || !mSpriteTexture) return;

    auto pos = mOwner->GetPosition();
    auto scale = mOwner->GetScale();
    float rotation = mOwner->GetRotation();
    Vector2 size = Vector2(static_cast<float>(fabs(mWidth * scale.x)), static_cast<float>(fabs(mHeight * scale.y)));
    size.x = static_cast<float>(mWidth) * (scale.x < 0.0f ? -scale.x : scale.x);
    size.y = static_cast<float>(mHeight) * (scale.y < 0.0f ? -scale.y : scale.y);
    Vector2 camera = mOwner->GetGame()->GetCameraPosition();

    Vector4 rect = Vector4::UnitRect;
    bool flipH;
    if (scale.x < 0.0f) flipH = true;
    else flipH = false;

    if (!mAnimName.empty() && !mAnimations[mAnimName].empty()) {
        const auto &frames = mAnimations[mAnimName];
        int currFrame = static_cast<int>(mAnimTimer) % frames.size();
        int spriteIdx = frames[currFrame];
        if (spriteIdx >= 0 && spriteIdx < mSpriteSheetData.size()) {
            rect = mSpriteSheetData[spriteIdx];
            
            if (scale.y < 0.0f) {
                float temp = rect.y;
                rect.y = rect.y + rect.w;
                rect.w = -rect.w;
            }
        }
    }
    renderer->DrawTexture(pos, size, rotation, mColor, mSpriteTexture, rect, camera, flipH, mTextureFactor);
}

void AnimatorComponent::Update(float deltaTime) {
    if (mIsPaused || mAnimations.empty() || mAnimName.empty() || mAnimations.find(mAnimName) == mAnimations.end()) {
        return;
    }
    const auto &frames = mAnimations[mAnimName];
    if (frames.empty()) return;
    mAnimTimer += deltaTime * mAnimFPS;
    if (mAnimTimer >= frames.size()) {
        mAnimTimer -= frames.size();
    }
}

void AnimatorComponent::SetAnimation(const std::string &name) {
    mAnimName = name;
    mAnimTimer = 0.0f;
    Update(0.0f);
}

void AnimatorComponent::AddAnimation(const std::string &name, const std::vector<int> &spriteNums) {
    mAnimations.emplace(name, spriteNums);
}
