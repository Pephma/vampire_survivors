#include "UpgradeMenu.h"
#include "../Game.h"
#include "../Actors/Player.h"
#include "../Renderer/Renderer.h"
#include "../Renderer/VertexArray.h"
#include "../Renderer/TextRenderer.h"
#include "../Components/DrawComponent.h"
#include "../Random.h"
#include <SDL.h>

UpgradeMenu::UpgradeMenu(Game* game)
    : Menu(game)
{
}

void UpgradeMenu::ProcessInput(const Uint8* keyState)
{
    Uint32 currentTime = SDL_GetTicks();
    
    // Handle navigation with debouncing
    if (keyState[SDL_SCANCODE_W] || keyState[SDL_SCANCODE_UP])
    {
        if (currentTime - mLastKeyPress > 200) // 200ms delay
        {
            mSelectedIndex--;
            if (mSelectedIndex < 0)
                mSelectedIndex = static_cast<int>(mAvailableUpgrades.size() - 1);
            mLastKeyPress = currentTime;
        }
    }
    else if (keyState[SDL_SCANCODE_S] || keyState[SDL_SCANCODE_DOWN])
    {
        if (currentTime - mLastKeyPress > 200) // 200ms delay
        {
            mSelectedIndex++;
            if (mSelectedIndex >= static_cast<int>(mAvailableUpgrades.size()))
                mSelectedIndex = 0;
            mLastKeyPress = currentTime;
        }
    }
    else if (keyState[SDL_SCANCODE_RETURN] || keyState[SDL_SCANCODE_SPACE])
    {
        if (currentTime - mLastKeyPress > 200)
        {
            if (mSelectedIndex >= 0 && mSelectedIndex < static_cast<int>(mAvailableUpgrades.size()))
            {
                if (mAvailableUpgrades[mSelectedIndex].onSelect)
                {
                    mAvailableUpgrades[mSelectedIndex].onSelect();
                    
                    auto* player = mGame->GetPlayer();
                    if (player)
                    {
                        player->DecrementPendingUpgrades();
                    }
                    
                    if (player && player->GetPendingUpgrades() > 0)
                    {
                        GenerateUpgrades();
                    }
                    else
                    {
                        mGame->ResumeGame();
                    }
                }
            }
            mLastKeyPress = currentTime;
        }
    }
    else
    {
        // Reset timer when no keys are pressed for a while
        if (currentTime - mLastKeyPress > 500)
        {
            mLastKeyPress = 0;
        }
    }
}

void UpgradeMenu::Draw(Renderer* renderer)
{
    // Draw semi-transparent background overlay
    std::vector<Vector2> bgVertices;
    bgVertices.emplace_back(Vector2(0.0f, 0.0f));
    bgVertices.emplace_back(Vector2(static_cast<float>(Game::WINDOW_WIDTH), 0.0f));
    bgVertices.emplace_back(Vector2(static_cast<float>(Game::WINDOW_WIDTH), static_cast<float>(Game::WINDOW_HEIGHT)));
    bgVertices.emplace_back(Vector2(0.0f, static_cast<float>(Game::WINDOW_HEIGHT)));
    
    std::vector<float> bgFloatArray;
    std::vector<unsigned int> bgIndices;
    for (size_t i = 0; i < bgVertices.size(); ++i)
    {
        bgFloatArray.push_back(bgVertices[i].x);
        bgFloatArray.push_back(bgVertices[i].y);
        bgFloatArray.push_back(0.0f);
        bgIndices.push_back(static_cast<unsigned int>(i));
    }
    
    Matrix4 bgMatrix = Matrix4::Identity;
    VertexArray bgVA(bgFloatArray.data(), static_cast<unsigned int>(bgVertices.size()), bgIndices.data(), static_cast<unsigned int>(bgIndices.size()));
    Vector3 bgColor(0.05f, 0.05f, 0.1f);  // Dark blue background
    renderer->Draw(bgMatrix, &bgVA, bgColor);
    
    // Draw title background
    std::vector<Vector2> titleBg;
    titleBg.emplace_back(Vector2(static_cast<float>(Game::WINDOW_WIDTH) / 2.0f - 200.0f, 30.0f));
    titleBg.emplace_back(Vector2(static_cast<float>(Game::WINDOW_WIDTH) / 2.0f + 200.0f, 30.0f));
    titleBg.emplace_back(Vector2(static_cast<float>(Game::WINDOW_WIDTH) / 2.0f + 200.0f, 90.0f));
    titleBg.emplace_back(Vector2(static_cast<float>(Game::WINDOW_WIDTH) / 2.0f - 200.0f, 90.0f));
    
    std::vector<float> titleFloatArray;
    std::vector<unsigned int> titleIndices;
    for (size_t i = 0; i < titleBg.size(); ++i)
    {
        titleFloatArray.push_back(titleBg[i].x);
        titleFloatArray.push_back(titleBg[i].y);
        titleFloatArray.push_back(0.0f);
        titleIndices.push_back(static_cast<unsigned int>(i));
    }
    
    Matrix4 titleMatrix = Matrix4::Identity;
    VertexArray titleVA(titleFloatArray.data(), static_cast<unsigned int>(titleBg.size()), titleIndices.data(), static_cast<unsigned int>(titleIndices.size()));
    Vector3 titleColor(0.1f, 0.3f, 0.5f);
    renderer->Draw(titleMatrix, &titleVA, titleColor);
    
    // Draw title text - make it bigger and more visible
    TextRenderer::DrawText(renderer, "LEVEL UP!", Vector2(static_cast<float>(Game::WINDOW_WIDTH) / 2.0f - 140.0f, 45.0f), 1.8f, Vector3(1.0f, 1.0f, 0.0f));
    TextRenderer::DrawText(renderer, "CHOOSE UPGRADE", Vector2(static_cast<float>(Game::WINDOW_WIDTH) / 2.0f - 160.0f, 70.0f), 1.2f, Vector3(1.0f, 1.0f, 1.0f));
    
    // Draw upgrades
    float yPos = 120.0f;
    for (size_t i = 0; i < mAvailableUpgrades.size(); ++i)
    {
        const auto& upgrade = mAvailableUpgrades[i];
        bool isSelected = (static_cast<int>(i) == mSelectedIndex);
        
        // Draw upgrade box with border - make selection more visible
        std::vector<Vector2> upgradeVertices;
        float boxWidth = 450.0f;
        float boxHeight = 70.0f;
        float xCenter = static_cast<float>(Game::WINDOW_WIDTH) / 2.0f;
        
        upgradeVertices.emplace_back(Vector2(xCenter - boxWidth/2, yPos));
        upgradeVertices.emplace_back(Vector2(xCenter + boxWidth/2, yPos));
        upgradeVertices.emplace_back(Vector2(xCenter + boxWidth/2, yPos + boxHeight));
        upgradeVertices.emplace_back(Vector2(xCenter - boxWidth/2, yPos + boxHeight));
        
        std::vector<float> upgradeFloatArray;
        std::vector<unsigned int> upgradeIndices;
        for (size_t j = 0; j < upgradeVertices.size(); ++j)
        {
            upgradeFloatArray.push_back(upgradeVertices[j].x);
            upgradeFloatArray.push_back(upgradeVertices[j].y);
            upgradeFloatArray.push_back(0.0f);
            upgradeIndices.push_back(static_cast<unsigned int>(j));
        }
        
        Matrix4 upgradeMatrix = Matrix4::Identity;
        VertexArray upgradeVA(upgradeFloatArray.data(), static_cast<unsigned int>(upgradeVertices.size()), upgradeIndices.data(), static_cast<unsigned int>(upgradeIndices.size()));
        
        // Use rarity color for background
        Vector3 rarityColor = GetRarityColor(upgrade.rarity);
        Vector3 bgColor;
        if (isSelected)
        {
            bgColor = Vector3(rarityColor.x * 0.6f, rarityColor.y * 0.6f, rarityColor.z * 0.6f);
        }
        else
        {
            bgColor = Vector3(rarityColor.x * 0.2f, rarityColor.y * 0.2f, rarityColor.z * 0.2f);
        }
        renderer->Draw(upgradeMatrix, &upgradeVA, bgColor);
        
        // Draw selection indicator - make it bigger and more visible
        if (isSelected)
        {
            std::vector<Vector2> indicator;
            float indSize = 8.0f;
            indicator.emplace_back(Vector2(xCenter - boxWidth/2 - 15, yPos + boxHeight/2 - indSize));
            indicator.emplace_back(Vector2(xCenter - boxWidth/2 - 15, yPos + boxHeight/2 + indSize));
            indicator.emplace_back(Vector2(xCenter - boxWidth/2 - 5, yPos + boxHeight/2));
            
            std::vector<float> indFloatArray;
            std::vector<unsigned int> indIndices;
            for (size_t j = 0; j < indicator.size(); ++j)
            {
                indFloatArray.push_back(indicator[j].x);
                indFloatArray.push_back(indicator[j].y);
                indFloatArray.push_back(0.0f);
                indIndices.push_back(static_cast<unsigned int>(j));
            }
            
            Matrix4 indMatrix = Matrix4::Identity;
            VertexArray indVA(indFloatArray.data(), static_cast<unsigned int>(indicator.size()), indIndices.data(), static_cast<unsigned int>(indIndices.size()));
            renderer->Draw(indMatrix, &indVA, Vector3(1.0f, 0.8f, 0.0f));
        }
        
        // Draw upgrade text with rarity color (reuse rarityColor from above)
        Vector3 textColor = isSelected ? Vector3(1.0f, 1.0f, 0.3f) : rarityColor;
        
        // Draw rarity indicator
        std::string rarityText = "";
        if (upgrade.rarity == UpgradeRarity::Rare) rarityText = "[RARE] ";
        else if (upgrade.rarity == UpgradeRarity::Legendary) rarityText = "[LEGENDARY] ";
        
        TextRenderer::DrawText(renderer, rarityText + upgrade.name, Vector2(xCenter - boxWidth/2 + 20.0f, yPos + 18.0f), 1.4f, textColor);
        TextRenderer::DrawText(renderer, upgrade.description, Vector2(xCenter - boxWidth/2 + 20.0f, yPos + 45.0f), 1.0f, Vector3(0.8f, 0.8f, 0.8f));
        
        yPos += 90.0f;
    }
    
    // Draw instructions - make it bigger
    TextRenderer::DrawText(renderer, "USE ARROWS TO SELECT, ENTER TO CHOOSE", Vector2(static_cast<float>(Game::WINDOW_WIDTH) / 2.0f - 240.0f, static_cast<float>(Game::WINDOW_HEIGHT) - 50.0f), 1.0f, Vector3(1.0f, 1.0f, 0.5f));
}

Vector3 UpgradeMenu::GetRarityColor(UpgradeRarity rarity) const
{
    switch (rarity)
    {
        case UpgradeRarity::Common:
            return Vector3(0.9f, 0.9f, 0.9f); // White
        case UpgradeRarity::Rare:
            return Vector3(0.2f, 0.6f, 1.0f); // Blue
        case UpgradeRarity::Legendary:
            return Vector3(1.0f, 0.5f, 0.0f); // Orange/Gold
        default:
            return Vector3(1.0f, 1.0f, 1.0f);
    }
}

void UpgradeMenu::GenerateUpgrades()
{
    mAvailableUpgrades.clear();
    mMenuItems.clear();
    
    auto* player = mGame->GetPlayer();
    if (!player)
        return;
    
    // Create lists of upgrades by rarity
    std::vector<Upgrade> commonUpgrades;
    std::vector<Upgrade> rareUpgrades;
    std::vector<Upgrade> legendaryUpgrades;
    
    // ===== COMMON UPGRADES =====
    
    // Health upgrade
    Upgrade healthUpgrade;
    healthUpgrade.name = "MAX HEALTH +20";
    healthUpgrade.description = "Increase maximum health by 20";
    healthUpgrade.rarity = UpgradeRarity::Common;
    healthUpgrade.onSelect = [player]() { player->IncreaseMaxHealth(20.0f); };
    commonUpgrades.push_back(healthUpgrade);
    
    // Damage upgrade
    Upgrade damageUpgrade;
    damageUpgrade.name = "DAMAGE +15%";
    damageUpgrade.description = "Increase weapon damage by 15%";
    damageUpgrade.rarity = UpgradeRarity::Common;
    damageUpgrade.onSelect = [player]() { player->IncreaseDamageMultiplier(0.15f); };
    commonUpgrades.push_back(damageUpgrade);
    
    // Attack speed upgrade
    Upgrade speedUpgrade;
    speedUpgrade.name = "ATTACK SPEED +20%";
    speedUpgrade.description = "Increase attack speed by 20%";
    speedUpgrade.rarity = UpgradeRarity::Common;
    speedUpgrade.onSelect = [player]() { player->IncreaseAttackSpeed(0.20f); };
    commonUpgrades.push_back(speedUpgrade);
    
    // Movement speed upgrade
    Upgrade moveSpeedUpgrade;
    moveSpeedUpgrade.name = "MOVEMENT SPEED +20%";
    moveSpeedUpgrade.description = "Move faster";
    moveSpeedUpgrade.rarity = UpgradeRarity::Common;
    moveSpeedUpgrade.onSelect = [player]() { player->IncreaseMoveSpeed(0.2f); };
    commonUpgrades.push_back(moveSpeedUpgrade);
    
    // Projectile count upgrade
    Upgrade projectileUpgrade;
    projectileUpgrade.name = "MORE PROJECTILES";
    projectileUpgrade.description = "Fire 2 additional projectiles";
    projectileUpgrade.rarity = UpgradeRarity::Common;
    projectileUpgrade.onSelect = [player]() { player->IncreaseProjectileCount(2); };
    commonUpgrades.push_back(projectileUpgrade);
    
    // Health regen upgrade
    Upgrade regenUpgrade;
    regenUpgrade.name = "HEALTH REGEN";
    regenUpgrade.description = "Regenerate 5 HP per second";
    regenUpgrade.rarity = UpgradeRarity::Common;
    regenUpgrade.onSelect = [player]() { player->EnableHealthRegen(); };
    commonUpgrades.push_back(regenUpgrade);
    
    // ===== RARE UPGRADES =====
    
    // Critical Strike
    Upgrade critUpgrade;
    critUpgrade.name = "CRITICAL STRIKE";
    critUpgrade.description = "15% chance to deal 3x damage";
    critUpgrade.rarity = UpgradeRarity::Rare;
    critUpgrade.onSelect = [player]() { player->IncreaseCritChance(0.15f); player->IncreaseCritMultiplier(1.0f); };
    rareUpgrades.push_back(critUpgrade);
    
    // Pierce
    Upgrade pierceUpgrade;
    pierceUpgrade.name = "PIERCING SHOTS";
    pierceUpgrade.description = "Projectiles pierce through 3 enemies";
    pierceUpgrade.rarity = UpgradeRarity::Rare;
    pierceUpgrade.onSelect = [player]() { player->SetProjectilePierce(3); };
    rareUpgrades.push_back(pierceUpgrade);
    
    // Lifesteal
    Upgrade lifestealUpgrade;
    lifestealUpgrade.name = "LIFESTEAL";
    lifestealUpgrade.description = "Heal 10% of damage dealt";
    lifestealUpgrade.rarity = UpgradeRarity::Rare;
    lifestealUpgrade.onSelect = [player]() { player->EnableLifesteal(); };
    rareUpgrades.push_back(lifestealUpgrade);
    
    // Mega Damage
    Upgrade megaDamageUpgrade;
    megaDamageUpgrade.name = "MEGA DAMAGE";
    megaDamageUpgrade.description = "Increase damage by 50%";
    megaDamageUpgrade.rarity = UpgradeRarity::Rare;
    megaDamageUpgrade.onSelect = [player]() { player->IncreaseDamageMultiplier(0.5f); };
    rareUpgrades.push_back(megaDamageUpgrade);
    
    // Speed Demon
    Upgrade speedDemonUpgrade;
    speedDemonUpgrade.name = "SPEED DEMON";
    speedDemonUpgrade.description = "Increase movement speed by 50%";
    speedDemonUpgrade.rarity = UpgradeRarity::Rare;
    speedDemonUpgrade.onSelect = [player]() { player->IncreaseMoveSpeed(0.5f); };
    rareUpgrades.push_back(speedDemonUpgrade);
    
    // Rapid Fire
    Upgrade rapidFireUpgrade;
    rapidFireUpgrade.name = "RAPID FIRE";
    rapidFireUpgrade.description = "Double attack speed";
    rapidFireUpgrade.rarity = UpgradeRarity::Rare;
    rapidFireUpgrade.onSelect = [player]() { player->IncreaseAttackSpeed(1.0f); };
    rareUpgrades.push_back(rapidFireUpgrade);
    
    // Shotgun Mode
    Upgrade shotgunUpgrade;
    shotgunUpgrade.name = "SHOTGUN MODE";
    shotgunUpgrade.description = "Wide spread attack pattern";
    shotgunUpgrade.rarity = UpgradeRarity::Rare;
    shotgunUpgrade.onSelect = [player]() { player->EnableShotgunMode(); };
    rareUpgrades.push_back(shotgunUpgrade);
    
    // Spiral Attack
    Upgrade spiralUpgrade;
    spiralUpgrade.name = "SPIRAL ATTACK";
    spiralUpgrade.description = "Rotating spiral pattern";
    spiralUpgrade.rarity = UpgradeRarity::Rare;
    spiralUpgrade.onSelect = [player]() { player->EnableSpiralMode(); };
    rareUpgrades.push_back(spiralUpgrade);
    
    // Orbital Weapons
    Upgrade orbitalUpgrade;
    orbitalUpgrade.name = "ORBITAL WEAPONS";
    orbitalUpgrade.description = "Projectiles orbit around you";
    orbitalUpgrade.rarity = UpgradeRarity::Rare;
    orbitalUpgrade.onSelect = [player]() { player->EnableOrbitalWeapons(); };
    rareUpgrades.push_back(orbitalUpgrade);
    
    // Reverse Shot
    Upgrade reverseShotUpgrade;
    reverseShotUpgrade.name = "REVERSE SHOT";
    reverseShotUpgrade.description = "Also shoot backwards";
    reverseShotUpgrade.rarity = UpgradeRarity::Rare;
    reverseShotUpgrade.onSelect = [player]() { player->EnableReverseShot(); };
    rareUpgrades.push_back(reverseShotUpgrade);
    
    // Dash Ability
    Upgrade dashUpgrade;
    dashUpgrade.name = "DASH";
    dashUpgrade.description = "Press SPACE to dash (2s cooldown)";
    dashUpgrade.rarity = UpgradeRarity::Rare;
    dashUpgrade.onSelect = [player]() { player->EnableDash(); };
    rareUpgrades.push_back(dashUpgrade);
    
    // ===== LEGENDARY UPGRADES =====
    
    // Death Ray
    Upgrade deathRayUpgrade;
    deathRayUpgrade.name = "DEATH RAY";
    deathRayUpgrade.description = "Triple damage + infinite pierce";
    deathRayUpgrade.rarity = UpgradeRarity::Legendary;
    deathRayUpgrade.onSelect = [player]() { player->IncreaseDamageMultiplier(3.0f); player->SetProjectilePierce(999); };
    legendaryUpgrades.push_back(deathRayUpgrade);
    
    // Omnipotence
    Upgrade omnipotenceUpgrade;
    omnipotenceUpgrade.name = "OMNIPOTENCE";
    omnipotenceUpgrade.description = "50% crit chance + 5x crit damage";
    omnipotenceUpgrade.rarity = UpgradeRarity::Legendary;
    omnipotenceUpgrade.onSelect = [player]() { player->IncreaseCritChance(0.5f); player->IncreaseCritMultiplier(3.0f); };
    legendaryUpgrades.push_back(omnipotenceUpgrade);
    
    // Vampiric Aura
    Upgrade vampiricUpgrade;
    vampiricUpgrade.name = "VAMPIRIC AURA";
    vampiricUpgrade.description = "50% lifesteal + 50 HP regen/sec";
    vampiricUpgrade.rarity = UpgradeRarity::Legendary;
    vampiricUpgrade.onSelect = [player]() { player->EnableLifesteal(); player->IncreaseLifestealPercent(0.4f); player->EnableHealthRegen(); player->IncreaseHealthRegenRate(45.0f); };
    legendaryUpgrades.push_back(vampiricUpgrade);
    
    // Bullet Storm
    Upgrade bulletStormUpgrade;
    bulletStormUpgrade.name = "BULLET STORM";
    bulletStormUpgrade.description = "+10 projectiles + 2x attack speed";
    bulletStormUpgrade.rarity = UpgradeRarity::Legendary;
    bulletStormUpgrade.onSelect = [player]() { player->IncreaseProjectileCount(10); player->IncreaseAttackSpeed(2.0f); };
    legendaryUpgrades.push_back(bulletStormUpgrade);
    
    // Experience Boost
    Upgrade expBoostUpgrade;
    expBoostUpgrade.name = "WISDOM";
    expBoostUpgrade.description = "Gain 2x experience from kills";
    expBoostUpgrade.rarity = UpgradeRarity::Legendary;
    expBoostUpgrade.onSelect = [player]() { player->IncreaseExperienceGain(2.0f); };
    legendaryUpgrades.push_back(expBoostUpgrade);
    
    // God Mode
    Upgrade godModeUpgrade;
    godModeUpgrade.name = "GOD MODE";
    godModeUpgrade.description = "+200 HP + 100% damage + 50% speed";
    godModeUpgrade.rarity = UpgradeRarity::Legendary;
    godModeUpgrade.onSelect = [player]() { player->IncreaseMaxHealth(200.0f); player->IncreaseDamageMultiplier(1.0f); player->IncreaseMoveSpeed(0.5f); };
    legendaryUpgrades.push_back(godModeUpgrade);
    
    // Homing Missiles
    Upgrade homingUpgrade;
    homingUpgrade.name = "HOMING MISSILES";
    homingUpgrade.description = "Projectiles track enemies";
    homingUpgrade.rarity = UpgradeRarity::Legendary;
    homingUpgrade.onSelect = [player]() { player->EnableHomingProjectiles(); player->IncreaseDamageMultiplier(0.5f); };
    legendaryUpgrades.push_back(homingUpgrade);
    
    // Explosive Rounds - REMOVED
    
    // Double Orbital Ring
    Upgrade doubleOrbitalUpgrade;
    doubleOrbitalUpgrade.name = "DOUBLE ORBITAL RING";
    doubleOrbitalUpgrade.description = "2x orbital weapons + 4 more";
    doubleOrbitalUpgrade.rarity = UpgradeRarity::Legendary;
    doubleOrbitalUpgrade.onSelect = [player]() { player->EnableOrbitalWeapons(); player->IncreaseOrbitalCount(4); player->IncreaseDamageMultiplier(0.3f); };
    legendaryUpgrades.push_back(doubleOrbitalUpgrade);
    
    // Chaos Mode
    Upgrade chaosUpgrade;
    chaosUpgrade.name = "CHAOS MODE";
    chaosUpgrade.description = "Shotgun + Spiral + Reverse";
    chaosUpgrade.rarity = UpgradeRarity::Legendary;
    chaosUpgrade.onSelect = [player]() { player->EnableShotgunMode(); player->EnableSpiralMode(); player->EnableReverseShot(); player->IncreaseProjectileCount(5); };
    legendaryUpgrades.push_back(chaosUpgrade);
    
    // Select upgrades with weighted rarity - better chances for rare/legendary
    // 50% common, 35% rare, 15% legendary (improved chances)
    while (mAvailableUpgrades.size() < 3)
    {
        float roll = Random::GetFloat();
        Upgrade selected;
        
        if (roll < 0.15f && !legendaryUpgrades.empty())
        {
            // 15% legendary (increased from 10%)
            int index = Random::GetIntRange(0, static_cast<int>(legendaryUpgrades.size()) - 1);
            selected = legendaryUpgrades[index];
            legendaryUpgrades.erase(legendaryUpgrades.begin() + index);
        }
        else if (roll < 0.5f && !rareUpgrades.empty())
        {
            // 35% rare (increased from 30%)
            int index = Random::GetIntRange(0, static_cast<int>(rareUpgrades.size()) - 1);
            selected = rareUpgrades[index];
            rareUpgrades.erase(rareUpgrades.begin() + index);
        }
        else if (!commonUpgrades.empty())
        {
            // 60% common
            int index = Random::GetIntRange(0, static_cast<int>(commonUpgrades.size()) - 1);
            selected = commonUpgrades[index];
            commonUpgrades.erase(commonUpgrades.begin() + index);
        }
        else if (!rareUpgrades.empty())
        {
            int index = Random::GetIntRange(0, static_cast<int>(rareUpgrades.size()) - 1);
            selected = rareUpgrades[index];
            rareUpgrades.erase(rareUpgrades.begin() + index);
        }
        else if (!legendaryUpgrades.empty())
        {
            int index = Random::GetIntRange(0, static_cast<int>(legendaryUpgrades.size()) - 1);
            selected = legendaryUpgrades[index];
            legendaryUpgrades.erase(legendaryUpgrades.begin() + index);
        }
        else
        {
            break; // No more upgrades available
        }
        
        mAvailableUpgrades.push_back(selected);
    }
    
    // Create menu items - match the yPos from Draw function
    float yPos = 120.0f;
    for (size_t i = 0; i < mAvailableUpgrades.size(); ++i)
    {
        MenuItem item;
        item.text = mAvailableUpgrades[i].name;
        item.position = Vector2(static_cast<float>(Game::WINDOW_WIDTH) / 2.0f - 225.0f, yPos);
        item.size = Vector2(450.0f, 70.0f);
        item.onSelect = mAvailableUpgrades[i].onSelect;
        mMenuItems.push_back(item);
        yPos += 90.0f;
    }
    
    mSelectedIndex = 0;
}

