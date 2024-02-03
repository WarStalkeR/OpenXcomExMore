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

#include <sstream>
#include "ArticleStateBaseFacility.h"
#include "../Mod/ArticleDefinition.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleBaseFacility.h"
#include "../Engine/Game.h"
#include "../Engine/Font.h"
#include "../Engine/Palette.h"
#include "../Engine/Surface.h"
#include "../Engine/SurfaceSet.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Unicode.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextList.h"

namespace OpenXcom
{

	ArticleStateBaseFacility::ArticleStateBaseFacility(ArticleDefinitionBaseFacility *defs, std::shared_ptr<ArticleCommonState> state) : ArticleState(defs->id, std::move(state))
	{
		RuleBaseFacility *facility = _game->getMod()->getBaseFacility(defs->id, true);

		// add screen elements
		_txtTitle = new Text(200, 17, 10, 24);

		// Set palette
		setStandardPalette("PAL_BASESCAPE");

		ArticleState::initLayout();

		// add other elements
		add(_txtTitle);

		// Set up objects
		_game->getMod()->getSurface("BACK09.SCR")->blitNShade(_bg, 0, 0);
		_btnOk->setColor(Palette::blockOffset(4));
		_btnPrev->setColor(Palette::blockOffset(4));
		_btnNext->setColor(Palette::blockOffset(4));
		_btnInfo->setColor(Palette::blockOffset(4));
		_btnInfo->setVisible(_game->getMod()->getShowPediaInfoButton());

		_txtTitle->setColor(Palette::blockOffset(13)+10);
		_txtTitle->setBig();
		_txtTitle->setText(tr(defs->getTitleForPage(_state->current_page)));

		// build preview image
		int tile_size = 32;
		_image = new Surface(
			tile_size * std::max(1, Mod::PEDIA_FACILITY_RENDER_PARAMETERS[0]),
			tile_size * std::max(1, Mod::PEDIA_FACILITY_RENDER_PARAMETERS[1]),
			232 + Mod::PEDIA_FACILITY_RENDER_PARAMETERS[2],
			16 + Mod::PEDIA_FACILITY_RENDER_PARAMETERS[3]);
		add(_image);

		SurfaceSet *graphic = _game->getMod()->getSurfaceSet("BASEBITS.PCK");
		Surface *frame;
		int x_offset, y_offset;
		int x_pos, y_pos;
		int num;

		// calculate preview offset
		x_offset = (tile_size * std::max(0, Mod::PEDIA_FACILITY_RENDER_PARAMETERS[0] - facility->getSize())) / 2;
		y_offset = (tile_size * std::max(0, Mod::PEDIA_FACILITY_RENDER_PARAMETERS[1] - facility->getSize())) / 2;

		// render build preview
		num = 0;
		y_pos = y_offset;
		for (int y = 0; y < facility->getSize(); ++y)
		{
			x_pos = x_offset;
			for (int x = 0; x < facility->getSize(); ++x)
			{
				frame = graphic->getFrame(facility->getSpriteShape() + num);
				frame->blitNShade(_image, x_pos, y_pos);

				if (facility->getSpriteEnabled())
				{
					frame = graphic->getFrame(facility->getSpriteFacility() + num);
					frame->blitNShade(_image, x_pos, y_pos);
				}

				x_pos += tile_size;
				num++;
			}
			y_pos += tile_size;
		}

		_txtInfo = new Text(300, 90, 10, 104);
		add(_txtInfo);

		_txtInfo->setColor(Palette::blockOffset(13)+10);
		_txtInfo->setSecondaryColor(Palette::blockOffset(13));
		_txtInfo->setWordWrap(true);
		_txtInfo->setScrollable(true);
		_txtInfo->setText(tr(defs->getTextForPage(_state->current_page)));

		int row = 0;
		const int maxRows = Mod::PEDIA_FACILITY_ROWS_CUTOFF;
		const bool lockedStats = Mod::PEDIA_FACILITY_LOCKED_STATS;
		const auto& colOffset = Mod::PEDIA_FACILITY_COL_OFFSET;
		const int colStat = (lockedStats ? 130 : 125) + colOffset;
		const int colValue = (lockedStats ? 60 : 55) - colOffset;
		_lstInfo = new TextList(colStat + colValue + 3, 48, 10, 41);
		add(_lstInfo);

		_lstInfo->setColor(Palette::blockOffset(13)+10);
		_lstInfo->setColumns(2, colStat, colValue);
		_lstInfo->setScrolling(!lockedStats, 0);
		_lstInfo->setWordWrap(!lockedStats);
		_lstInfo->setCondensed(true);
		_lstInfo->setDot(true);

		_lstInfo->addRow(2, tr("STR_CONSTRUCTION_TIME").c_str(), tr("STR_DAY", facility->getBuildTime()).c_str());
		_lstInfo->setCellColor(row, 1, Palette::blockOffset(13)+0);
		row++;

		std::ostringstream ts;
		std::ostringstream ss;

		ts << tr("STR_CONSTRUCTION_COST");
		ss << Unicode::formatFunding(facility->getBuildCost());
		addToStatList(&ts, &ss, colStat, colValue, row);
		row++;

		ts.str(""); ts.clear(); ss.str(""); ss.clear();
		ts << tr("STR_MAINTENANCE_COST");
		ss << Unicode::formatFunding(facility->getMonthlyCost());
		addToStatList(&ts, &ss, colStat, colValue, row);
		row++;

		if (facility->getCrafts() > 0)
		{
			if (!lockedStats || (lockedStats && row < maxRows))
			{
				ts.str(""); ts.clear(); ss.str(""); ss.clear();
				ts << tr("STR_HANGAR_CRAFT_CAP");
				ss << facility->getCrafts();
				addToStatList(&ts, &ss, colStat, colValue, row);
				row++;
			}

			if (!lockedStats || (lockedStats && row < maxRows))
			{
				ts.str(""); ts.clear(); ss.str(""); ss.clear();
				ts << tr("STR_HANGAR_CRAFT_SLOTS");
				if (facility->getOptionGroups().size() > 0 &&
					(size_t)facility->getCraftGroupSum() == facility->getCraftOptions().size())
				{
					int optionIt = 0;
					auto optionGroupsIt = facility->getOptionGroups().begin();
					while (optionGroupsIt != facility->getOptionGroups().end())
					{
						std::ostringstream sss;
						for (int i = 0; i < *optionGroupsIt; ++i)
						{
							const int& slotSize = facility->getCraftOptions().at(optionIt).max;
							const std::string sizeClass = _game->getMod()->getCraftClassFromSize(slotSize);
							if (sizeClass != _game->getMod()->getCraftSizeClassMap()->begin()->second)
							{
								// broken slot entry is hidden in ufopaedia, but shown in analysis
								if (!sss.str().empty()) sss << "~";
								const std::string slotClass = sizeClass.empty() ?
									std::to_string(slotSize) : tr(sizeClass + "_UC").c_str();
								sss << slotClass;
							}
							optionIt++;
						}
						if (!sss.str().empty())
						{
							if (!ss.str().empty()) ss << ", ";
							ss << sss.str().c_str();
						}
						if ((size_t)optionIt >= facility->getCraftOptions().size())
							optionIt = facility->getCraftOptions().size() - 1;
						++optionGroupsIt;
					}
				}
				else
				{
					const int maxCraftOptions = (size_t)facility->getCrafts() >
						facility->getCraftOptions().size() ?
						facility->getCrafts() :
						facility->getCraftOptions().size();
					for (int i = 0; i < maxCraftOptions; ++i)
					{
						const int& slotSize = facility->getCraftOptions().size() > (size_t)i ?
							facility->getCraftOptions().at(i).max : 0;
						const std::string sizeClass = _game->getMod()->getCraftClassFromSize(slotSize);
						if (sizeClass != _game->getMod()->getCraftSizeClassMap()->begin()->second)
						{
							// broken slot entry is hidden in ufopaedia, but shown in analysis
							const std::string slotClass = sizeClass.empty() ?
								std::to_string(slotSize) : tr(sizeClass + "_UC").c_str();
							if (!ss.str().empty()) ss << ", ";
							ss << slotClass;
						}
					}
				}
				addToStatList(&ts, &ss, colStat, colValue, row);
				row++;
			}
		}

		if (facility->getDefenseValue() > 0)
		{
			if (!lockedStats || (lockedStats && row < maxRows))
			{
				ts.str(""); ts.clear(); ss.str(""); ss.clear();
				ts << tr("STR_DEFENSE_VALUE");
				ss << facility->getDefenseValue();
				addToStatList(&ts, &ss, colStat, colValue, row);
				row++;
			}

			if (!lockedStats || (lockedStats && row < maxRows))
			{
				ts.str(""); ts.clear(); ss.str(""); ss.clear();
				ts << tr("STR_HIT_RATIO");
				ss << Unicode::formatPercentage(facility->getHitRatio());
				addToStatList(&ts, &ss, colStat, colValue, row);
				row++;
			}
		}
		centerAllSurfaces();
	}

	ArticleStateBaseFacility::~ArticleStateBaseFacility()
	{}

	int ArticleStateBaseFacility::getTextWidth(std::ostringstream *strStream) const
	{
		int textWidth = 0;
		UString text = Unicode::convUtf8ToUtf32(strStream->str());

		for (UString::const_iterator i = text.begin(); i < text.end(); ++i)
		{
			textWidth += _txtInfo->getFont()->getCharSize(*i).w;
		}

		return textWidth;
	}

	void ArticleStateBaseFacility::addToStatList(std::ostringstream *sStream, std::ostringstream *vStream, int colStat, int colValue, int row)
	{
		int dynColValue, dynColStat;
		int statSize = getTextWidth(sStream) + 10;
		int valueSize = getTextWidth(vStream) + 1;
		dynColStat = std::max(statSize, colStat);
		dynColStat = std::min(dynColStat, colStat);
		dynColValue = colStat + colValue - dynColStat;
		dynColValue = std::max(dynColValue, std::max(colValue, valueSize));
		dynColStat = colStat + colValue - dynColValue;
		if (dynColStat < statSize)
		{
			dynColStat = statSize;
			dynColValue = colStat + colValue - dynColStat;
		}
		_lstInfo->setColumns(2, dynColStat, dynColValue);
		_lstInfo->addRow(2, sStream->str().c_str(), vStream->str().c_str());
		_lstInfo->setCellColor(row, 1, Palette::blockOffset(13)+0);
	}
}
