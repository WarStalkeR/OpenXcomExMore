#pragma once
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
#include <string>
#include <yaml-cpp/yaml.h>
#include "ModScript.h"

namespace OpenXcom
{

/**
 * Starting base data set used when selecting initial starting base
 */
struct RuleStartingBaseSet
{
	/// Base set name. Duplicates will be overwritten.
	std::string Name;
	/// Base data that used by default and as fallback.
	YAML::Node BaseDefault;
	/// Base data that used on a Beginner difficulty.
	YAML::Node BaseBeginner;
	/// Base data that used on an Experienced difficulty.
	YAML::Node BaseExperienced;
	/// Base data that used on a Veteran difficulty.
	YAML::Node BaseVeteran;
	/// Base data that used on a Genius difficulty.
	YAML::Node BaseGenius;
	/// Base data that used on a Superhuman difficulty.
	YAML::Node BaseSuperhuman;

	/// Default constructor.
	RuleStartingBaseSet(const std::string &name);
	/// Constructor that allows to create a new starting base data set from existing data.
	RuleStartingBaseSet(std::string setName, YAML::Node dataDefault, YAML::Node dataBeginner,
		YAML::Node dataExperienced, YAML::Node dataVeteran, YAML::Node dataGenius, YAML::Node dataSuperhuman);

	/// Loads item data from YAML.
	void load(const YAML::Node& node);
	/// Cross link with other rules.
	void afterLoad(const Mod* mod);
};

} //namespace OpenXcom
