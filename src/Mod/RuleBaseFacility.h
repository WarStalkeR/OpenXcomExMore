#pragma once
/*
 * Copyright 2010-2016 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <string>
#include <vector>
#include <map>
#include <bitset>
#include <yaml-cpp/yaml.h>
#include "RuleBaseFacilityFunctions.h"

namespace OpenXcom
{

class Mod;
class Base;
class Position;
class RuleItem;
struct VerticalLevel;
enum BasePlacementErrors : int;

struct CraftOption
{
	/// Horizontal offset for rendering craft in the facility.
	int x = 0;
	/// Vertical offset for rendering craft in the facility.
	int y = 0;
	/// Minimum size of craft that can be housed in the slot.
	int min = 0;
	/// Maximum size of craft that can be housed in the slot.
	int max = 0;
	/// Is craft hidden or rendered in the base in the slot.
	bool hide = false;
	/// Group number of the slot. If 0 then there is no group.
	int group = 0;
	/// Subset number of the group. Irrelevant if group is 0.
	int subset = 0;
	/// Default constructor for the struct.
	CraftOption() : x(0), y(0), min(0), max(0),
		hide(false), group(0), subset(0)
	{ /* Initialized via constructor */ }
	/// Constructor that allows to define facility craft slots.
	CraftOption(int xOffset, int yOffset, int minSize, int maxSize,
		bool isHidden, int slotGroup, int slotSubset) :
		x(xOffset), y(yOffset), min(minSize), max(maxSize),
		hide(isHidden), group(slotGroup), subset(slotSubset)
	{ /* Initialized via constructor */ }
};

struct CraftValue
{
	/// Smallest possible number of crafts in the facility.
	int totalMin = 0;
	/// Largest possible number of crafts in the facility.
	int totalMax = 0;
	/// Default constructor for the struct.
	CraftValue() : totalMin(0), totalMax(0)
	{ /* Initialized via constructor */ }
	/// Constructor that defines possible min/max slots number.
	CraftValue(int totalCraftsMin, int totalCraftsMax) :
		totalMin(totalCraftsMin), totalMax(totalCraftsMax)
	{ /* Initialized via constructor */ }
};

/**
 * Represents a specific type of base facility.
 * Contains constant info about a facility like
 * costs, capacities, size, etc.
 * @sa BaseFacility
 */
class RuleBaseFacility
{
private:
	std::string _type;
	std::vector<std::string> _requires;
	RuleBaseFacilityFunctions _requiresBaseFunc = 0;
	RuleBaseFacilityFunctions _provideBaseFunc = 0;
	RuleBaseFacilityFunctions _forbiddenBaseFunc = 0;
	int _spriteShape, _spriteFacility;
	bool _connectorsDisabled;
	int _missileAttraction;
	int _fakeUnderwater;
	bool _lift, _hyper, _mind, _grav;
	int _mindPower;
	int _sizeX, _sizeY;
	int _buildCost, _refundValue, _buildTime, _monthlyCost;
	std::map<std::string, std::pair<int, int> > _buildCostItems;
	int _storage, _personnel, _aliens, _crafts, _labs, _workshops, _psiLabs;
	bool _spriteEnabled, _altBuildSprite, _craftsHidden;
	std::vector<CraftOption> _craftOptions;
	int _sightRange, _sightChance;
	int _radarRange, _radarChance, _defense, _hitRatio, _fireSound, _hitSound, _placeSound;
	int _ammoMax, _rearmRate;
	int _ammoNeeded;
	const RuleItem* _ammoItem = nullptr;
	std::string _ammoItemName;
	std::string _mapName;
	int _listOrder, _trainingRooms;
	int _maxAllowedPerBase;
	int _manaRecoveryPerDay = 0;
	int _healthRecoveryPerDay = 0;
	float _sickBayAbsoluteBonus, _sickBayRelativeBonus;
	int _prisonType;
	int _rightClickActionType;
	std::vector<VerticalLevel> _verticalLevels;
	std::vector<const RuleBaseFacility*> _leavesBehindOnSell;
	int _removalTime;
	bool _canBeBuiltOver;
	std::vector<const RuleBaseFacility*> _buildOverFacilities;
	std::vector<Position> _storageTiles;
	std::string _destroyedFacilityName;
	const RuleBaseFacility* _destroyedFacility;


	std::vector<std::string> _leavesBehindOnSellNames;
	std::vector<std::string> _buildOverFacilitiesNames;

public:
	/// Creates a blank facility ruleset.
	RuleBaseFacility(const std::string &type, int listOrder);
	/// Cleans up the facility ruleset.
	~RuleBaseFacility();
	/// Loads the facility from YAML.
	void load(const YAML::Node& node, Mod *mod);
	/// Cross link with other rules.
	void afterLoad(const Mod* mod);
	/// Gets the facility's type.
	const std::string& getType() const;
	/// Gets the facility's requirements.
	const std::vector<std::string> &getRequirements() const;
	/// Gets the facility's required function in base to build.
	RuleBaseFacilityFunctions getRequireBaseFunc() const { return _requiresBaseFunc; }
	/// Gets the functions that facility provide in base.
	RuleBaseFacilityFunctions getProvidedBaseFunc() const { return _provideBaseFunc; }
	/// Gets the functions that facility prevent in base.
	RuleBaseFacilityFunctions getForbiddenBaseFunc() const { return _forbiddenBaseFunc; }
	/// Gets the facility's shape sprite.
	int getSpriteShape() const;
	/// Gets the facility's content sprite.
	int getSpriteFacility() const;
	/// Retain sprite over shape behavior for bigger facility?
	bool getSpriteEnabled() const;
	bool getSpriteEnabledRaw() const { return _spriteEnabled; }
	/// Use alternative sprite during construction (like shapes do)?
	bool useAltBuildSprite() const { return _altBuildSprite; }
	/// Should there be connectors leading to this facility?
	bool connectorsDisabled() const { return _connectorsDisabled; }
	/// Gets if the facility's size is 1x1.
	bool isSmall() const;
	/// Gets the facility's size.
	int getSizeX() const { return _sizeX; }
	/// Gets the facility's size.
	int getSizeY() const { return _sizeY; }
	/// Gets the facility's missile attraction.
	int getMissileAttraction() const { return _missileAttraction; }
	/// Is this facility allowed for a given type of base?
	bool isAllowedForBaseType(bool fakeUnderwaterBase) const;
	int getFakeUnderwaterRaw() const { return _fakeUnderwater; }
	/// Gets if the facility is an access lift.
	bool isLift() const;
	/// Gets if the facility has hyperwave detection.
	bool isHyperwave() const;
	/// Gets if the facility is a mind shield.
	bool isMindShield() const;
	/// Gets the mind shield power.
	int getMindShieldPower() const;
	/// Gets if the facility is a grav shield.
	bool isGravShield() const;
	/// Gets the facility's construction cost.
	int getBuildCost() const;
	/// Gets the facility's refund value.
	int getRefundValue() const;
	/// Gets the facility's construction cost in items, `first` is build cost, `second` is refund.
	const std::map<std::string, std::pair<int, int> >& getBuildCostItems() const;
	/// Gets the facility's construction time.
	int getBuildTime() const;
	/// Gets the facility's monthly cost.
	int getMonthlyCost() const;
	/// Gets the facility's storage capacity.
	int getStorage() const;
	/// Gets the facility's personnel capacity.
	int getPersonnel() const;
	/// Gets the facility's alien capacity.
	int getAliens() const;
	/// Gets the facility's craft capacity.
	int getCrafts() const;
	/// Gets the facility's laboratory space.
	int getLaboratories() const;
	/// Gets the facility's workshop space.
	int getWorkshops() const;
	/// Gets the facility's psi-training capacity.
	int getPsiLaboratories() const;
	/// Gets if facility's crafts are hidden or not.
	bool getCraftsHidden() const;
	/// Gets a list of craft slots in this facility
	const std::vector<CraftOption>& getCraftOptions() const;
	/// Gets the minimum/maximum possible crafts number in the facility.
	const CraftValue getCraftsNum() const;
	/// Gets the facility's sight range.
	int getSightRange() const { return _sightRange; }
	/// Gets the facility's alien base detection chance.
	int getSightChance() const { return _sightChance; }
	/// Gets the facility's radar range.
	int getRadarRange() const;
	/// Gets the facility's detection chance.
	int getRadarChance() const;
	/// Gets the facility's defense value.
	int getDefenseValue() const;
	/// Gets the facility's weapon hit ratio.
	int getHitRatio() const;
	/// Gets the facility's weapon ammo capacity.
	int getAmmoMax() const { return _ammoMax; }
	/// Gets the facility's weapon rearm rate.
	int getRearmRate() const { return _rearmRate; }
	/// Gets the facility's weapon ammo spent per shot.
	int getAmmoNeeded() const { return _ammoNeeded; }
	/// Gets the facility's weapon ammo item.
	const RuleItem* getAmmoItem() const { return _ammoItem; }
	/// Gets the facility's battlescape map name.
	std::string getMapName() const;
	/// Gets the facility's fire sound.
	int getFireSound() const;
	/// Gets the facility's hit sound.
	int getHitSound() const;
	/// Gets the facility's place sound.
	int getPlaceSound() const { return _placeSound; }
	/// Gets the facility's list weight.
	int getListOrder() const;
	/// Gets the facility's training capacity.
	int getTrainingFacilities() const;
	/// Gets the maximum allowed number of facilities per base.
	int getMaxAllowedPerBase() const;
	/// Gets the facility's mana recovery rate.
	int getManaRecoveryPerDay() const { return _manaRecoveryPerDay; }
	/// Gets the facility's health recovery rate.
	int getHealthRecoveryPerDay() const { return _healthRecoveryPerDay; }
	/// Gets the facility's bonus to wound healing.
	float getSickBayAbsoluteBonus() const { return _sickBayAbsoluteBonus; }
	/// Gets the facility's bonus to wound healing (as percentage of max hp of the soldier).
	float getSickBayRelativeBonus() const { return _sickBayRelativeBonus; }
	/// Gets the prison type.
	int getPrisonType() const;
	/// Gets the action type to perform on right click.
	int getRightClickActionType() const;
	/// Gets the vertical levels for this facility map generation.
	const std::vector<VerticalLevel> &getVerticalLevels() const;
	/// Gets the facility left behind when this one is sold
	const std::vector<const RuleBaseFacility*> &getLeavesBehindOnSell() const { return _leavesBehindOnSell; }
	/// Gets how long facilities left behind when this one is sold should take to build
	int getRemovalTime() const;
	/// Gets whether or not this facility can be built over by other ones
	bool getCanBeBuiltOver() const;
	/// Check if a given facility `fac` can be replaced by this facility.
	BasePlacementErrors getCanBuildOverOtherFacility(const RuleBaseFacility* fac) const;
	/// Gets which facilities are allowed to be replaced by this building
	const std::vector<const RuleBaseFacility*> &getBuildOverFacilities() const { return _buildOverFacilities; }
	/// Gets a list of which tiles are used to place items stored in this facility
	const std::vector<Position> &getStorageTiles() const;
	/// Gets the ruleset for the destroyed version of this facility.
	const RuleBaseFacility* getDestroyedFacility() const;
};

}

namespace YAML
{
	template<>
	struct convert<OpenXcom::CraftOption>
	{
		static Node encode(const OpenXcom::CraftOption& crOpt)
		{
			Node node;
			node.SetStyle(EmitterStyle::Flow);
			node["x"] = crOpt.x;
			node["y"] = crOpt.y;
			node["min"] = crOpt.min;
			node["max"] = crOpt.max;
			node["hide"] = crOpt.hide;
			node["group"] = crOpt.group;
			node["subset"] = crOpt.subset;
			return node;
		}

		static bool decode(const Node& node, OpenXcom::CraftOption& crOpt)
		{
			if (!node.IsMap())
				return false;

			crOpt.x = node["x"].as<int>(2);
			crOpt.y = node["y"].as<int>(-4);
			crOpt.min = node["min"].as<int>(0);
			crOpt.max = node["max"].as<int>(0);
			crOpt.hide = node["hide"].as<bool>(false);
			crOpt.group = node["group"].as<int>(0);
			crOpt.subset = node["subset"].as<int>(0);

			if (crOpt.min > crOpt.max) return false;
			return true;
		}
	};
}
