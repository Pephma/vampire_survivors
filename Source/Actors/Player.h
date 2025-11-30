#pragma once
#include "Actor.h"

enum class PlayerDirection
{
    Front,
    Back,
    Right,
    Left
};

class Player : public Actor
{
public:
    explicit Player(class Game* game);

    void OnProcessInput(const Uint8* keyState) override;
    void OnUpdate(float deltaTime) override;
    
    // Health system
    float GetHealth() const { return mHealth; }
    float GetMaxHealth() const { return mMaxHealth; }
    void TakeDamage(float damage);
    void Heal(float amount);
    void IncreaseMaxHealth(float amount) { mMaxHealth += amount; mHealth += amount; }
    
    // Upgrade system
    void IncreaseDamageMultiplier(float amount) { mDamageMultiplier += amount; }
    void IncreaseAttackSpeed(float amount) { mAttackSpeedMultiplier += amount; }
    void IncreaseMoveSpeed(float amount) { mMoveSpeed *= (1.0f + amount); }
    void IncreaseProjectileCount(int amount) { mProjectileCount += amount; }
    void EnableHealthRegen() { mHasHealthRegen = true; }
    void IncreaseHealthRegenRate(float amount) { mHealthRegenRate += amount; }
    void SetProjectilePierce(int pierce) { mProjectilePierce = pierce; }
    void IncreaseCritChance(float amount) { mCritChance += amount; if (mCritChance > 1.0f) mCritChance = 1.0f; }
    void IncreaseCritMultiplier(float amount) { mCritMultiplier += amount; }
    void EnableLifesteal() { mHasLifesteal = true; }
    void IncreaseLifestealPercent(float amount) { mLifestealPercent += amount; }
    void IncreaseExperienceGain(float multiplier) { mExperienceMultiplier *= multiplier; }
    void SetBounceProjectiles(bool bounce) { mBounceProjectiles = bounce; }
    void EnableShotgunMode() { mShotgunMode = true; }
    void EnableSpiralMode() { mSpiralMode = true; }
    void EnableOrbitalWeapons() { mOrbitalWeapons = true; mOrbitalAngle = 0.0f; }
    void EnableReverseShot() { mReverseShot = true; }
    void EnableHomingProjectiles() { mHomingProjectiles = true; }
    void EnableExplosiveProjectiles() { mExplosiveProjectiles = true; }
    void IncreaseOrbitalCount(int count) { mOrbitalCount += count; }
    float GetDamageMultiplier() const { return mDamageMultiplier; }
    float GetAttackSpeedMultiplier() const { return mAttackSpeedMultiplier; }
    int GetProjectilePierce() const { return mProjectilePierce; }
    float GetCritChance() const { return mCritChance; }
    float GetCritMultiplier() const { return mCritMultiplier; }
    float GetExperienceMultiplier() const { return mExperienceMultiplier; }
    float GetLifestealPercent() const { return mLifestealPercent; }
    bool HasLifesteal() const { return mHasLifesteal; }
    
    // Experience system
    void AddExperience(float exp);
    float GetExperience() const { return mExperience; }
    float GetExperienceToNextLevel() const { return mExperienceToNextLevel; }
    int GetLevel() const { return mLevel; }
    int GetPendingUpgrades() const { return mPendingUpgrades; }
    void DecrementPendingUpgrades() { if (mPendingUpgrades > 0) mPendingUpgrades--; }
    
    // Auto-attack system
    void UpdateAutoAttack(float deltaTime);

private:
    void UpdateAnimation(const Vector2& moveDirection);
    
    float mHealth;
    float mMaxHealth;
    float mMoveSpeed;
    float mAttackCooldown;
    
    // Upgrades
    float mDamageMultiplier;
    float mAttackSpeedMultiplier;
    int mProjectileCount;
    bool mHasHealthRegen;
    float mHealthRegenRate;
    int mProjectilePierce;
    float mCritChance;
    float mCritMultiplier;
    bool mHasLifesteal;
    float mLifestealPercent;
    float mExperienceMultiplier;
    bool mBounceProjectiles;
    
    // Weapon types
    bool mShotgunMode;
    bool mSpiralMode;
    bool mOrbitalWeapons;
    float mOrbitalAngle;
    int mOrbitalCount;
    bool mReverseShot;
    bool mHomingProjectiles;
    bool mExplosiveProjectiles;
    
    // Experience
    float mExperience;
    float mExperienceToNextLevel;
    int mLevel;
    int mPendingUpgrades;
    
    class AnimatorComponent* mAnimatorComponent;
    class RigidBodyComponent* mRigidBodyComponent;
    class CircleColliderComponent* mCircleColliderComponent;
    
    PlayerDirection mCurrentDirection;
};

