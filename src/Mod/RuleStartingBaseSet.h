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
#include "../Engine/Yaml.h"
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
	YAML::YamlString BaseDefault;
	/// Base data that used on a Beginner difficulty.
	YAML::YamlString BaseBeginner;
	/// Base data that used on an Experienced difficulty.
	YAML::YamlString BaseExperienced;
	/// Base data that used on a Veteran difficulty.
	YAML::YamlString BaseVeteran;
	/// Base data that used on a Genius difficulty.
	YAML::YamlString BaseGenius;
	/// Base data that used on a Superhuman difficulty.
	YAML::YamlString BaseSuperhuman;

	/// Default constructor.
	RuleStartingBaseSet(const std::string &name);
	/// Deletes a RuleStartingBaseSet.
	~RuleStartingBaseSet() = default;

	/// Loads item data from YAML.
	void load(const YAML::YamlNodeReader& node);
	/// Cross link with other rules.
	void afterLoad(const Mod* mod);
};

} //namespace OpenXcom
