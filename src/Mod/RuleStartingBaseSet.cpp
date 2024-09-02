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
 * Loads item data from YAML.
 * @param node Node with data.
 */
void RuleStartingBaseSet::load(const YAML::Node& node)
{
	if (const YAML::Node &parent = node["refNode"])
	{
		load(parent);
	}

	auto loadStartingSetBase = [&](const YAML::Node &srcRef, YAML::Node &destRef)
	{
		if (srcRef && srcRef.IsMap())
		{
			for (YAML::const_iterator i = srcRef.begin(); i != srcRef.end(); ++i)
			{
				destRef[i->first.as<std::string>()] = YAML::Node(i->second);
			}
		}
	};

	loadStartingSetBase(node["baseDefault"], BaseDefault);
	loadStartingSetBase(node["baseBeginner"], BaseBeginner);
	loadStartingSetBase(node["baseExperienced"], BaseExperienced);
	loadStartingSetBase(node["baseVeteran"], BaseVeteran);
	loadStartingSetBase(node["baseGenius"], BaseGenius);
	loadStartingSetBase(node["baseSuperhuman"], BaseSuperhuman);

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
	if (BaseDefault.IsNull() && !BaseBeginner.IsNull())
		BaseDefault = YAML::Clone(BaseBeginner);
	else if (BaseDefault.IsNull() && !BaseExperienced.IsNull())
		BaseDefault = YAML::Clone(BaseExperienced);
	else if (BaseDefault.IsNull() && !BaseVeteran.IsNull())
		BaseDefault = YAML::Clone(BaseVeteran);
	else if (BaseDefault.IsNull() && !BaseGenius.IsNull())
		BaseDefault = YAML::Clone(BaseGenius);
	else if (BaseDefault.IsNull() && !BaseSuperhuman.IsNull())
		BaseDefault = YAML::Clone(BaseSuperhuman);
}

} //namespace OpenXcom
