/*
 * Copyright 2010-2024 OpenXcom Developers.
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
#include "RuleStartingBaseSet.h"
#include "../Engine/Exception.h"
#include "Mod.h"

namespace OpenXcom
{

/**
 * Default constructor (requires name).
 * @param unique name of the starting base set.
 */
RuleStartingBaseSet::RuleStartingBaseSet(const std::string &name) : Name(name),
	BaseDefault(YAML::Node()), BaseBeginner(YAML::Node()), BaseExperienced(YAML::Node()),
	BaseVeteran(YAML::Node()), BaseGenius(YAML::Node()), BaseSuperhuman(YAML::Node())
{

}

/**
 * Constructor that allows to create a new starting base data set from existing data.
 * @param unique name of the starting base set.
 * @param raw YAML data for default/fallback starting base.
 * @param raw YAML data for Beginner difficulty starting base.
 * @param raw YAML data for Experienced difficulty starting base.
 * @param raw YAML data for Veteran difficulty starting base.
 * @param raw YAML data for Genius difficulty starting base.
 * @param raw YAML data for Superhuman difficulty starting base.
 */
RuleStartingBaseSet::RuleStartingBaseSet(std::string setName, YAML::Node dataDefault, YAML::Node dataBeginner,
	YAML::Node dataExperienced, YAML::Node dataVeteran, YAML::Node dataGenius, YAML::Node dataSuperhuman)
{
	Name = setName;

	// If empty, any non-default value will be assigned.
	BaseDefault = dataDefault;

	// Keep it safe, in case if optional values aren't assigned
	if (dataBeginner && !dataBeginner.IsNull()) BaseBeginner = dataBeginner;
	if (dataExperienced && !dataExperienced.IsNull()) BaseExperienced = dataExperienced;
	if (dataVeteran && !dataVeteran.IsNull()) BaseVeteran = dataVeteran;
	if (dataGenius && !dataGenius.IsNull()) BaseGenius = dataGenius;
	if (dataSuperhuman && !dataSuperhuman.IsNull()) BaseSuperhuman = dataSuperhuman;

	// Empty data not allowed, since constructor has no default layouts
	if (BaseDefault.IsNull() && !BaseBeginner.IsNull()) BaseDefault = BaseBeginner;
	else if (BaseDefault.IsNull() && !BaseExperienced.IsNull()) BaseDefault = BaseExperienced;
	else if (BaseDefault.IsNull() && !BaseVeteran.IsNull()) BaseDefault = BaseVeteran;
	else if (BaseDefault.IsNull() && !BaseGenius.IsNull()) BaseDefault = BaseGenius;
	else if (BaseDefault.IsNull() && !BaseSuperhuman.IsNull()) BaseDefault = BaseSuperhuman;
	else throw Exception("Staring base set contains no data!");
}

/**
 * Loads item data from YAML.
 * @param node Node with data.
 */
void RuleStartingBaseSet::load(const YAML::Node& node)
{
	if (const YAML::Node &parent = node["refNode"])
	{
		load(parent);
	}

	BaseDefault = node["baseDefault"].as<YAML::Node>(BaseDefault);
	BaseBeginner = node["baseBeginner"].as<YAML::Node>(BaseBeginner);
	BaseExperienced = node["baseExperienced"].as<YAML::Node>(BaseExperienced);
	BaseVeteran = node["baseVeteran"].as<YAML::Node>(BaseVeteran);
	BaseGenius = node["baseGenius"].as<YAML::Node>(BaseGenius);
	BaseSuperhuman = node["baseSuperhuman"].as<YAML::Node>(BaseSuperhuman);

	// Empty data not allowed, since constructor has no default layouts
	if (BaseDefault.IsNull() && BaseBeginner.IsNull() && BaseExperienced.IsNull() &&
		BaseVeteran.IsNull() && BaseGenius.IsNull() && BaseSuperhuman.IsNull())
	{
		throw Exception("Staring base set contains no data!");
	}
}

/**
 * Cross link with other rules.
 */
void RuleStartingBaseSet::afterLoad(const Mod* mod)
{
	// Always assign fallback layout from first existing layout
	if (BaseDefault.IsNull() && !BaseBeginner.IsNull()) BaseDefault = BaseBeginner;
	else if (BaseDefault.IsNull() && !BaseExperienced.IsNull()) BaseDefault = BaseExperienced;
	else if (BaseDefault.IsNull() && !BaseVeteran.IsNull()) BaseDefault = BaseVeteran;
	else if (BaseDefault.IsNull() && !BaseGenius.IsNull()) BaseDefault = BaseGenius;
	else if (BaseDefault.IsNull() && !BaseSuperhuman.IsNull()) BaseDefault = BaseSuperhuman;
}

} //namespace OpenXcom
