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
	BaseDefault(YAML::YamlString()), BaseBeginner(YAML::YamlString()),
	BaseExperienced(YAML::YamlString()), BaseVeteran(YAML::YamlString()),
	BaseGenius(YAML::YamlString()), BaseSuperhuman(YAML::YamlString())
{

}

/**
 * Loads item data from YAML.
 * @param node Node with data.
 */
void RuleStartingBaseSet::load(const YAML::YamlNodeReader& node)
{
	const auto& reader = node.useIndex();
	if (const auto& parent = reader["refNode"])
	{
		load(parent);
	}

	auto loadStartingSetBase = [&](const YAML::YamlNodeReader &srcRef, YAML::YamlString &destRef)
	{
		if (srcRef && srcRef.isMap())
		{
			destRef = srcRef.emitDescendants();
		}
	};

	loadStartingSetBase(node["baseDefault"], BaseDefault);
	loadStartingSetBase(node["baseBeginner"], BaseBeginner);
	loadStartingSetBase(node["baseExperienced"], BaseExperienced);
	loadStartingSetBase(node["baseVeteran"], BaseVeteran);
	loadStartingSetBase(node["baseGenius"], BaseGenius);
	loadStartingSetBase(node["baseSuperhuman"], BaseSuperhuman);

	// Empty data not allowed, since constructor has no default layouts
	if (BaseDefault.yaml.empty() && BaseBeginner.yaml.empty() &&
		BaseExperienced.yaml.empty() && BaseVeteran.yaml.empty() &&
		BaseGenius.yaml.empty() && BaseSuperhuman.yaml.empty())
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
	if (BaseDefault.yaml.empty() && !BaseBeginner.yaml.empty())
		BaseDefault.yaml = BaseBeginner.yaml;
	else if (BaseDefault.yaml.empty() && !BaseExperienced.yaml.empty())
		BaseDefault.yaml = BaseExperienced.yaml;
	else if (BaseDefault.yaml.empty() && !BaseVeteran.yaml.empty())
		BaseDefault.yaml = BaseVeteran.yaml;
	else if (BaseDefault.yaml.empty() && !BaseGenius.yaml.empty())
		BaseDefault.yaml = BaseGenius.yaml;
	else if (BaseDefault.yaml.empty() && !BaseSuperhuman.yaml.empty())
		BaseDefault.yaml = BaseSuperhuman.yaml;
}

} //namespace OpenXcom
