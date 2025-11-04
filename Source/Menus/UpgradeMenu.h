#pragma once
#include "Menu.h"
#include <string>

enum class UpgradeRarity
{
    Common,
    Rare,
    Legendary
};

struct Upgrade
{
    std::string name;
    std::string description;
    std::function<void()> onSelect;
    UpgradeRarity rarity;
    bool isSelected;
};

class UpgradeMenu : public Menu
{
public:
    UpgradeMenu(class Game* game);
    void ProcessInput(const Uint8* keyState) override;
    void Draw(class Renderer* renderer) override;
    void GenerateUpgrades();
    
private:
    std::vector<Upgrade> mAvailableUpgrades;
    Vector3 GetRarityColor(UpgradeRarity rarity) const;
};

