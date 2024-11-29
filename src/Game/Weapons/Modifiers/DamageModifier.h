
#pragma once

#include "Weapons/IWeaponModifier.h"

namespace game
{
    class DamageModifier : public IWeaponModifier
    {
    public:

        DamageModifier(int add_damage_delta);
        DamageModifier(float damage_multiplier);

        BulletConfiguration ModifyBullet(const BulletConfiguration& bullet_config) override;

        int m_damage_delta;
        float m_damage_multiplier;
    };

    class BulletWallModifier : public IWeaponModifier
    {
    public:

        BulletWallModifier();
        WeaponConfiguration ModifyWeapon(const WeaponConfiguration& weapon_config) override;
    };

    class CritChanceModifier : public IWeaponModifier
    {
    public:

        CritChanceModifier(int percent_units);
        BulletConfiguration ModifyBullet(const BulletConfiguration& bullet_config) override;

        const int m_percent_units;
    };
}

/*
Sure! Here are 50 weapon upgrades for a top-down roguelike game that could add a lot of variety, strategic depth, and fun customization to your weapons:

### 1. **Piercing Shot** – Bullets pass through enemies, hitting multiple targets in a line.
### 2. **Explosive Rounds** – Bullets cause a small explosion on impact, dealing area damage.
### 3. **Chain Lightning** – Shots can chain to additional nearby enemies.
### 4. **Double Tap** – Every shot is followed by a second, weaker shot.
### 5. **Critical Hit Chance** – Increases the chance to deal critical damage.
### 6. **Rapid Fire** – Increases the rate of fire for the weapon.
### 7. **Homing Bullets** – Bullets track and home in on enemies within range.
### 8. **Reload Speed** – Decreases the time required to reload the weapon.
### 9. **Auto-Reload** – Automatically reloads the weapon after a set period of time or when out of ammo.
### 10. **Fire Rate Boost** – Temporarily increases fire rate after each kill.
### 11. **Ice Rounds** – Shots freeze enemies on impact, slowing them down or freezing them solid.
### 12. **Poison Bullets** – Shots inflict poison damage over time to enemies.
### 13. **Electric Shots** – Bullets electrify enemies and nearby enemies, dealing electric damage.
### 14. **Shotgun Spread** – Increases the spread of shotgun-type weapons, hitting more targets.
### 15. **Ricochet** – Bullets bounce off walls or enemies, hitting additional targets.
### 16. **Berserk Mode** – Increases damage and rate of fire when low on health.
### 17. **Magnetic Bullets** – Pulls enemies toward the bullet's impact point before detonation.
### 18. **Laser Beam** – Converts the weapon into a continuous laser beam that cuts through enemies.
### 19. **Damage Boost** – Increases the damage dealt by the weapon.
### 20. **Weapon Jamming** – Occasionally jams the weapon, requiring a brief unjamming period.
### 21. **Ammo Regen** – Gradually regenerates ammo over time.
### 22. **Flame Rounds** – Bullets ignite enemies on impact, dealing fire damage over time.
### 23. **Ricochet + Bounce** – Allows bullets to bounce twice, with diminishing returns.
### 24. **Vampiric Shots** – Restores a small amount of health with each hit.
### 25. **Splinter Shot** – Each bullet splits into multiple smaller projectiles upon hitting a target.
### 26. **Laser Sight** – Adds a laser sight that makes it easier to aim and increases accuracy.
### 27. **Hollowpoint Rounds** – Increases damage against armored enemies.
### 28. **Vortex Rounds** – Shots create a small gravitational pull, drawing enemies toward the bullet's impact.
### 29. **Adrenaline Rush** – Temporarily increases your speed and fire rate after a kill.
### 30. **Toxic Cloud** – Shoots bullets that create poisonous clouds on impact.
### 31. **Bullet Storm** – Shoots a barrage of bullets in a short period of time, consuming more ammo.
### 32. **Overcharged Ammo** – Each shot consumes more ammo but deals massive damage.
### 33. **Slow Rounds** – Shots slow down enemies, reducing their movement speed.
### 34. **Explosive Barrels** – Shots create barrels that explode after a short delay.
### 35. **Homing Rockets** – Upgrades the weapon to shoot homing rockets instead of standard bullets.
### 36. **Trapping Shots** – Shots create a temporary trap or obstacle on the ground when they hit.
### 37. **Time Warp Rounds** – Slows down time briefly around the shot’s impact point, giving you more time to react.
### 38. **Elemental Fusion** – Shots can deal multiple types of damage (fire, ice, electric, etc.) at once.
### 39. **Health Leech** – Enemies killed by your shots have a chance to drop health pickups.
### 40. **Teleporting Shot** – After hitting an enemy, teleport you to the impact location.
### 41. **Arcane Shots** – Shots inflict random debuffs (blind, confuse, burn, freeze, etc.) on enemies.
### 42. **Stealth Rounds** – Shots make you briefly invisible or harder to detect.
### 43. **Triple Shot** – Fires three projectiles at once in a spread pattern.
### 44. **Heavy Impact** – Shots knock back enemies a greater distance.
### 45. **Magnetic Pull** – Shots pull items or enemies closer to you when they’re in proximity.
### 46. **Lifesteal** – Restores a portion of health on each hit.
### 47. **Reflective Shield** – Shots have a chance to reflect off walls or obstacles and hit enemies behind cover.
### 48. **Energy Charge** – Shots build up a charge, dealing more damage after consecutive hits.
### 49. **Trap Mines** – Bullets can plant invisible mines that explode when enemies come near.
### 50. **Shrapnel Blast** – On impact, the shot bursts into multiple smaller projectiles that spread outward.

These upgrades can be mixed and matched to create unique weapon playstyles, encouraging players to experiment with different strategies and synergies as they progress through the game.
*/
