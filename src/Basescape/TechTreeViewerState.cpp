/*
 * Copyright 2010-2015 OpenXcom Developers.
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
#include "TechTreeViewerState.h"
#include "TechTreeSelectState.h"
#include "../Engine/Action.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Mod/RuleArcScript.h"
#include "../Mod/RuleBaseFacility.h"
#include "../Mod/RuleCraft.h"
#include "../Mod/RuleCountry.h"
#include "../Mod/RuleEventScript.h"
#include "../Mod/RuleInterface.h"
#include "../Mod/RuleItem.h"
#include "../Mod/RuleManufacture.h"
#include "../Mod/RuleMissionScript.h"
#include "../Mod/RuleRegion.h"
#include "../Mod/RuleResearch.h"
#include "../Mod/RuleSoldierTransformation.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Engine/Unicode.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextButton.h"
#include "../Interface/TextList.h"
#include "../Savegame/AlienStrategy.h"
#include "../Savegame/Base.h"
#include "../Savegame/Region.h"
#include "../Savegame/Country.h"
#include "../Savegame/SavedGame.h"
#include <algorithm>
#include <unordered_set>

namespace OpenXcom
{

/**
 * Initializes all the elements on the UI.
 */
TechTreeViewerState::TechTreeViewerState(const RuleResearch *r, const RuleManufacture *m, const RuleBaseFacility *f, const RuleCraft *c, const RuleArcScript *as, const RuleEventScript *es, const RuleMissionScript *ms)
{
	if (r != 0)
	{
		_selectedTopic = r->getName();
		_selectedFlag = TTV_RESEARCH;
	}
	else if (m != 0)
	{
		_selectedTopic = m->getName();
		_selectedFlag = TTV_MANUFACTURING;
	}
	else if (f != 0)
	{
		_selectedTopic = f->getType();
		_selectedFlag = TTV_FACILITIES;
	}
	else if (c != 0)
	{
		_selectedTopic = c->getType();
		_selectedFlag = TTV_CRAFTS;
	}
	else if (as != 0)
	{
		_selectedTopic = as->getType();
		_selectedFlag = TTV_ARCS;
	}
	else if (es != 0)
	{
		_selectedTopic = es->getType();
		_selectedFlag = TTV_EVENTS;
	}
	else if (ms != 0)
	{
		_selectedTopic = ms->getType();
		_selectedFlag = TTV_MISSIONS;
	}

	// Create objects
	_window = new Window(this, 320, 200, 0, 0);
	_txtTitle = new Text(304, 17, 8, 7);
	_txtSelectedTopic = new Text(204, 9, 8, 24);
	_txtProgress = new Text(100, 9, 212, 24);
	_txtCostIndicator = new Text(100, 9, 16, 32); // experimental cost indicator
	_lstLeft = new TextList(132, 128, 8, 40);
	_lstRight = new TextList(132, 128, 164, 40);
	_lstFull = new TextList(288, 128, 8, 40);
	_btnNew = new TextButton(148, 16, 8, 176);
	_btnOk = new TextButton(148, 16, 164, 176);

	// Set palette
	setInterface("techTreeViewer");

	_purple = _game->getMod()->getInterface("techTreeViewer")->getElement("list")->color;
	_pink = _game->getMod()->getInterface("techTreeViewer")->getElement("list")->color2;
	_blue = _game->getMod()->getInterface("techTreeViewer")->getElement("list")->border;
	_white = _game->getMod()->getInterface("techTreeViewer")->getElement("listExtended")->color;
	_gold = _game->getMod()->getInterface("techTreeViewer")->getElement("listExtended")->color2;
	_grey = _game->getMod()->getInterface("techTreeViewer")->getElement("listExtended")->border;

	add(_window, "window", "techTreeViewer");
	add(_txtTitle, "text", "techTreeViewer");
	add(_txtSelectedTopic, "text", "techTreeViewer");
	add(_txtProgress, "text", "techTreeViewer");
	add(_txtCostIndicator, "text", "techTreeViewer");
	add(_lstLeft, "list", "techTreeViewer");
	add(_lstRight, "list", "techTreeViewer");
	add(_lstFull, "list", "techTreeViewer");
	add(_btnNew, "button", "techTreeViewer");
	add(_btnOk, "button", "techTreeViewer");

	centerAllSurfaces();

	// Set up reference values
	_save = _game->getSavedGame();
	_currMonth = _save->getMonthsPassed();
	_currScore = _save->getCurrentScore(_currMonth);
	_currDiff = _save->getDifficulty();
	_currFunds = _save->getFunds();

	// Set up objects
	setWindowBackground(_window, "techTreeViewer");

	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_CENTER);
	if (Options::oxceTechTreeDataView)
		_txtTitle->setText(tr("STR_GAME_DATA_VIEWER"));
	else _txtTitle->setText(tr("STR_TECH_TREE_VIEWER"));

	_txtSelectedTopic->setText(tr("STR_TOPIC").arg(""));

	_lstLeft->setColumns(1, 132);
	_lstLeft->setSelectable(true);
	_lstLeft->setBackground(_window);
	_lstLeft->setWordWrap(true);
	_lstLeft->onMouseClick((ActionHandler)&TechTreeViewerState::onSelectLeftTopic);

	_lstRight->setColumns(1, 132);
	_lstRight->setSelectable(true);
	_lstRight->setBackground(_window);
	_lstRight->setWordWrap(true);
	_lstRight->onMouseClick((ActionHandler)&TechTreeViewerState::onSelectRightTopic);

	_lstFull->setColumns(1, 288);
	_lstFull->setSelectable(true);
	_lstFull->setBackground(_window);
	_lstFull->setWordWrap(true);
	_lstFull->onMouseClick((ActionHandler)&TechTreeViewerState::onSelectFullTopic);

	_btnNew->setText(tr("STR_SELECT_TOPIC"));
	_btnNew->onMouseClick((ActionHandler)&TechTreeViewerState::btnNewClick);
	_btnNew->onKeyboardPress((ActionHandler)&TechTreeViewerState::btnNewClick, Options::keyToggleQuickSearch);

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&TechTreeViewerState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&TechTreeViewerState::btnOkClick, Options::keyCancel);
	_btnOk->onKeyboardPress((ActionHandler)&TechTreeViewerState::btnBackClick, SDLK_BACKSPACE);

	if (Options::oxceDisableTechTreeViewer)
	{
		_txtTitle->setHeight(_txtTitle->getHeight() * 9);
		_txtTitle->setWordWrap(true);
		_txtTitle->setText(tr("STR_THIS_FEATURE_IS_DISABLED_1"));
		_txtSelectedTopic->setVisible(false);
		_txtProgress->setVisible(false);
		_txtCostIndicator->setVisible(false);
		_lstLeft->setVisible(false);
		_lstRight->setVisible(false);
		_lstFull->setVisible(false);
		_btnNew->setVisible(false);
		return;
	}

	int discoveredSum = 0;
	// pre-calculate globally
	for (auto* discoveredResearchRule : _game->getSavedGame()->getDiscoveredResearch())
	{
		_alreadyAvailableResearch.insert(discoveredResearchRule->getName());
		discoveredSum += discoveredResearchRule->getCost();
	}
	for (auto& info : _game->getSavedGame()->getResearchRuleStatusRaw())
	{
		if (info.second == RuleResearch::RESEARCH_STATUS_DISABLED)
		{
			auto* rr = _game->getMod()->getResearch(info.first, false);
			if (rr)
			{
				_disabledResearch.insert(rr->getName());
			}
		}
	}

	int totalSum = 0;
	RuleResearch *resRule = 0;
	for (auto& resType : _game->getMod()->getResearchList())
	{
		resRule = _game->getMod()->getResearch(resType);
		if (resRule != 0)
		{
			totalSum += resRule->getCost();
		}
	}

	RuleManufacture *manRule = 0;
	for (auto& manuf : _game->getMod()->getManufactureList())
	{
		manRule = _game->getMod()->getManufacture(manuf);
		if (_game->getSavedGame()->isResearched(manRule->getRequirements()))
		{
			_alreadyAvailableManufacture.insert(manRule->getName());
		}
	}

	RuleBaseFacility *facRule = 0;
	for (auto& facType : _game->getMod()->getBaseFacilitiesList())
	{
		facRule = _game->getMod()->getBaseFacility(facType);
		if (_game->getSavedGame()->isResearched(facRule->getRequirements()))
		{
			_alreadyAvailableFacilities.insert(facRule->getType());
		}
	}

	RuleItem *itemRule = 0;
	for (auto& itemType : _game->getMod()->getItemsList())
	{
		itemRule = _game->getMod()->getItem(itemType);
		if (!itemRule->getRequirements().empty() || !itemRule->getBuyRequirements().empty())
		{
			_protectedItems.insert(itemRule->getType());
			if (_game->getSavedGame()->isResearched(itemRule->getRequirements()) && _game->getSavedGame()->isResearched(itemRule->getBuyRequirements()))
			{
				_alreadyAvailableItems.insert(itemRule->getType());
			}
		}
	}

	RuleCraft *craftRule = 0;
	for (auto& craftType : _game->getMod()->getCraftsList())
	{
		craftRule = _game->getMod()->getCraft(craftType);
		if (_game->getSavedGame()->isResearched(craftRule->getRequirements()))
		{
			_alreadyAvailableCrafts.insert(craftRule->getType());
		}
	}

	if (Options::oxceTechTreeDataView)
	{
		const RuleArcScript *arcScriptRule = 0;
		for (auto& arcScript : *_game->getMod()->getArcScriptList())
		{
			arcScriptRule = _game->getMod()->getArcScript(arcScript);
			if (Options::debug || isPossibleArc(arcScriptRule))
			{
				_listArcScripts.insert(arcScript);
			}
		}

		const RuleEventScript *eventScriptRule = 0;
		for (auto& eventScript : *_game->getMod()->getEventScriptList())
		{
			eventScriptRule = _game->getMod()->getEventScript(eventScript);
			if (Options::debug || isPossibleEvent(eventScriptRule))
			{
				_listEventScripts.insert(eventScript);
			}
		}

		const RuleMissionScript *missionScriptRule = 0;
		for (auto& missionScript : *_game->getMod()->getMissionScriptList())
		{
			missionScriptRule = _game->getMod()->getMissionScript(missionScript);
			if (Options::debug || isPossibleMission(missionScriptRule))
			{
				_listMissionScripts.insert(missionScript);
			}
		}
	}

	_txtProgress->setAlign(ALIGN_RIGHT);
	_txtProgress->setText(tr("STR_RESEARCH_PROGRESS").arg(discoveredSum * 100 / totalSum));
}

/**
 *
 */
TechTreeViewerState::~TechTreeViewerState()
{
}

/**
* Initializes the screen (fills the lists).
*/
void TechTreeViewerState::init()
{
	State::init();

	if (!Options::oxceDisableTechTreeViewer)
	{
		initLists();
	}
}

/**
* Returns to the previous screen.
* @param action Pointer to an action.
*/
void TechTreeViewerState::btnOkClick(Action *)
{
	_game->popState();
}

/**
 * Navigates to the previous topic from the browsing history.
 * @param action Pointer to an action.
 */
void TechTreeViewerState::btnBackClick(Action *)
{
	if (!_history.empty())
	{
		_selectedFlag = _history.back().second;
		_selectedTopic = _history.back().first;
		_history.pop_back();
		initLists();
	}
}

/**
* Opens the Select Topic screen.
* @param action Pointer to an action.
*/
void TechTreeViewerState::btnNewClick(Action *)
{
	_game->pushState(new TechTreeSelectState(this));
}

/**
 * Shows the filtered topics.
 */
void TechTreeViewerState::initLists()
{
	// Set topic name
	{
		std::ostringstream ss;
		if (_selectedFlag > TTV_CRAFTS)
			strPush(ss, _selectedTopic); // Data Viewer entries only
		else ss << tr(_selectedTopic);

		if (_selectedFlag == TTV_MANUFACTURING)
		{
			ss << tr("STR_M_FLAG");
		}
		else if (_selectedFlag == TTV_FACILITIES)
		{
			ss << tr("STR_F_FLAG");
		}
		else if (_selectedFlag == TTV_ITEMS)
		{
			ss << tr("STR_I_FLAG");
		}
		else if (_selectedFlag == TTV_CRAFTS)
		{
			ss << tr("STR_C_FLAG");
		}
		else if (_selectedFlag == TTV_ARCS)
		{
			ss << tr("STR_AS_FLAG");
		}
		else if (_selectedFlag == TTV_EVENTS)
		{
			ss << tr("STR_ES_FLAG");
		}
		else if (_selectedFlag == TTV_MISSIONS)
		{
			ss << tr("STR_MS_FLAG");
		}
		_txtSelectedTopic->setText(tr("STR_TOPIC").arg(ss.str()));
		_txtCostIndicator->setText("");
	}

	// reset
	_leftTopics.clear();
	_rightTopics.clear();
	_leftFlags.clear();
	_rightFlags.clear();
	_lstLeft->clearList();
	_lstRight->clearList();
	_lstFull->clearList();
	_lstLeft->setVisible(true);
	_lstRight->setVisible(true);
	_lstFull->setVisible(false);

	if (_selectedFlag == TTV_NONE)
	{
		return;
	}
	else if (_selectedFlag == TTV_RESEARCH)
	{
		handleResearchData();
	}
	else if (_selectedFlag == TTV_MANUFACTURING)
	{
		handleManufactureData();
	}
	else if (_selectedFlag == TTV_FACILITIES)
	{
		handleFacilityData();
	}
	else if (_selectedFlag == TTV_ITEMS)
	{
		handleItemData();
	}
	else if (_selectedFlag == TTV_CRAFTS)
	{
		handleCraftData();
	}
	else if (_selectedFlag == TTV_ARCS)
	{
		handleArcScript();
	}
	else if (_selectedFlag == TTV_EVENTS)
	{
		handleEventScript();
	}
	else if (_selectedFlag == TTV_MISSIONS)
	{
		handleMissionScript();
	}
}

/**
 * Handles, renders and organizes Research entry data.
 */
void TechTreeViewerState::handleResearchData()
{
	int row = 0;
	RuleResearch *rule = _game->getMod()->getResearch(_selectedTopic);
	if (rule == 0)
		return;

	// Cost indicator
	{
		std::ostringstream ss;
		int cost = rule->getCost();
		std::vector<std::pair<int, std::string>> symbol_values
				({{100, "#"}, {20, "="}, {5, "-"}});

		for (const auto& sym : symbol_values)
		{
			while (cost >= std::get<0>(sym))
			{
				cost -= std::get<0>(sym);
				ss << std::get<1>(sym);
			}
		}
		if (rule->getCost() < 10)
		{
			while (cost >= 1)
			{
				cost -= 1;
				ss << ".";
			}
		}
		_txtCostIndicator->setText(ss.str());
	}
	//

	const std::vector<std::string> &researchList = _game->getMod()->getResearchList();
	const std::vector<std::string> &manufactureList = _game->getMod()->getManufactureList();

	// 0. common pre-calculation
	const std::vector<const RuleResearch*>& reqs = rule->getRequirements();
	const std::vector<const RuleResearch*>& deps = rule->getDependencies();
	std::vector<std::string> unlockedBy;
	std::vector<std::string> disabledBy;
	std::vector<std::string> reenabledBy;
	std::vector<std::string> getForFreeFrom;
	std::vector<std::string> lookupOf;
	std::vector<std::string> requiredByResearch;
	std::vector<std::string> requiredByManufacture;
	std::vector<std::string> requiredByFacilities;
	std::vector<std::string> requiredByItems;
	std::vector<std::string> requiredByTransformations;
	std::vector<std::string> requiredByCrafts;
	std::vector<std::string> leadsTo;
	const std::vector<const RuleResearch*>& unlocks = rule->getUnlocked();
	const std::vector<const RuleResearch*>& disables = rule->getDisabled();
	const std::vector<const RuleResearch*>& reenables = rule->getReenabled();
	const std::vector<const RuleResearch*>& free = rule->getGetOneFree();
	auto& freeProtected = rule->getGetOneFreeProtected();

	for (auto& j : manufactureList)
	{
		RuleManufacture *temp = _game->getMod()->getManufacture(j);
		for (auto& i : temp->getRequirements())
		{
			if (i == rule)
			{
				requiredByManufacture.push_back(j);
			}
		}
	}

	for (auto& f : _game->getMod()->getBaseFacilitiesList())
	{
		RuleBaseFacility *temp = _game->getMod()->getBaseFacility(f);
		for (auto& i : temp->getRequirements())
		{
			if (i == rule->getName())
			{
				requiredByFacilities.push_back(f);
			}
		}
	}

	for (auto& item : _game->getMod()->getItemsList())
	{
		RuleItem *temp = _game->getMod()->getItem(item);
		for (auto& i : temp->getRequirements())
		{
			if (i == rule)
			{
				requiredByItems.push_back(item);
			}
		}
		for (auto& i : temp->getBuyRequirements())
		{
			if (i == rule)
			{
				requiredByItems.push_back(item);
			}
		}
	}

	for (auto& transf : _game->getMod()->getSoldierTransformationList())
	{
		RuleSoldierTransformation* temp = _game->getMod()->getSoldierTransformation(transf);
		for (auto& i : temp->getRequiredResearch())
		{
			if (i == rule->getName())
			{
				requiredByTransformations.push_back(transf);
			}
		}
	}

	for (auto& c : _game->getMod()->getCraftsList())
	{
		RuleCraft *temp = _game->getMod()->getCraft(c);
		for (auto& i : temp->getRequirements())
		{
			if (i == rule->getName())
			{
				requiredByCrafts.push_back(c);
			}
		}
	}

	for (auto& j : researchList)
	{
		RuleResearch *temp = _game->getMod()->getResearch(j);
		for (auto& i : temp->getUnlocked())
		{
			if (i == rule)
			{
				unlockedBy.push_back(j);
			}
		}
		for (auto& i : temp->getDisabled())
		{
			if (i == rule)
			{
				disabledBy.push_back(j);
			}
		}
		for (auto& i : temp->getReenabled())
		{
			if (i == rule)
			{
				reenabledBy.push_back(j);
			}
		}
		for (auto& i : temp->getGetOneFree())
		{
			if (i == rule)
			{
				getForFreeFrom.push_back(j);
			}
		}
		for (auto& itMap : temp->getGetOneFreeProtected())
		{
			for (auto& i : itMap.second)
			{
				if (i == rule)
				{
					getForFreeFrom.push_back(j);
				}
			}
		}
		if (!Mod::isEmptyRuleName(temp->getLookup()))
		{
			if (temp->getLookup() == rule->getName())
			{
				lookupOf.push_back(j);
			}
		}
		for (auto& i : temp->getRequirements())
		{
			if (i == rule)
			{
				requiredByResearch.push_back(j);
			}
		}
		for (auto& i : temp->getDependencies())
		{
			if (i == rule)
			{
				leadsTo.push_back(j);
			}
		}
	}

	// 1. item required
	if (rule->needItem())
	{
		if (rule->destroyItem())
		{
			_lstLeft->addRow(1, tr("STR_ITEM_DESTROYED").c_str());
		}
		else
		{
			_lstLeft->addRow(1, tr("STR_ITEM_REQUIRED").c_str());
		}
		_lstLeft->setRowColor(row, _blue);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;
		std::string itemName = tr(_selectedTopic);
		itemName.insert(0, "  ");
		_lstLeft->addRow(1, itemName.c_str());
		_lstLeft->setRowColor(row, getResearchColor(_selectedTopic));
		_leftTopics.push_back(_selectedTopic);
		_leftFlags.push_back(TTV_ITEMS);
		++row;
	}

	// 1b. requires services (from base facilities)
	if (rule->getRequireBaseFunc().any())
	{
		_lstLeft->addRow(1, tr("STR_SERVICES_REQUIRED").c_str());
		_lstLeft->setRowColor(row, _blue);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;
		for (const auto& reqServiceType : _game->getMod()->getBaseFunctionNames(rule->getRequireBaseFunc()))
		{
			std::string name = tr(reqServiceType);
			name.insert(0, "  ");
			_lstLeft->addRow(1, name.c_str());
			_lstLeft->setRowColor(row, _gold);
			_leftTopics.push_back("-");
			_leftFlags.push_back(TTV_NONE);
			++row;
		}
	}

	// 2. requires
	if (reqs.size() > 0)
	{
		_lstLeft->addRow(1, tr("STR_REQUIRES").c_str());
		_lstLeft->setRowColor(row, _blue);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;
		for (auto* i : reqs)
		{
			std::string name = tr(i->getName());
			name.insert(0, "  ");
			_lstLeft->addRow(1, name.c_str());
			_lstLeft->setRowColor(row, getResearchColor(i->getName()));
			_leftTopics.push_back(i->getName());
			_leftFlags.push_back(TTV_RESEARCH);
			++row;
		}
	}

	// 3. depends on
	if (deps.size() > 0)
	{
		_lstLeft->addRow(1, tr("STR_DEPENDS_ON").c_str());
		_lstLeft->setRowColor(row, _blue);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;
		for (auto* i : deps)
		{
			if (std::find(unlockedBy.begin(), unlockedBy.end(), i->getName()) != unlockedBy.end())
			{
				// if the same item is also in the "Unlocked by" section, skip it
				continue;
			}
			std::string name = tr(i->getName());
			name.insert(0, "  ");
			_lstLeft->addRow(1, name.c_str());
			_lstLeft->setRowColor(row, getResearchColor(i->getName()));
			_leftTopics.push_back(i->getName());
			_leftFlags.push_back(TTV_RESEARCH);
			++row;
		}
	}

	// 4a. unlocked by
	if (unlockedBy.size() > 0)
	{
		_lstLeft->addRow(1, tr("STR_UNLOCKED_BY").c_str());
		_lstLeft->setRowColor(row, _blue);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;
		for (const auto& res : unlockedBy)
		{
			std::string name = tr(res);
			name.insert(0, "  ");
			_lstLeft->addRow(1, name.c_str());
			_lstLeft->setRowColor(row, getResearchColor(res));
			_leftTopics.push_back(res);
			_leftFlags.push_back(TTV_RESEARCH);
			++row;
		}
	}

	// 4b. disabled by
	if (disabledBy.size() > 0)
	{
		_lstLeft->addRow(1, tr("STR_DISABLED_BY").c_str());
		_lstLeft->setRowColor(row, _blue);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;
		for (const auto& res : disabledBy)
		{
			std::string name = tr(res);
			name.insert(0, "  ");
			_lstLeft->addRow(1, name.c_str());
			_lstLeft->setRowColor(row, getResearchColor(res));
			_leftTopics.push_back(res);
			_leftFlags.push_back(TTV_RESEARCH);
			++row;
		}
	}

	// 4c. reenabled by
	if (reenabledBy.size() > 0)
	{
		_lstLeft->addRow(1, tr("STR_REENABLED_BY").c_str());
		_lstLeft->setRowColor(row, _blue);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;
		for (const auto& res : reenabledBy)
		{
			std::string name = tr(res);
			name.insert(0, "  ");
			_lstLeft->addRow(1, name.c_str());
			_lstLeft->setRowColor(row, getResearchColor(res));
			_leftTopics.push_back(res);
			_leftFlags.push_back(TTV_RESEARCH);
			++row;
		}
	}

	// 5. get for free from
	if (getForFreeFrom.size() > 0)
	{
		_lstLeft->addRow(1, tr("STR_GET_FOR_FREE_FROM").c_str());
		_lstLeft->setRowColor(row, _blue);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;
		for (const auto& res : getForFreeFrom)
		{
			std::string name = tr(res);
			name.insert(0, "  ");
			_lstLeft->addRow(1, name.c_str());
			_lstLeft->setRowColor(row, getResearchColor(res));
			_leftTopics.push_back(res);
			_leftFlags.push_back(TTV_RESEARCH);
			++row;
		}
	}

	// is lookup of
	if (lookupOf.size() > 0)
	{
		_lstLeft->addRow(1, tr("STR_IS_LOOKUP_OF").c_str());
		_lstLeft->setRowColor(row, _blue);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;
		for (const auto& res : lookupOf)
		{
			std::string name = tr(res);
			name.insert(0, "  ");
			_lstLeft->addRow(1, name.c_str());
			_lstLeft->setRowColor(row, getResearchColor(res));
			_leftTopics.push_back(res);
			_leftFlags.push_back(TTV_RESEARCH);
			++row;
		}
	}

	row = 0;

	// lookup link
	if (!Mod::isEmptyRuleName(rule->getLookup()))
	{
		_lstRight->addRow(1, tr("STR_LOOKUP").c_str());
		_lstRight->setRowColor(row, _blue);
		_rightTopics.push_back("-");
		_rightFlags.push_back(TTV_NONE);
		++row;

		std::string name = tr(rule->getLookup());
		name.insert(0, "  ");
		_lstRight->addRow(1, name.c_str());
		_lstRight->setRowColor(row, getResearchColor(rule->getLookup()));
		_rightTopics.push_back(rule->getLookup());
		_rightFlags.push_back(TTV_RESEARCH);
		++row;
	}

	// spawned item
	if (!Mod::isEmptyRuleName(rule->getSpawnedItem()) || !rule->getSpawnedItemList().empty())
	{
		_lstRight->addRow(1, tr("STR_SPAWNED_ITEMS").c_str());
		_lstRight->setRowColor(row, _blue);
		_rightTopics.push_back("-");
		_rightFlags.push_back(TTV_NONE);
		++row;
	}
	if (!Mod::isEmptyRuleName(rule->getSpawnedItem()))
	{
		std::string name = tr(rule->getSpawnedItem());
		name.insert(0, "  ");
		if (rule->getSpawnedItemCount() > 1)
		{
			name.append(" x");
			name.append(std::to_string(rule->getSpawnedItemCount()));
		}
		_lstRight->addRow(1, name.c_str());
		_lstRight->setRowColor(row, _white);
		_rightTopics.push_back(rule->getSpawnedItem());
		_rightFlags.push_back(TTV_ITEMS);
		++row;
	}
	for (auto& sil : rule->getSpawnedItemList())
	{
		std::string name = tr(sil);
		name.insert(0, "  ");
		_lstRight->addRow(1, name.c_str());
		_lstRight->setRowColor(row, _white);
		_rightTopics.push_back(sil);
		_rightFlags.push_back(TTV_ITEMS);
		++row;
	}

	// spawned event
	if (!Mod::isEmptyRuleName(rule->getSpawnedEvent()))
	{
		_lstRight->addRow(1, tr("STR_SPAWNED_EVENT").c_str());
		_lstRight->setRowColor(row, _blue);
		_rightTopics.push_back("-");
		_rightFlags.push_back(TTV_NONE);
		++row;

		std::string name = tr(rule->getSpawnedEvent());
		name.insert(0, "  ");
		_lstRight->addRow(1, name.c_str());
		_lstRight->setRowColor(row, _white);
		_rightTopics.push_back("-");
		_rightFlags.push_back(TTV_NONE);
		++row;
	}

	// 6. required by
	if (requiredByResearch.size() > 0 || requiredByManufacture.size() > 0 || requiredByFacilities.size() > 0 || requiredByItems.size() > 0 || requiredByCrafts.size() > 0)
	{
		_lstRight->addRow(1, tr("STR_REQUIRED_BY").c_str());
		_lstRight->setRowColor(row, _blue);
		_rightTopics.push_back("-");
		_rightFlags.push_back(TTV_NONE);
		++row;
	}

	// 6a. required by research
	if (requiredByResearch.size() > 0)
	{
		for (const auto& res : requiredByResearch)
		{
			std::string name = tr(res);
			name.insert(0, "  ");
			_lstRight->addRow(1, name.c_str());
			_lstRight->setRowColor(row, getResearchColor(res));
			_rightTopics.push_back(res);
			_rightFlags.push_back(TTV_RESEARCH);
			++row;
		}
	}

	// 6b. required by manufacture
	if (requiredByManufacture.size() > 0)
	{
		for (const auto& manuf : requiredByManufacture)
		{
			std::string name = tr(manuf);
			name.insert(0, "  ");
			name.append(tr("STR_M_FLAG"));
			_lstRight->addRow(1, name.c_str());
			if (!isDiscoveredManufacture(manuf))
			{
				_lstRight->setRowColor(row, _pink);
			}
			_rightTopics.push_back(manuf);
			_rightFlags.push_back(TTV_MANUFACTURING);
			++row;
		}
	}

	// 6c. required by facilities
	if (requiredByFacilities.size() > 0)
	{
		for (const auto& facType : requiredByFacilities)
		{
			std::string name = tr(facType);
			name.insert(0, "  ");
			name.append(tr("STR_F_FLAG"));
			_lstRight->addRow(1, name.c_str());
			if (!isDiscoveredFacility(facType))
			{
				_lstRight->setRowColor(row, _pink);
			}
			_rightTopics.push_back(facType);
			_rightFlags.push_back(TTV_FACILITIES);
			++row;
		}
	}

	// 6d. required by items
	if (requiredByItems.size() > 0)
	{
		for (const auto& itemType : requiredByItems)
		{
			std::string name = tr(itemType);
			name.insert(0, "  ");
			name.append(tr("STR_I_FLAG"));
			_lstRight->addRow(1, name.c_str());
			if (!isProtectedAndDiscoveredItem(itemType))
			{
				_lstRight->setRowColor(row, _pink);
			}
			_rightTopics.push_back(itemType);
			_rightFlags.push_back(TTV_ITEMS);
			++row;
		}
	}

	// 6e. required by crafts
	if (requiredByCrafts.size() > 0)
	{
		for (const auto& craftType : requiredByCrafts)
		{
			std::string name = tr(craftType);
			name.insert(0, "  ");
			name.append(tr("STR_C_FLAG"));
			_lstRight->addRow(1, name.c_str());
			if (!isDiscoveredCraft(craftType))
			{
				_lstRight->setRowColor(row, _pink);
			}
			_rightTopics.push_back(craftType);
			_rightFlags.push_back(TTV_CRAFTS);
			++row;
		}
	}

	// 6f. required by transformations
	if (requiredByTransformations.size() > 0)
	{
		_lstRight->addRow(1, tr("STR_REQUIRED_BY_TRANSFORMATIONS").c_str());
		_lstRight->setRowColor(row, _blue);
		_rightTopics.push_back("-");
		_rightFlags.push_back(TTV_NONE);
		++row;

		for (const auto& transformationType : requiredByTransformations)
		{
			std::string name = tr(transformationType);
			name.insert(0, "  ");
			_lstRight->addRow(1, name.c_str());
			_lstRight->setRowColor(row, _white);
			_rightTopics.push_back("-");
			_rightFlags.push_back(TTV_NONE);
			++row;
		}
	}

	// 7. leads to
	if (leadsTo.size() > 0)
	{
		_lstRight->addRow(1, tr("STR_LEADS_TO").c_str());
		_lstRight->setRowColor(row, _blue);
		_rightTopics.push_back("-");
		_rightFlags.push_back(TTV_NONE);
		++row;
		for (const auto& res : leadsTo)
		{
			const RuleResearch *iTemp = _game->getMod()->getResearch(res);
			if (std::find(unlocks.begin(), unlocks.end(), iTemp) != unlocks.end())
			{
				// if the same topic is also in the "Unlocks" section, skip it
				continue;
			}
			std::string name = tr(res);
			name.insert(0, "  ");
			_lstRight->addRow(1, name.c_str());
			_lstRight->setRowColor(row, getResearchColor(res));
			_rightTopics.push_back(res);
			_rightFlags.push_back(TTV_RESEARCH);
			++row;
		}
	}

	// 8a. unlocks
	if (unlocks.size() > 0)
	{
		_lstRight->addRow(1, tr("STR_UNLOCKS").c_str());
		_lstRight->setRowColor(row, _blue);
		_rightTopics.push_back("-");
		_rightFlags.push_back(TTV_NONE);
		++row;
		for (auto* i : unlocks)
		{
			std::string name = tr(i->getName());
			name.insert(0, "  ");
			_lstRight->addRow(1, name.c_str());
			_lstRight->setRowColor(row, getResearchColor(i->getName()));
			_rightTopics.push_back(i->getName());
			_rightFlags.push_back(TTV_RESEARCH);
			++row;
		}
	}

	// 8b. disables
	if (disables.size() > 0)
	{
		_lstRight->addRow(1, tr("STR_DISABLES").c_str());
		_lstRight->setRowColor(row, _blue);
		_rightTopics.push_back("-");
		_rightFlags.push_back(TTV_NONE);
		++row;
		for (auto* i : disables)
		{
			std::string name = tr(i->getName());
			name.insert(0, "  ");
			_lstRight->addRow(1, name.c_str());
			_lstRight->setRowColor(row, getResearchColor(i->getName()));
			_rightTopics.push_back(i->getName());
			_rightFlags.push_back(TTV_RESEARCH);
			++row;
		}
	}

	// 8c. reenables
	if (reenables.size() > 0)
	{
		_lstRight->addRow(1, tr("STR_REENABLES").c_str());
		_lstRight->setRowColor(row, _blue);
		_rightTopics.push_back("-");
		_rightFlags.push_back(TTV_NONE);
		++row;
		for (auto* i : reenables)
		{
			std::string name = tr(i->getName());
			name.insert(0, "  ");
			_lstRight->addRow(1, name.c_str());
			_lstRight->setRowColor(row, getResearchColor(i->getName()));
			_rightTopics.push_back(i->getName());
			_rightFlags.push_back(TTV_RESEARCH);
			++row;
		}
	}

	// 9. gives one for free
	if (free.size() > 0 || freeProtected.size() > 0)
	{
		int remaining = 0;
		int total = 0;
		for (auto* i : free)
		{
			if (getResearchColor(i->getName()) == _pink)
			{
				++remaining;
			}
			++total;
		}
		for (auto& itMap : freeProtected)
		{
			for (auto* i : itMap.second)
			{
				if (getResearchColor(i->getName()) == _pink)
				{
					++remaining;
				}
				++total;
			}
		}
		std::ostringstream ssFree;
		if (rule->sequentialGetOneFree())
		{
			ssFree << tr("STR_GIVES_ONE_FOR_FREE_SEQ");
		}
		else
		{
			ssFree << tr("STR_GIVES_ONE_FOR_FREE");
		}
		ssFree << " " << remaining << "/" << total;
		_lstRight->addRow(1, ssFree.str().c_str());
		_lstRight->setRowColor(row, _blue);
		_rightTopics.push_back("-");
		_rightFlags.push_back(TTV_NONE);
		++row;
		for (auto* i : free)
		{
			std::string name = tr(i->getName());
			name.insert(0, "  ");
			_lstRight->addRow(1, name.c_str());
			_lstRight->setRowColor(row, getResearchColor(i->getName()));
			_rightTopics.push_back(i->getName());
			_rightFlags.push_back(TTV_RESEARCH);
			++row;
		}
		for (auto& itMap : freeProtected)
		{
			std::string name2 = tr(itMap.first->getName());
			name2.insert(0, " ");
			name2.append(":");
			_lstRight->addRow(1, name2.c_str());
			_lstRight->setRowColor(row, getAltResearchColor(itMap.first->getName()));
			_rightTopics.push_back(itMap.first->getName());
			_rightFlags.push_back(TTV_RESEARCH);
			++row;
			for (auto* i : itMap.second)
			{
				std::string name = tr(i->getName());
				name.insert(0, "  ");
				_lstRight->addRow(1, name.c_str());
				_lstRight->setRowColor(row, getResearchColor(i->getName()));
				_rightTopics.push_back(i->getName());
				_rightFlags.push_back(TTV_RESEARCH);
				++row;
			}
		}
	}

	// 10. unlocks/disables alien missions, game arcs or geoscape events
	std::unordered_set<std::string> unlocksArcs, disablesArcs;
	std::unordered_set<std::string> unlocksEvents, disablesEvents;
	std::unordered_set<std::string> unlocksMissions, disablesMissions;
	bool affectsGameProgression = false;

	for (auto& arcScriptId : *_game->getMod()->getArcScriptList())
	{
		auto* arcScript = _game->getMod()->getArcScript(arcScriptId, false);
		if (arcScript)
		{
			for (auto& trigger : arcScript->getResearchTriggers())
			{
				if (trigger.first == _selectedTopic)
				{
					if (trigger.second)
						unlocksArcs.insert(arcScriptId);
					else
						disablesArcs.insert(arcScriptId);
				}
			}
		}
	}
	for (auto& eventScriptId : *_game->getMod()->getEventScriptList())
	{
		auto* eventScript = _game->getMod()->getEventScript(eventScriptId, false);
		if (eventScript)
		{
			for (auto& trigger : eventScript->getResearchTriggers())
			{
				if (trigger.first == _selectedTopic)
				{
					if (eventScript->getAffectsGameProgression()) affectsGameProgression = true; // remember for later
					if (trigger.second)
						unlocksEvents.insert(eventScriptId);
					else
						disablesEvents.insert(eventScriptId);
				}
			}
		}
	}
	for (auto& missionScriptId : *_game->getMod()->getMissionScriptList())
	{
		auto* missionScript = _game->getMod()->getMissionScript(missionScriptId, false);
		if (missionScript)
		{
			for (auto& trigger : missionScript->getResearchTriggers())
			{
				if (trigger.first == _selectedTopic)
				{
					if (trigger.second)
						unlocksMissions.insert(missionScriptId);
					else
						disablesMissions.insert(missionScriptId);
				}
			}
		}
	}
	bool showDetails = false;
	if (Options::isPasswordCorrect() && _game->isAltPressed())
	{
		showDetails = true;
	}
	if (showDetails)
	{
		auto addGameProgressionEntry = [&](const std::unordered_set<std::string>& list, const std::string& label)
		{
			if (!list.empty())
			{
				_lstRight->addRow(1, tr(label).c_str());
				_lstRight->setRowColor(row, _blue);
				_rightTopics.push_back("-");
				_rightFlags.push_back(TTV_NONE);
				++row;
				for (auto& i : list)
				{
					std::ostringstream name;
					name << "  " << tr(i);
					_lstRight->addRow(1, name.str().c_str());
					_lstRight->setRowColor(row, _white);
					_rightTopics.push_back("-");
					_rightFlags.push_back(TTV_NONE);
					++row;
				}
			}
		};

		addGameProgressionEntry(unlocksArcs, "STR_UNLOCKS_ARCS");
		addGameProgressionEntry(disablesArcs, "STR_DISABLES_ARCS");

		addGameProgressionEntry(unlocksMissions, "STR_UNLOCKS_MISSIONS");
		addGameProgressionEntry(disablesMissions, "STR_DISABLES_MISSIONS");

		addGameProgressionEntry(unlocksEvents, "STR_UNLOCKS_EVENTS");
		addGameProgressionEntry(disablesEvents, "STR_DISABLES_EVENTS");
	}
	else
	{
		int showDisclaimer = 0;
		if (!unlocksMissions.empty() || !disablesMissions.empty() || !unlocksArcs.empty() || !disablesArcs.empty())
		{
			showDisclaimer = 1; // STR_AFFECTS_GAME_PROGRESSION
		}
		else if (!unlocksEvents.empty() || !disablesEvents.empty())
		{
			if (affectsGameProgression)
			{
				showDisclaimer = 1; // STR_AFFECTS_GAME_PROGRESSION
			}
			else if (Options::isPasswordCorrect())
			{
				showDisclaimer = 2; // .
			}
		}
		if (showDisclaimer > 0)
		{
			_lstRight->addRow(1, tr("STR_AFFECTS_GAME_PROGRESSION").c_str());
			if (showDisclaimer == 1)
				_lstRight->setRowColor(row, _gold);
			else
				_lstRight->setRowColor(row, _white);
			_rightTopics.push_back("-");
			_rightFlags.push_back(TTV_NONE);
			++row;
		}
	}
}

/**
 * Handles, renders and organizes Manufacture entry data.
 */
void TechTreeViewerState::handleManufactureData()
{
	int row = 0;
	RuleManufacture *rule = _game->getMod()->getManufacture(_selectedTopic);
	if (rule == 0)
		return;

	// 1. requires
	const std::vector<const RuleResearch*> reqs = rule->getRequirements();
	if (reqs.size() > 0)
	{
		_lstLeft->addRow(1, tr("STR_RESEARCH_REQUIRED").c_str());
		_lstLeft->setRowColor(row, _blue);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;
		for (auto* i : reqs)
		{
			std::string name = tr(i->getName());
			name.insert(0, "  ");
			_lstLeft->addRow(1, name.c_str());
			_lstLeft->setRowColor(row, getResearchColor(i->getName()));
			_leftTopics.push_back(i->getName());
			_leftFlags.push_back(TTV_RESEARCH);
			++row;
		}
	}

	// 2. requires services (from base facilities)
	if (rule->getRequireBaseFunc().any())
	{
		_lstLeft->addRow(1, tr("STR_SERVICES_REQUIRED").c_str());
		_lstLeft->setRowColor(row, _blue);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;
		for (const auto& requiredServiceType : _game->getMod()->getBaseFunctionNames(rule->getRequireBaseFunc()))
		{
			std::string name = tr(requiredServiceType);
			name.insert(0, "  ");
			_lstLeft->addRow(1, name.c_str());
			_lstLeft->setRowColor(row, _gold);
			_leftTopics.push_back("-");
			_leftFlags.push_back(TTV_NONE);
			++row;
		}
	}

	// 3. inputs
	const std::map<const RuleCraft*, int> craftInputs = rule->getRequiredCrafts();
	const std::map<const RuleItem*, int> inputs = rule->getRequiredItems();
	if (inputs.size() > 0 || craftInputs.size() > 0)
	{
		_lstLeft->addRow(1, tr("STR_MATERIALS_REQUIRED").c_str());
		_lstLeft->setRowColor(row, _blue);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;
		for (auto& i : craftInputs)
		{
			std::ostringstream name;
			name << "  ";
			name << tr(i.first->getType());
			name << ": ";
			name << i.second;
			_lstLeft->addRow(1, name.str().c_str());
			_lstLeft->setRowColor(row, _white);
			_leftTopics.push_back("-");
			_leftFlags.push_back(TTV_NONE);
			++row;
		}
		for (auto& i : inputs)
		{
			std::ostringstream name;
			name << "  ";
			name << tr(i.first->getType());
			name << ": ";
			name << i.second;
			_lstLeft->addRow(1, name.str().c_str());
			_lstLeft->setRowColor(row, _white);
			_leftTopics.push_back(i.first->getType());
			_leftFlags.push_back(TTV_ITEMS);
			++row;
		}
	}

	// empty line
	_lstLeft->addRow(1, "");
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	++row;

	// cost per unit
	if (rule->getManufactureCost() > 0)
	{
		_lstLeft->addRow(1, tr("STR_TTV_COST_PER_UNIT").c_str());
		_lstLeft->setRowColor(row, _blue);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;

		std::ostringstream txt;
		txt << "  ";
		txt << Unicode::formatFunding(rule->getManufactureCost());
		_lstLeft->addRow(1, txt.str().c_str());
		_lstLeft->setRowColor(row, _white);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;
	}
	// engineer hours
	if (rule->getManufactureTime() > 0)
	{
		_lstLeft->addRow(1, tr("STR_TTV_ENGINEER_HOURS").c_str());
		_lstLeft->setRowColor(row, _blue);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;

		int days = rule->getManufactureTime() / 24;
		int hours = rule->getManufactureTime() % 24;
		std::ostringstream txt;
		txt << "  ";
		txt << rule->getManufactureTime();
		txt << " (";
		if (days > 0)
		{
			txt << tr("STR_DAY_SHORT").arg(days);
		}
		if (hours > 0)
		{
			if (days > 0)
			{
				txt << "/";
			}
			txt << tr("STR_HOUR_SHORT").arg(hours);
		}
		txt << ")";
		_lstLeft->addRow(1, txt.str().c_str());
		_lstLeft->setRowColor(row, _white);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;
	}
	// work space required
	if (rule->getRequiredSpace() > 0)
	{
		_lstLeft->addRow(1, tr("STR_TTV_WORK_SPACE_REQUIRED").c_str());
		_lstLeft->setRowColor(row, _blue);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;

		std::ostringstream txt;
		txt << "  ";
		txt << rule->getRequiredSpace();
		_lstLeft->addRow(1, txt.str().c_str());
		_lstLeft->setRowColor(row, _white);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;
	}

	row = 0;

	// 4. outputs
	const std::map<const RuleItem*, int> outputs = rule->getProducedItems();
	if (outputs.size() > 0 || rule->getProducedCraft())
	{
		_lstRight->addRow(1, tr("STR_ITEMS_PRODUCED").c_str());
		_lstRight->setRowColor(row, _blue);
		_rightTopics.push_back("-");
		_rightFlags.push_back(TTV_NONE);
		++row;
		if (rule->getProducedCraft())
		{
			std::ostringstream name;
			name << "  ";
			name << tr(rule->getProducedCraft()->getType());
			name << ": 1";
			_lstRight->addRow(1, name.str().c_str());
			_lstRight->setRowColor(row, _white);
			_rightTopics.push_back("-");
			_rightFlags.push_back(TTV_NONE);
			++row;
		}
		for (auto& i : outputs)
		{
			std::ostringstream name;
			name << "  ";
			name << tr(i.first->getType());
			name << ": ";
			name << i.second;
			_lstRight->addRow(1, name.str().c_str());
			_lstRight->setRowColor(row, _white);
			_rightTopics.push_back(i.first->getType());
			_rightFlags.push_back(TTV_ITEMS);
			++row;
		}
	}

	// 4b. random outputs
	auto& randomOutputs = rule->getRandomProducedItems();
	if (randomOutputs.size() > 0)
	{
		_lstRight->addRow(1, tr("STR_RANDOM_PRODUCTION_DISCLAIMER").c_str());
		_lstRight->setRowColor(row, _blue);
		_rightTopics.push_back("-");
		_rightFlags.push_back(TTV_NONE);
		++row;
		int total = 0;
		for (auto& n : randomOutputs)
		{
			total += n.first;
		}
		for (auto& randomOutput : randomOutputs)
		{
			std::ostringstream chance;
			chance << " " << randomOutput.first * 100 / total << "%";
			_lstRight->addRow(1, chance.str().c_str());
			_lstRight->setRowColor(row, _gold);
			_rightTopics.push_back("-");
			_rightFlags.push_back(TTV_NONE);
			++row;
			for (auto& i : randomOutput.second)
			{
				std::ostringstream name;
				name << "  ";
				name << tr(i.first->getType());
				name << ": ";
				name << i.second;
				_lstRight->addRow(1, name.str().c_str());
				_lstRight->setRowColor(row, _white);
				_rightTopics.push_back(i.first->getType());
				_rightFlags.push_back(TTV_ITEMS);
				++row;
			}
		}
	}

	// 5. person joining
	if (rule->getSpawnedPersonType() != "")
	{
		_lstRight->addRow(1, tr("STR_PERSON_RECRUITED").c_str());
		_lstRight->setRowColor(row, _blue);
		_rightTopics.push_back("-");
		_rightFlags.push_back(TTV_NONE);
		++row;

		// person joining
		std::ostringstream name;
		name << "  ";
		if (!rule->getSpawnedSoldierTemplate().IsNull())
		{
			name << "*";
		}
		name << tr(rule->getSpawnedPersonName() != "" ? rule->getSpawnedPersonName() : rule->getSpawnedPersonType());
		_lstRight->addRow(1, name.str().c_str());
		_lstRight->setRowColor(row, _white);
		_rightTopics.push_back("-");
		_rightFlags.push_back(TTV_NONE);
		++row;
	}
}

/**
 * Handles, renders and organizes Facility entry data.
 */
void TechTreeViewerState::handleFacilityData()
{
	int row = 0;
	RuleBaseFacility *rule = _game->getMod()->getBaseFacility(_selectedTopic);
	if (rule == 0)
		return;

	// 1. requires
	const std::vector<std::string> reqs = rule->getRequirements();
	if (reqs.size() > 0)
	{
		_lstLeft->addRow(1, tr("STR_RESEARCH_REQUIRED").c_str());
		_lstLeft->setRowColor(row, _blue);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;
		for (const auto& res : reqs)
		{
			std::string name = tr(res);
			name.insert(0, "  ");
			_lstLeft->addRow(1, name.c_str());
			_lstLeft->setRowColor(row, getResearchColor(res));
			_leftTopics.push_back(res);
			_leftFlags.push_back(TTV_RESEARCH);
			++row;
		}
	}

	// 2. requires services (from other base facilities)
	if (rule->getRequireBaseFunc().any())
	{
		_lstLeft->addRow(1, tr("STR_SERVICES_REQUIRED").c_str());
		_lstLeft->setRowColor(row, _blue);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;
		for (const auto& requiredServiceType : _game->getMod()->getBaseFunctionNames(rule->getRequireBaseFunc()))
		{
			std::string name = tr(requiredServiceType);
			name.insert(0, "  ");
			_lstLeft->addRow(1, name.c_str());
			_lstLeft->setRowColor(row, _gold);
			_leftTopics.push_back("-");
			_leftFlags.push_back(TTV_NONE);
			++row;
		}
	}

	row = 0;

	// 3. provides services
	if (rule->getProvidedBaseFunc().any())
	{
		_lstRight->addRow(1, tr("STR_SERVICES_PROVIDED").c_str());
		_lstRight->setRowColor(row, _blue);
		_rightTopics.push_back("-");
		_rightFlags.push_back(TTV_NONE);
		++row;
		for (const auto& providedServiceType : _game->getMod()->getBaseFunctionNames(rule->getProvidedBaseFunc()))
		{
			std::string name = tr(providedServiceType);
			name.insert(0, "  ");
			_lstRight->addRow(1, name.c_str());
			_lstRight->setRowColor(row, _gold);
			_rightTopics.push_back("-");
			_rightFlags.push_back(TTV_NONE);
			++row;
		}
	}

	// 4. forbids services
	if (rule->getForbiddenBaseFunc().any())
	{
		_lstRight->addRow(1, tr("STR_SERVICES_FORBIDDEN").c_str());
		_lstRight->setRowColor(row, _blue);
		_rightTopics.push_back("-");
		_rightFlags.push_back(TTV_NONE);
		++row;
		for (const auto& forbiddenServiceType : _game->getMod()->getBaseFunctionNames(rule->getForbiddenBaseFunc()))
		{
			std::string name = tr(forbiddenServiceType);
			name.insert(0, "  ");
			_lstRight->addRow(1, name.c_str());
			_lstRight->setRowColor(row, _white);
			_rightTopics.push_back("-");
			_rightFlags.push_back(TTV_NONE);
			++row;
		}
	}
}

/**
 * Handles, renders and organizes Item entry data.
 */
void TechTreeViewerState::handleItemData()
{
	_lstLeft->setVisible(false);
	_lstRight->setVisible(false);
	_lstFull->setVisible(true);

	int row = 0;
	RuleItem *rule = _game->getMod()->getItem(_selectedTopic);
	if (rule == 0)
		return;

	// 1. requires to use/equip
	const std::vector<const RuleResearch *> reqs = rule->getRequirements();
	if (reqs.size() > 0)
	{
		_lstFull->addRow(1, tr("STR_RESEARCH_REQUIRED_USE").c_str());
		_lstFull->setRowColor(row, _blue);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;
		for (auto* i : reqs)
		{
			std::string name = tr(i->getName());
			name.insert(0, "  ");
			_lstFull->addRow(1, name.c_str());
			_lstFull->setRowColor(row, getResearchColor(i->getName()));
			_leftTopics.push_back(i->getName());
			_leftFlags.push_back(TTV_RESEARCH);
			++row;
		}
	}

	// 2. requires to buy
	const std::vector<const RuleResearch *> reqsBuy = rule->getBuyRequirements();
	if (reqsBuy.size() > 0)
	{
		_lstFull->addRow(1, tr("STR_RESEARCH_REQUIRED_BUY").c_str());
		_lstFull->setRowColor(row, _blue);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;
		for (auto* i : reqsBuy)
		{
			std::string name = tr(i->getName());
			name.insert(0, "  ");
			_lstFull->addRow(1, name.c_str());
			_lstFull->setRowColor(row, getResearchColor(i->getName()));
			_leftTopics.push_back(i->getName());
			_leftFlags.push_back(TTV_RESEARCH);
			++row;
		}
	}

	// 3. services (from base facilities) required to buy
	if (rule->getRequiresBuyBaseFunc().any())
	{
		const std::vector<std::string> servicesBuy = _game->getMod()->getBaseFunctionNames(rule->getRequiresBuyBaseFunc());
		_lstFull->addRow(1, tr("STR_SERVICES_REQUIRED_BUY").c_str());
		_lstFull->setRowColor(row, _blue);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;
		for (auto& i : servicesBuy)
		{
			std::string name = tr(i);
			name.insert(0, "  ");
			_lstFull->addRow(1, name.c_str());
			_lstFull->setRowColor(row, _gold);
			_leftTopics.push_back("-");
			_leftFlags.push_back(TTV_NONE);
			++row;
		}
	}

	// 4. produced by
	std::vector<std::string> producedBy;
	for (auto& j : _game->getMod()->getManufactureList())
	{
		RuleManufacture* temp = _game->getMod()->getManufacture(j);
		bool found = false;
		for (auto& i : temp->getProducedItems())
		{
			if (i.first == rule)
			{
				producedBy.push_back(j);
				found = true;
				break;
			}
		}
		if (!found)
		{
			for (auto& itMap : temp->getRandomProducedItems())
			{
				for (auto& i : itMap.second)
				{
					if (i.first == rule)
					{
						producedBy.push_back(j);
						found = true;
						break;
					}
				}
				if (found) break;
			}
		}
	}
	if (producedBy.size() > 0)
	{
		_lstFull->addRow(1, tr("STR_PRODUCED_BY").c_str());
		_lstFull->setRowColor(row, _blue);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;
		for (const auto& manuf : producedBy)
		{
			std::string name = tr(manuf);
			name.insert(0, "  ");
			name.append(tr("STR_M_FLAG"));
			_lstFull->addRow(1, name.c_str());
			if (!isDiscoveredManufacture(manuf))
			{
				_lstFull->setRowColor(row, _pink);
			}
			_leftTopics.push_back(manuf);
			_leftFlags.push_back(TTV_MANUFACTURING);
			++row;
		}
	}

	// 5. spawned by
	std::vector<std::string> spawnedBy;
	for (auto& j : _game->getMod()->getResearchList())
	{
		RuleResearch* temp = _game->getMod()->getResearch(j);
		if (temp->getSpawnedItem() == rule->getType())
		{
			spawnedBy.push_back(j);
		}
		else
		{
			for (auto& sil : temp->getSpawnedItemList())
			{
				if (sil == rule->getType())
				{
					spawnedBy.push_back(j);
					break;
				}
			}
		}
	}
	if (spawnedBy.size() > 0)
	{
		_lstFull->addRow(1, tr("STR_SPAWNED_BY").c_str());
		_lstFull->setRowColor(row, _blue);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;
		for (const auto& res : spawnedBy)
		{
			std::string name = tr(res);
			name.insert(0, "  ");
			_lstFull->addRow(1, name.c_str());
			_lstFull->setRowColor(row, getResearchColor(res));
			_leftTopics.push_back(res);
			_leftFlags.push_back(TTV_RESEARCH);
			++row;
		}
	}

	// empty line
	_lstFull->addRow(1, "");
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	++row;

	// cost to buy
	if (rule->getBuyCost() > 0)
	{
		_lstFull->addRow(1, tr("STR_TTV_COST_PER_UNIT").c_str());
		_lstFull->setRowColor(row, _blue);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;

		std::ostringstream txt;
		txt << "  ";
		txt << Unicode::formatFunding(rule->getBuyCost());
		_lstFull->addRow(1, txt.str().c_str());
		_lstFull->setRowColor(row, _white);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;
	}
}

/**
 * Handles, renders and organizes Craft entry data.
 */
void TechTreeViewerState::handleCraftData()
{
	int row = 0;
	RuleCraft *rule = _game->getMod()->getCraft(_selectedTopic);
	if (rule == 0)
		return;

	// 1. requires
	const std::vector<std::string> reqs = rule->getRequirements();
	if (reqs.size() > 0)
	{
		_lstLeft->addRow(1, tr("STR_RESEARCH_REQUIRED").c_str());
		_lstLeft->setRowColor(row, _blue);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;
		for (const auto& res : reqs)
		{
			std::string name = tr(res);
			name.insert(0, "  ");
			_lstLeft->addRow(1, name.c_str());
			_lstLeft->setRowColor(row, getResearchColor(res));
			_leftTopics.push_back(res);
			_leftFlags.push_back(TTV_RESEARCH);
			++row;
		}
	}

	// 2. services (from base facilities) required to buy
	if (rule->getRequiresBuyBaseFunc().any())
	{
		_lstLeft->addRow(1, tr("STR_SERVICES_REQUIRED_BUY").c_str());
		_lstLeft->setRowColor(row, _blue);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;
		for (const auto& serviceReqToBuy : _game->getMod()->getBaseFunctionNames(rule->getRequiresBuyBaseFunc()))
		{
			std::string name = tr(serviceReqToBuy);
			name.insert(0, "  ");
			_lstLeft->addRow(1, name.c_str());
			_lstLeft->setRowColor(row, _gold);
			_leftTopics.push_back("-");
			_leftFlags.push_back(TTV_NONE);
			++row;
		}
	}

	// 3. produced by
	std::vector<std::string> producedBy;
	for (auto& j : _game->getMod()->getManufactureList())
	{
		RuleManufacture* temp = _game->getMod()->getManufacture(j);
		if (temp->getProducedCraft() == rule)
		{
			producedBy.push_back(j);
			break;
		}
	}
	if (producedBy.size() > 0)
	{
		_lstLeft->addRow(1, tr("STR_PRODUCED_BY").c_str());
		_lstLeft->setRowColor(row, _blue);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;
		for (const auto& manuf : producedBy)
		{
			std::string name = tr(manuf);
			name.insert(0, "  ");
			name.append(tr("STR_M_FLAG"));
			_lstLeft->addRow(1, name.c_str());
			if (!isDiscoveredManufacture(manuf))
			{
				_lstLeft->setRowColor(row, _pink);
			}
			_leftTopics.push_back(manuf);
			_leftFlags.push_back(TTV_MANUFACTURING);
			++row;
		}
	}

	// empty line
	_lstLeft->addRow(1, "");
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	++row;

	// cost to buy
	if (rule->getBuyCost() > 0)
	{
		_lstLeft->addRow(1, tr("STR_TTV_COST_PER_UNIT").c_str());
		_lstLeft->setRowColor(row, _blue);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;

		std::ostringstream txt;
		txt << "  ";
		txt << Unicode::formatFunding(rule->getBuyCost());
		_lstLeft->addRow(1, txt.str().c_str());
		_lstLeft->setRowColor(row, _white);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		++row;
	}
}

/**
 * Handles, renders and organizes Arc script data.
 */
void TechTreeViewerState::handleArcScript()
{
	int row = 0;
	const RuleArcScript *rule = _game->getMod()->getArcScript(_selectedTopic);
	if (rule == 0)
		return;

	// 1. Basic Triggers
	_lstLeft->addRow(1, tr("STR_TRIGGERS_BASIC").c_str());
	_lstLeft->setRowColor(row, _white);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++;
	std::ostringstream bTrigger;

	// 1a. Execution Odds
	bTrigger << "  " << tr("STR_TRIGGER_ODDS") << " ";
	bTrigger << Unicode::formatPercentage(rule->getExecutionOdds());
	_lstLeft->addRow(1, bTrigger.str().c_str());
	_lstLeft->setRowColor(row, _gold);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++; bTrigger.str(""); bTrigger.clear();

	// 1b. First/Last Month
	bTrigger << "  " << tr("STR_TRIGGER_MONTH") << " ";
	if (rule->getFirstMonth() == 0) bTrigger << tr("STR_TRMONTH_FIRST");
	else bTrigger << tr("STR_TRMONTH_LATER") << " " << rule->getFirstMonth();
	if (rule->getFirstMonth() != rule->getLastMonth())
	{
		if (rule->getLastMonth() == -1) bTrigger << " " << tr("STR_TRSIGN_UP");
		else bTrigger << " " << tr("STR_TRSIGN_MID") << " " <<
			tr("STR_TRMONTH_LATER") << " " << rule->getLastMonth();
	}
	bool isValidTrigger = isValidMonthTrigger(rule);
	_lstLeft->addRow(1, bTrigger.str().c_str());
	_lstLeft->setRowColor(row, isValidTrigger ? _purple : _pink);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++; bTrigger.str(""); bTrigger.clear();

	// 1c. Min/Max Difficulty
	bTrigger << "  " << tr("STR_TRIGGER_DIFFICULTY") << " ";
	bTrigger << tr("STR_TRDIFF_LVL_" + std::to_string(rule->getMinDifficulty()));
	if (rule->getMinDifficulty() != rule->getMaxDifficulty())
	{
		bTrigger << " " << tr("STR_TRSIGN_MID") << " ";
		bTrigger << tr("STR_TRDIFF_LVL_" + std::to_string(rule->getMaxDifficulty()));
	}
	isValidTrigger = isValidDiffTrigger(rule);
	_lstLeft->addRow(1, bTrigger.str().c_str());
	_lstLeft->setRowColor(row, isValidTrigger ? _purple : _pink);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++; bTrigger.str(""); bTrigger.clear();

	// 1d. Min/Max Score
	bTrigger << "  " << tr("STR_TRIGGER_SCORE") << " ";
	if (rule->getMinScore() == INT_MIN && rule->getMaxScore() == INT_MAX)
		bTrigger << tr("STR_TRSCORE_NONE");
	else
	{
		if (rule->getMinScore() > INT_MIN) bTrigger << rule->getMinScore() << " ";
		else bTrigger << tr("STR_TRSIGN_UP") << " ";
		if (rule->getMinScore() > INT_MIN && rule->getMaxScore() < INT_MAX)
			bTrigger << tr("STR_TRSIGN_MID") << " ";
		if (rule->getMaxScore() < INT_MAX) bTrigger << rule->getMaxScore();
		else bTrigger << tr("STR_TRSIGN_UP");
	}
	isValidTrigger = isValidScoreTrigger(rule);
	_lstLeft->addRow(1, bTrigger.str().c_str());
	_lstLeft->setRowColor(row, isValidTrigger ? _purple : _pink);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++; bTrigger.str(""); bTrigger.clear();

	// 1e. Min/Max Funds
	bTrigger << "  " << tr("STR_TRIGGER_FUNDS") << " ";
	if (rule->getMinFunds() == INT64_MIN && rule->getMaxFunds() == INT64_MAX)
		bTrigger << tr("STR_TRFUNDS_NONE");
	else
	{
		if (rule->getMinFunds() > INT64_MIN) bTrigger <<
			Unicode::formatFunding(rule->getMinFunds()) << " ";
		else bTrigger << tr("STR_TRSIGN_UP") << " ";
		if (rule->getMinFunds() > INT64_MIN && rule->getMaxFunds() < INT64_MAX)
			bTrigger << tr("STR_TRSIGN_MID") << " ";
		if (rule->getMaxFunds() < INT64_MAX) bTrigger <<
			Unicode::formatFunding(rule->getMaxFunds());
		else bTrigger << tr("STR_TRSIGN_UP");
	}
	isValidTrigger = isValidFundsTrigger(rule);
	_lstLeft->addRow(1, bTrigger.str().c_str());
	_lstLeft->setRowColor(row, isValidTrigger ? _purple : _pink);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++; bTrigger.str(""); bTrigger.clear();

	// 1f. Min/Max Counter
	bTrigger << "  " << tr("STR_TRIGGER_COUNTER") << " ";
	if (rule->getMissionVarName().empty() && rule->getMissionMarkerName().empty())
		bTrigger << tr("STR_TRCOUNT_NONE");
	else if (rule->getCounterMin() == 0 && rule->getCounterMax() == -1)
		bTrigger << tr("STR_TRCOUNT_NONE");
	else
	{
		if (rule->getCounterMin() > 0) bTrigger << rule->getCounterMin() << " ";
		else bTrigger << tr("STR_TRSIGN_UP") << " ";
		if (rule->getCounterMin() > 0 && rule->getCounterMax() != -1)
			bTrigger << tr("STR_TRSIGN_MID") << " ";
		if (rule->getCounterMax() != 1) bTrigger << rule->getCounterMax();
	}
	isValidTrigger = isValidCounterTrigger(rule);
	_lstLeft->addRow(1, bTrigger.str().c_str());
	_lstLeft->setRowColor(row, isValidTrigger ? _purple : _pink);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++; bTrigger.str(""); bTrigger.clear();

	// 1g. Max Arcs Limit
	const auto& arcSeqResearch = rule->getSequentialArcs();
	const auto& arcRandResearch = rule->getRandomArcs();
	bTrigger << "  " << tr("STR_TRIGGER_ARCLIMIT") << " ";
	if (rule->getMaxArcs() < 0) bTrigger << tr("STR_TRARCLIM_NONE");
	else bTrigger << rule->getMaxArcs();
	bool isValidAmount = (rule->getMaxArcs() == -1);
	if (rule->getMaxArcs() > -1)
	{
		int currArcNum = 0;
		for (auto& arcSeqRes : arcSeqResearch)
			if (_save->isResearched(arcSeqRes))
				currArcNum++;
		for (auto& arcRandRes : arcRandResearch.getChoices())
			if (_save->isResearched(arcRandRes.first))
				currArcNum++;
		isValidAmount = (rule->getMaxArcs() > currArcNum);
	}
	_lstLeft->addRow(1, bTrigger.str().c_str());
	_lstLeft->setRowColor(row, isValidAmount ? _purple : _pink);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++; bTrigger.str(""); bTrigger.clear();

	// 2. Research Trigger
	const auto& resTriggers = rule->getResearchTriggers();
	if (resTriggers.size() > 0)
	{
		_lstLeft->addRow(1, ""); _leftTopics.push_back("-"); _leftFlags.push_back(TTV_NONE); row++;
		_lstLeft->addRow(1, tr("STR_TRIGGERS_RESEARCH").c_str());
		_lstLeft->setRowColor(row, _white);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		row++;
		for (auto& resTrigger : resTriggers)
		{
			std::ostringstream rowRes;
			rowRes << "  " << tr(resTrigger.first);
			rowRes << ": " << std::boolalpha << resTrigger.second;
			bool isResearched = (_save->isResearched(resTrigger.first) == resTrigger.second);
			_lstLeft->addRow(1, rowRes.str().c_str());
			_lstLeft->setRowColor(row, isResearched ? _purple : _pink);
			_leftTopics.push_back("-");
			_leftFlags.push_back(TTV_NONE);
			row++;
		}
	}

	// 3. Item Triggers
	const auto& itemTriggers = rule->getItemTriggers();
	if (itemTriggers.size() > 0)
	{
		_lstLeft->addRow(1, ""); _leftTopics.push_back("-"); _leftFlags.push_back(TTV_NONE); row++;
		_lstLeft->addRow(1, tr("STR_TRIGGERS_ITEM").c_str());
		_lstLeft->setRowColor(row, _white);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		row++;
		for (auto& itemTrigger : itemTriggers)
		{
			std::ostringstream rowItem;
			rowItem << "  " << tr(itemTrigger.first);
			rowItem << ": " << std::boolalpha << itemTrigger.second;
			bool isObtained = (_save->isItemObtained(itemTrigger.first) == itemTrigger.second);
			_lstLeft->addRow(1, rowItem.str().c_str());
			_lstLeft->setRowColor(row, isObtained ? _purple : _pink);
			_leftTopics.push_back("-");
			_leftFlags.push_back(TTV_NONE);
			row++;
		}
	}

	// 4. Facility Triggers
	const auto& facTriggers = rule->getFacilityTriggers();
	if (facTriggers.size() > 0)
	{
		_lstLeft->addRow(1, ""); _leftTopics.push_back("-"); _leftFlags.push_back(TTV_NONE); row++;
		_lstLeft->addRow(1, tr("STR_TRIGGERS_FACILITY").c_str());
		_lstLeft->setRowColor(row, _white);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		row++;
		for (auto& facTrigger : facTriggers)
		{
			std::ostringstream rowFac;
			rowFac << "  " << tr(facTrigger.first);
			rowFac << ": " << std::boolalpha << facTrigger.second;
			bool isConstructed = (_save->isFacilityBuilt(facTrigger.first) == facTrigger.second);
			_lstLeft->addRow(1, rowFac.str().c_str());
			_lstLeft->setRowColor(row, isConstructed ? _purple : _pink);
			_leftTopics.push_back("-");
			_leftFlags.push_back(TTV_NONE);
			row++;
		}
	}

	// 5. Regional Triggers
	const auto& regTriggers = rule->getXcomBaseInRegionTriggers();
	const auto& terTriggers = rule->getXcomBaseInCountryTriggers();
	if (regTriggers.size() > 0 || terTriggers.size() > 0)
	{
		_lstLeft->addRow(1, ""); _leftTopics.push_back("-"); _leftFlags.push_back(TTV_NONE); row++;
		std::set<std::string> baseRegions;
		std::set<std::string> baseCountries;
		for (auto* xcomBase : *_save->getBases())
		{
			Region* region = _save->locateRegion(*xcomBase);
			if (region) baseRegions.insert(region->getRules()->getType());
			Country* country = _save->locateCountry(*xcomBase);
			if (country) baseCountries.insert(country->getRules()->getType());
		}
		_lstLeft->addRow(1, tr("STR_TRIGGERS_REGIONAL").c_str());
		_lstLeft->setRowColor(row, _white);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		row++;
		for (auto& regTrigger : regTriggers)
		{
			std::ostringstream rowReg;
			rowReg << "  " << tr(regTrigger.first);
			rowReg << ": " << std::boolalpha << regTrigger.second;
			bool isRegFound = (baseRegions.find(regTrigger.first) != baseRegions.end());
			bool isRegValid = isRegFound == regTrigger.second;
			_lstLeft->addRow(1, rowReg.str().c_str());
			_lstLeft->setRowColor(row, isRegValid ? _purple : _pink);
			_leftTopics.push_back("-");
			_leftFlags.push_back(TTV_NONE);
			row++;
		}
		for (auto& terTrigger : terTriggers)
		{
			std::ostringstream rowTer;
			rowTer << "  " << tr(terTrigger.first);
			rowTer << ": " << std::boolalpha << terTrigger.second;
			bool isTerFound = (baseCountries.find(terTrigger.first) != baseCountries.end());
			bool isTerValid = isTerFound == terTrigger.second;
			_lstLeft->addRow(1, rowTer.str().c_str());
			_lstLeft->setRowColor(row, isTerValid ? _purple : _pink);
			_leftTopics.push_back("-");
			_leftFlags.push_back(TTV_NONE);
			row++;
		}
	}

	row = 0; // Right UI Panel Switch

	// 7. Sequential Arcs
	if (arcSeqResearch.size() > 0)
	{
		_lstRight->addRow(1, tr("STR_ARC_RESEARCH_SEQ").c_str());
		_lstRight->setRowColor(row, _white);
		_rightTopics.push_back("-");
		_rightFlags.push_back(TTV_NONE);
		row++;
		for (auto& arcSeqRes : arcSeqResearch)
		{
			std::ostringstream arcSeq;
			arcSeq << "  ";
			strPush(arcSeq, arcSeqRes);
			_lstRight->addRow(1, arcSeq.str().c_str());
			_lstRight->setRowColor(row,
				_save->isResearched(arcSeqRes) ?
				_purple : _pink);
			_rightTopics.push_back("-");
			_rightFlags.push_back(TTV_NONE);
			row++;
		}
	}

	// 8. Random Arcs
	if (arcRandResearch.getChoices().size() > 0)
	{
		if (arcSeqResearch.size() > 0)
		{
			_lstRight->addRow(1, ""); _rightTopics.push_back("-");
			_rightFlags.push_back(TTV_NONE); row++;
		}
		_lstRight->addRow(1, tr("STR_ARC_RESEARCH_RAND").c_str());
		_lstRight->setRowColor(row, _white);
		_rightTopics.push_back("-");
		_rightFlags.push_back(TTV_NONE);
		row++;
		for (auto& arcRandRes : arcRandResearch.getChoices())
		{
			std::ostringstream arcRand;
			arcRand << "  ";
			strPush(arcRand, arcRandRes.first);
			arcRand << ": ";
			arcRand << arcRandRes.second;
			_lstRight->addRow(1, arcRand.str().c_str());
			_lstRight->setRowColor(row,
				_save->isResearched(arcRandRes.first) ?
				_purple : _pink);
			_rightTopics.push_back("-");
			_rightFlags.push_back(TTV_NONE);
			row++;
		}
	}
}

/**
 * Handles, renders and organizes Event script data.
 */
void TechTreeViewerState::handleEventScript()
{
	int row = 0;
	const RuleEventScript *rule = _game->getMod()->getEventScript(_selectedTopic);
	if (rule == 0)
		return;

	// 1. Basic Triggers
	_lstLeft->addRow(1, tr("STR_TRIGGERS_BASIC").c_str());
	_lstLeft->setRowColor(row, _white);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++;
	std::ostringstream bTrigger;

	// 1a. Execution Odds
	bTrigger << "  " << tr("STR_TRIGGER_ODDS") << " ";
	bTrigger << Unicode::formatPercentage(rule->getExecutionOdds());
	_lstLeft->addRow(1, bTrigger.str().c_str());
	_lstLeft->setRowColor(row, _gold);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++; bTrigger.str(""); bTrigger.clear();

	// 1b. First/Last Month
	bTrigger << "  " << tr("STR_TRIGGER_MONTH") << " ";
	if (rule->getFirstMonth() == 0) bTrigger << tr("STR_TRMONTH_FIRST");
	else bTrigger << tr("STR_TRMONTH_LATER") << " " << rule->getFirstMonth();
	if (rule->getFirstMonth() != rule->getLastMonth())
	{
		if (rule->getLastMonth() == -1) bTrigger << " " << tr("STR_TRSIGN_UP");
		else bTrigger << " " << tr("STR_TRSIGN_MID") << " " <<
			tr("STR_TRMONTH_LATER") << " " << rule->getLastMonth();
	}
	bool isValidTrigger = isValidMonthTrigger(0, rule);
	_lstLeft->addRow(1, bTrigger.str().c_str());
	_lstLeft->setRowColor(row, isValidTrigger ? _purple : _pink);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++; bTrigger.str(""); bTrigger.clear();

	// 1c. Min/Max Difficulty
	bTrigger << "  " << tr("STR_TRIGGER_DIFFICULTY") << " ";
	bTrigger << tr("STR_TRDIFF_LVL_" + std::to_string(rule->getMinDifficulty()));
	if (rule->getMinDifficulty() != rule->getMaxDifficulty())
	{
		bTrigger << " " << tr("STR_TRSIGN_MID") << " ";
		bTrigger << tr("STR_TRDIFF_LVL_" + std::to_string(rule->getMaxDifficulty()));
	}
	isValidTrigger = isValidDiffTrigger(0, rule);
	_lstLeft->addRow(1, bTrigger.str().c_str());
	_lstLeft->setRowColor(row, isValidTrigger ? _purple : _pink);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++; bTrigger.str(""); bTrigger.clear();

	// 1d. Min/Max Score
	bTrigger << "  " << tr("STR_TRIGGER_SCORE") << " ";
	if (rule->getMinScore() == INT_MIN && rule->getMaxScore() == INT_MAX)
		bTrigger << tr("STR_TRSCORE_NONE");
	else
	{
		if (rule->getMinScore() > INT_MIN) bTrigger << rule->getMinScore() << " ";
		else bTrigger << tr("STR_TRSIGN_UP") << " ";
		if (rule->getMinScore() > INT_MIN && rule->getMaxScore() < INT_MAX)
			bTrigger << tr("STR_TRSIGN_MID") << " ";
		if (rule->getMaxScore() < INT_MAX) bTrigger << rule->getMaxScore();
		else bTrigger << tr("STR_TRSIGN_UP");
	}
	isValidTrigger = isValidScoreTrigger(0, rule);
	_lstLeft->addRow(1, bTrigger.str().c_str());
	_lstLeft->setRowColor(row, isValidTrigger ? _purple : _pink);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++; bTrigger.str(""); bTrigger.clear();

	// 1e. Min/Max Funds
	bTrigger << "  " << tr("STR_TRIGGER_FUNDS") << " ";
	if (rule->getMinFunds() == INT64_MIN && rule->getMaxFunds() == INT64_MAX)
		bTrigger << tr("STR_TRFUNDS_NONE");
	else
	{
		if (rule->getMinFunds() > INT64_MIN) bTrigger <<
			Unicode::formatFunding(rule->getMinFunds()) << " ";
		else bTrigger << tr("STR_TRSIGN_UP") << " ";
		if (rule->getMinFunds() > INT64_MIN && rule->getMaxFunds() < INT64_MAX)
			bTrigger << tr("STR_TRSIGN_MID") << " ";
		if (rule->getMaxFunds() < INT64_MAX) bTrigger <<
			Unicode::formatFunding(rule->getMaxFunds());
		else bTrigger << tr("STR_TRSIGN_UP");
	}
	isValidTrigger = isValidFundsTrigger(0, rule);
	_lstLeft->addRow(1, bTrigger.str().c_str());
	_lstLeft->setRowColor(row, isValidTrigger ? _purple : _pink);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++; bTrigger.str(""); bTrigger.clear();

	// 1f. Min/Max Counter
	bTrigger << "  " << tr("STR_TRIGGER_COUNTER") << " ";
	if (rule->getMissionVarName().empty() && rule->getMissionMarkerName().empty())
		bTrigger << tr("STR_TRCOUNT_NONE");
	else if (rule->getCounterMin() == 0 && rule->getCounterMax() == -1)
		bTrigger << tr("STR_TRCOUNT_NONE");
	else
	{
		if (rule->getCounterMin() > 0) bTrigger << rule->getCounterMin() << " ";
		else bTrigger << tr("STR_TRSIGN_UP") << " ";
		if (rule->getCounterMin() > 0 && rule->getCounterMax() != -1)
			bTrigger << tr("STR_TRSIGN_MID") << " ";
		if (rule->getCounterMax() != 1) bTrigger << rule->getCounterMax();
	}
	isValidTrigger = isValidCounterTrigger(0, rule);
	_lstLeft->addRow(1, bTrigger.str().c_str());
	_lstLeft->setRowColor(row, isValidTrigger ? _purple : _pink);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++; bTrigger.str(""); bTrigger.clear();

	// 2. Research Trigger
	const auto& resTriggers = rule->getResearchTriggers();
	if (resTriggers.size() > 0)
	{
		_lstLeft->addRow(1, ""); _leftTopics.push_back("-"); _leftFlags.push_back(TTV_NONE); row++;
		_lstLeft->addRow(1, tr("STR_TRIGGERS_RESEARCH").c_str());
		_lstLeft->setRowColor(row, _white);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		row++;
		for (auto& resTrigger : resTriggers)
		{
			std::ostringstream rowRes;
			rowRes << "  " << tr(resTrigger.first);
			rowRes << ": " << std::boolalpha << resTrigger.second;
			bool isResearched = (_save->isResearched(resTrigger.first) == resTrigger.second);
			_lstLeft->addRow(1, rowRes.str().c_str());
			_lstLeft->setRowColor(row, isResearched ? _purple : _pink);
			_leftTopics.push_back("-");
			_leftFlags.push_back(TTV_NONE);
			row++;
		}
	}

	// 3. Item Triggers
	const auto& itemTriggers = rule->getItemTriggers();
	if (itemTriggers.size() > 0)
	{
		_lstLeft->addRow(1, ""); _leftTopics.push_back("-"); _leftFlags.push_back(TTV_NONE); row++;
		_lstLeft->addRow(1, tr("STR_TRIGGERS_ITEM").c_str());
		_lstLeft->setRowColor(row, _white);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		row++;
		for (auto& itemTrigger : itemTriggers)
		{
			std::ostringstream rowItem;
			rowItem << "  " << tr(itemTrigger.first);
			rowItem << ": " << std::boolalpha << itemTrigger.second;
			bool isObtained = (_save->isItemObtained(itemTrigger.first) == itemTrigger.second);
			_lstLeft->addRow(1, rowItem.str().c_str());
			_lstLeft->setRowColor(row, isObtained ? _purple : _pink);
			_leftTopics.push_back("-");
			_leftFlags.push_back(TTV_NONE);
			row++;
		}
	}

	// 4. Facility Triggers
	const auto& facTriggers = rule->getFacilityTriggers();
	if (facTriggers.size() > 0)
	{
		_lstLeft->addRow(1, ""); _leftTopics.push_back("-"); _leftFlags.push_back(TTV_NONE); row++;
		_lstLeft->addRow(1, tr("STR_TRIGGERS_FACILITY").c_str());
		_lstLeft->setRowColor(row, _white);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		row++;
		for (auto& facTrigger : facTriggers)
		{
			std::ostringstream rowFac;
			rowFac << "  " << tr(facTrigger.first);
			rowFac << ": " << std::boolalpha << facTrigger.second;
			bool isConstructed = (_save->isFacilityBuilt(facTrigger.first) == facTrigger.second);
			_lstLeft->addRow(1, rowFac.str().c_str());
			_lstLeft->setRowColor(row, isConstructed ? _purple : _pink);
			_leftTopics.push_back("-");
			_leftFlags.push_back(TTV_NONE);
			row++;
		}
	}

	// 5. Soldier Triggers
	const auto& crewTriggers = rule->getSoldierTypeTriggers();
	if (crewTriggers.size() > 0)
	{
		_lstLeft->addRow(1, ""); _leftTopics.push_back("-"); _leftFlags.push_back(TTV_NONE); row++;
		_lstLeft->addRow(1, tr("STR_TRIGGERS_SOLDIER").c_str());
		_lstLeft->setRowColor(row, _white);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		row++;
		for (auto& crewTrigger : crewTriggers)
		{
			std::ostringstream rowCrew;
			rowCrew << "  " << tr(crewTrigger.first);
			rowCrew << ": " << std::boolalpha << crewTrigger.second;
			bool isTypeHired = (_save->isSoldierTypeHired(crewTrigger.first) == crewTrigger.second);
			_lstLeft->addRow(1, rowCrew.str().c_str());
			_lstLeft->setRowColor(row, isTypeHired ? _purple : _pink);
			_leftTopics.push_back("-");
			_leftFlags.push_back(TTV_NONE);
			row++;
		}
	}

	// 6. Regional Triggers
	const auto& regTriggers = rule->getXcomBaseInRegionTriggers();
	const auto& terTriggers = rule->getXcomBaseInCountryTriggers();
	if (regTriggers.size() > 0 || terTriggers.size() > 0)
	{
		_lstLeft->addRow(1, ""); _leftTopics.push_back("-"); _leftFlags.push_back(TTV_NONE); row++;
		std::set<std::string> baseRegions;
		std::set<std::string> baseCountries;
		for (auto* xcomBase : *_save->getBases())
		{
			Region* region = _save->locateRegion(*xcomBase);
			if (region) baseRegions.insert(region->getRules()->getType());
			Country* country = _save->locateCountry(*xcomBase);
			if (country) baseCountries.insert(country->getRules()->getType());
		}
		_lstLeft->addRow(1, tr("STR_TRIGGERS_REGIONAL").c_str());
		_lstLeft->setRowColor(row, _white);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		row++;
		for (auto& regTrigger : regTriggers)
		{
			std::ostringstream rowReg;
			rowReg << "  " << tr(regTrigger.first);
			rowReg << ": " << std::boolalpha << regTrigger.second;
			bool isRegFound = (baseRegions.find(regTrigger.first) != baseRegions.end());
			bool isRegValid = isRegFound == regTrigger.second;
			_lstLeft->addRow(1, rowReg.str().c_str());
			_lstLeft->setRowColor(row, isRegValid ? _purple : _pink);
			_leftTopics.push_back("-");
			_leftFlags.push_back(TTV_NONE);
			row++;
		}
		for (auto& terTrigger : terTriggers)
		{
			std::ostringstream rowTer;
			rowTer << "  " << tr(terTrigger.first);
			rowTer << ": " << std::boolalpha << terTrigger.second;
			bool isTerFound = (baseCountries.find(terTrigger.first) != baseCountries.end());
			bool isTerValid = isTerFound == terTrigger.second;
			_lstLeft->addRow(1, rowTer.str().c_str());
			_lstLeft->setRowColor(row, isTerValid ? _purple : _pink);
			_leftTopics.push_back("-");
			_leftFlags.push_back(TTV_NONE);
			row++;
		}
	}

	row = 0; // Right UI Panel Switch

	// 7. One Time Sequential Events
	const auto& oneTimeSeqEvents = rule->getOneTimeSequentialEvents();
	if (oneTimeSeqEvents.size() > 0)
	{
		_lstRight->addRow(1, tr("STR_EVENT_ONETIME_SEQ").c_str());
		_lstRight->setRowColor(row, _white);
		_rightTopics.push_back("-");
		_rightFlags.push_back(TTV_NONE);
		row++;
		for (auto& eventSeq : oneTimeSeqEvents)
		{
			std::ostringstream rowSeq;
			rowSeq << "  ";
			strPush(rowSeq, eventSeq);
			_lstRight->addRow(1, rowSeq.str().c_str());
			_lstRight->setRowColor(row,
				_save->wasEventGenerated(eventSeq) ?
				_purple : _pink);
			_rightTopics.push_back("-");
			_rightFlags.push_back(TTV_NONE);
			row++;
		}
	}

	// 8. One Time Random Events
	const auto& oneTimeRandEvents = rule->getOneTimeRandomEvents().getChoices();
	if (oneTimeRandEvents.size() > 0)
	{
		if (oneTimeSeqEvents.size() > 0)
		{
			_lstRight->addRow(1, ""); _rightTopics.push_back("-");
			_rightFlags.push_back(TTV_NONE); row++;
		}
		_lstRight->addRow(1, tr("STR_EVENT_ONETIME_RAND").c_str());
		_lstRight->setRowColor(row, _white);
		_rightTopics.push_back("-");
		_rightFlags.push_back(TTV_NONE);
		row++;
		for (auto& eventRand : oneTimeRandEvents)
		{
			std::ostringstream rowRand;
			rowRand << "  ";
			strPush(rowRand, eventRand.first);
			rowRand << ": ";
			rowRand << eventRand.second;
			_lstRight->addRow(1, rowRand.str().c_str());
			_lstRight->setRowColor(row,
				_save->wasEventGenerated(eventRand.first) ?
				_purple : _pink);
			_rightTopics.push_back("-");
			_rightFlags.push_back(TTV_NONE);
			row++;
		}
	}

	// 9. Event Weights
	const auto& eventWeights = rule->getEventWeights();
	if (eventWeights.size() > 0)
	{
		if (oneTimeRandEvents.size() > 0)
		{
			_lstRight->addRow(1, ""); _rightTopics.push_back("-");
			_rightFlags.push_back(TTV_NONE); row++;
		}
		_lstRight->addRow(1, tr("STR_EVENT_WEIGHTS").c_str());
		_lstRight->setRowColor(row, _white);
		_rightTopics.push_back("-");
		_rightFlags.push_back(TTV_NONE);
		row++;
		for (auto evIt = eventWeights.begin(); evIt != eventWeights.end(); ++evIt)
		{
			auto& eventSet = *evIt;
			if (eventSet.second->getChoices().size() > 0)
			{
				std::ostringstream rowSet;
				rowSet << "  ";
				if (eventSet.first == 0) rowSet << tr("STR_CASE_AT_START");
				else rowSet << tr("STR_CASE_AT_MONTH").arg(eventSet.first);
				_lstRight->addRow(1, rowSet.str().c_str());
				_lstRight->setRowColor(row, _blue);
				_rightTopics.push_back("-");
				_rightFlags.push_back(TTV_NONE);
				row++;
				bool isCurrValid = _currMonth >= (int)eventSet.first;
				bool isNextValid = (std::next(evIt) != eventWeights.end()
					&& _currMonth >= (int)std::next(evIt)->first);
				auto& choiceColor = isCurrValid && !isNextValid ? _purple : _pink;
				for (auto& eventChoice : eventSet.second->getChoices())
				{
					std::ostringstream rowOpt;
					rowOpt << "    ";
					strPush(rowOpt, eventChoice.first);
					rowOpt << ": ";
					rowOpt << eventChoice.second;
					_lstRight->addRow(1, rowOpt.str().c_str());
					_lstRight->setRowColor(row, choiceColor);
					_rightTopics.push_back("-");
					_rightFlags.push_back(TTV_NONE);
					row++;
				}
			}
		}
	}
}

/**
 * Handles, renders and organizes Mission script data.
 */
void TechTreeViewerState::handleMissionScript()
{
	int row = 0;
	const RuleMissionScript *rule = _game->getMod()->getMissionScript(_selectedTopic);
	if (rule == 0)
		return;

	// 1. Basic Triggers
	_lstLeft->addRow(1, tr("STR_TRIGGERS_BASIC").c_str());
	_lstLeft->setRowColor(row, _white);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++;
	std::ostringstream bTrigger;

	// 1a. Execution Odds
	bTrigger << "  " << tr("STR_TRIGGER_ODDS") << " ";
	bTrigger << Unicode::formatPercentage(rule->getExecutionOdds());
	_lstLeft->addRow(1, bTrigger.str().c_str());
	_lstLeft->setRowColor(row, _gold);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++; bTrigger.str(""); bTrigger.clear();

	// 1b. First/Last Month
	bTrigger << "  " << tr("STR_TRIGGER_MONTH") << " ";
	if (rule->getFirstMonth() == 0) bTrigger << tr("STR_TRMONTH_FIRST");
	else bTrigger << tr("STR_TRMONTH_LATER") << " " << rule->getFirstMonth();
	if (rule->getFirstMonth() != rule->getLastMonth())
	{
		if (rule->getLastMonth() == -1) bTrigger << " " << tr("STR_TRSIGN_UP");
		else bTrigger << " " << tr("STR_TRSIGN_MID") << " " <<
			tr("STR_TRMONTH_LATER") << " " << rule->getLastMonth();
	}
	bool isValidTrigger = isValidMonthTrigger(0, 0, rule);
	_lstLeft->addRow(1, bTrigger.str().c_str());
	_lstLeft->setRowColor(row, isValidTrigger ? _purple : _pink);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++; bTrigger.str(""); bTrigger.clear();

	// 1c. Min/Max Difficulty
	bTrigger << "  " << tr("STR_TRIGGER_DIFFICULTY") << " ";
	bTrigger << tr("STR_TRDIFF_LVL_" + std::to_string(rule->getMinDifficulty()));
	if (rule->getMinDifficulty() < 4) bTrigger << " " << tr("STR_TRSIGN_UP");
	isValidTrigger = isValidDiffTrigger(0, 0, rule);
	_lstLeft->addRow(1, bTrigger.str().c_str());
	_lstLeft->setRowColor(row, isValidTrigger ? _purple : _pink);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++; bTrigger.str(""); bTrigger.clear();

	// 1d. Min/Max Score
	bTrigger << "  " << tr("STR_TRIGGER_SCORE") << " ";
	if (rule->getMinScore() == INT_MIN && rule->getMaxScore() == INT_MAX)
		bTrigger << tr("STR_TRSCORE_NONE");
	else
	{
		if (rule->getMinScore() > INT_MIN) bTrigger << rule->getMinScore() << " ";
		else bTrigger << tr("STR_TRSIGN_UP") << " ";
		if (rule->getMinScore() > INT_MIN && rule->getMaxScore() < INT_MAX)
			bTrigger << tr("STR_TRSIGN_MID") << " ";
		if (rule->getMaxScore() < INT_MAX) bTrigger << rule->getMaxScore();
		else bTrigger << tr("STR_TRSIGN_UP");
	}
	isValidTrigger = isValidScoreTrigger(0, 0, rule);
	_lstLeft->addRow(1, bTrigger.str().c_str());
	_lstLeft->setRowColor(row, isValidTrigger ? _purple : _pink);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++; bTrigger.str(""); bTrigger.clear();

	// 1e. Min/Max Funds
	bTrigger << "  " << tr("STR_TRIGGER_FUNDS") << " ";
	if (rule->getMinFunds() == INT64_MIN && rule->getMaxFunds() == INT64_MAX)
		bTrigger << tr("STR_TRFUNDS_NONE");
	else
	{
		if (rule->getMinFunds() > INT64_MIN) bTrigger <<
			Unicode::formatFunding(rule->getMinFunds()) << " ";
		else bTrigger << tr("STR_TRSIGN_UP") << " ";
		if (rule->getMinFunds() > INT64_MIN && rule->getMaxFunds() < INT64_MAX)
			bTrigger << tr("STR_TRSIGN_MID") << " ";
		if (rule->getMaxFunds() < INT64_MAX) bTrigger <<
			Unicode::formatFunding(rule->getMaxFunds());
		else bTrigger << tr("STR_TRSIGN_UP");
	}
	isValidTrigger = isValidFundsTrigger(0, 0, rule);
	_lstLeft->addRow(1, bTrigger.str().c_str());
	_lstLeft->setRowColor(row, isValidTrigger ? _purple : _pink);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++; bTrigger.str(""); bTrigger.clear();

	// 1f. Min/Max Counter
	bTrigger << "  " << tr("STR_TRIGGER_COUNTER") << " ";
	if (rule->getMissionVarName().empty() && rule->getMissionMarkerName().empty())
		bTrigger << tr("STR_TRCOUNT_NONE");
	else if (rule->getCounterMin() == 0 && rule->getCounterMax() == -1)
		bTrigger << tr("STR_TRCOUNT_NONE");
	else
	{
		if (rule->getCounterMin() > 0) bTrigger << rule->getCounterMin() << " ";
		else bTrigger << tr("STR_TRSIGN_UP") << " ";
		if (rule->getCounterMin() > 0 && rule->getCounterMax() != -1)
			bTrigger << tr("STR_TRSIGN_MID") << " ";
		if (rule->getCounterMax() != 1) bTrigger << rule->getCounterMax();
	}
	isValidTrigger = isValidCounterTrigger(0, 0, rule);
	_lstLeft->addRow(1, bTrigger.str().c_str());
	_lstLeft->setRowColor(row, isValidTrigger ? _purple : _pink);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++; bTrigger.str(""); bTrigger.clear();

	// 2. Mission Triggers
	_lstLeft->addRow(1, ""); _leftTopics.push_back("-"); _leftFlags.push_back(TTV_NONE); row++;
	_lstLeft->addRow(1, tr("STR_TRIGGERS_MISSION").c_str());
	_lstLeft->setRowColor(row, _white);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++;

	// 2a. Target Base Odds
	bTrigger << "  " << tr("STR_TRIGGER_BASE") << " ";
	bTrigger << Unicode::formatPercentage(rule->getTargetBaseOdds());
	_lstLeft->addRow(1, bTrigger.str().c_str());
	_lstLeft->setRowColor(row, _gold);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++; bTrigger.str(""); bTrigger.clear();

	// 2b. Execution Label
	bTrigger << "  " << tr("STR_TRIGGER_LABEL") << " ";
	if (rule->getLabel() == 0)
		bTrigger << tr("STR_TRLABEL_NONE");
	else bTrigger << rule->getLabel();
	_lstLeft->addRow(1, bTrigger.str().c_str());
	_lstLeft->setRowColor(row, _blue);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++; bTrigger.str(""); bTrigger.clear();

	// 2c. Start/Random Delay
	bTrigger << "  " << tr("STR_TRIGGER_DELAY") << " ";
	bTrigger << rule->getBaseDelay();
	if (rule->getRandomDelay() > 0)
	{
		bTrigger << " " << tr("STR_TRSIGN_MID");
		bTrigger << " " << (rule->getBaseDelay()
			+ rule->getRandomDelay());
	}
	bTrigger << " " << tr("STR_TRDELAY_MINUTES");
	_lstLeft->addRow(1, bTrigger.str().c_str());
	_lstLeft->setRowColor(row, _blue);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++; bTrigger.str(""); bTrigger.clear();

	// 2d. Conditionals
	bTrigger << "  " << tr("STR_TRIGGER_CONDS") << " ";
	const auto& numConditions = rule->getConditionals();
	if (numConditions.size() > 0)
	{
		for (size_t i = 0; i < numConditions.size(); ++i)
		{
			if (i != 0) bTrigger << ", ";
			bTrigger << numConditions[i];
		}
	}
	else bTrigger << tr("STR_TRCONDS_NONE");
	_lstLeft->addRow(1, bTrigger.str().c_str());
	_lstLeft->setRowColor(row, _blue);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++; bTrigger.str(""); bTrigger.clear();

	// 2e. Repeat Avoidance
	bTrigger << "  " << tr("STR_TRIGGER_REPEAT") << " ";
	if (rule->getRepeatAvoidance() == 0)
		bTrigger << tr("STR_TRREPEAT_NONE");
	else bTrigger << rule->getRepeatAvoidance();
	_lstLeft->addRow(1, bTrigger.str().c_str());
	_lstLeft->setRowColor(row, _blue);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++; bTrigger.str(""); bTrigger.clear();

	// 2f. Max Mission Runs
	bTrigger << "  " << tr("STR_TRIGGER_MAXRUNS") << " ";
	if (rule->getMaxRuns() == -1)
		bTrigger << tr("STR_TRMAXRUN_NONE");
	else bTrigger << rule->getMaxRuns();
	isValidTrigger = (rule->getMaxRuns() == -1 || rule->getMaxRuns() >
		_save->getAlienStrategy().getMissionsRun(rule->getVarName()));
	_lstLeft->addRow(1, bTrigger.str().c_str());
	_lstLeft->setRowColor(row, isValidTrigger ? _purple : _pink);
	_leftTopics.push_back("-");
	_leftFlags.push_back(TTV_NONE);
	row++; bTrigger.str(""); bTrigger.clear();

	// 3. Research Trigger
	const auto& resTriggers = rule->getResearchTriggers();
	if (resTriggers.size() > 0)
	{
		_lstLeft->addRow(1, ""); _leftTopics.push_back("-"); _leftFlags.push_back(TTV_NONE); row++;
		_lstLeft->addRow(1, tr("STR_TRIGGERS_RESEARCH").c_str());
		_lstLeft->setRowColor(row, _white);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		row++;
		for (auto& resTrigger : resTriggers)
		{
			std::ostringstream rowRes;
			rowRes << "  " << tr(resTrigger.first);
			rowRes << ": " << std::boolalpha << resTrigger.second;
			bool isResearched = (_save->isResearched(resTrigger.first) == resTrigger.second);
			_lstLeft->addRow(1, rowRes.str().c_str());
			_lstLeft->setRowColor(row, isResearched ? _purple : _pink);
			_leftTopics.push_back("-");
			_leftFlags.push_back(TTV_NONE);
			row++;
		}
	}

	// 4. Item Triggers
	const auto& itemTriggers = rule->getItemTriggers();
	if (itemTriggers.size() > 0)
	{
		_lstLeft->addRow(1, ""); _leftTopics.push_back("-"); _leftFlags.push_back(TTV_NONE); row++;
		_lstLeft->addRow(1, tr("STR_TRIGGERS_ITEM").c_str());
		_lstLeft->setRowColor(row, _white);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		row++;
		for (auto& itemTrigger : itemTriggers)
		{
			std::ostringstream rowItem;
			rowItem << "  " << tr(itemTrigger.first);
			rowItem << ": " << std::boolalpha << itemTrigger.second;
			bool isObtained = (_save->isItemObtained(itemTrigger.first) == itemTrigger.second);
			_lstLeft->addRow(1, rowItem.str().c_str());
			_lstLeft->setRowColor(row, isObtained ? _purple : _pink);
			_leftTopics.push_back("-");
			_leftFlags.push_back(TTV_NONE);
			row++;
		}
	}

	// 5. Facility Triggers
	const auto& facTriggers = rule->getFacilityTriggers();
	if (facTriggers.size() > 0)
	{
		_lstLeft->addRow(1, ""); _leftTopics.push_back("-"); _leftFlags.push_back(TTV_NONE); row++;
		_lstLeft->addRow(1, tr("STR_TRIGGERS_FACILITY").c_str());
		_lstLeft->setRowColor(row, _white);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		row++;
		for (auto& facTrigger : facTriggers)
		{
			std::ostringstream rowFac;
			rowFac << "  " << tr(facTrigger.first);
			rowFac << ": " << std::boolalpha << facTrigger.second;
			bool isConstructed = (_save->isFacilityBuilt(facTrigger.first) == facTrigger.second);
			_lstLeft->addRow(1, rowFac.str().c_str());
			_lstLeft->setRowColor(row, isConstructed ? _purple : _pink);
			_leftTopics.push_back("-");
			_leftFlags.push_back(TTV_NONE);
			row++;
		}
	}

	// 6. Regional Triggers
	const auto& regTriggers = rule->getXcomBaseInRegionTriggers();
	const auto& terTriggers = rule->getXcomBaseInCountryTriggers();
	if (regTriggers.size() > 0 || terTriggers.size() > 0)
	{
		_lstLeft->addRow(1, ""); _leftTopics.push_back("-"); _leftFlags.push_back(TTV_NONE); row++;
		std::set<std::string> baseRegions;
		std::set<std::string> baseCountries;
		for (auto* xcomBase : *_save->getBases())
		{
			Region* region = _save->locateRegion(*xcomBase);
			if (region) baseRegions.insert(region->getRules()->getType());
			Country* country = _save->locateCountry(*xcomBase);
			if (country) baseCountries.insert(country->getRules()->getType());
		}
		_lstLeft->addRow(1, tr("STR_TRIGGERS_REGIONAL").c_str());
		_lstLeft->setRowColor(row, _white);
		_leftTopics.push_back("-");
		_leftFlags.push_back(TTV_NONE);
		row++;
		for (auto& regTrigger : regTriggers)
		{
			std::ostringstream rowReg;
			rowReg << "  " << tr(regTrigger.first);
			rowReg << ": " << std::boolalpha << regTrigger.second;
			bool isRegFound = (baseRegions.find(regTrigger.first) != baseRegions.end());
			bool isRegValid = isRegFound == regTrigger.second;
			_lstLeft->addRow(1, rowReg.str().c_str());
			_lstLeft->setRowColor(row, isRegValid ? _purple : _pink);
			_leftTopics.push_back("-");
			_leftFlags.push_back(TTV_NONE);
			row++;
		}
		for (auto& terTrigger : terTriggers)
		{
			std::ostringstream rowTer;
			rowTer << "  " << tr(terTrigger.first);
			rowTer << ": " << std::boolalpha << terTrigger.second;
			bool isTerFound = (baseCountries.find(terTrigger.first) != baseCountries.end());
			bool isTerValid = isTerFound == terTrigger.second;
			_lstLeft->addRow(1, rowTer.str().c_str());
			_lstLeft->setRowColor(row, isTerValid ? _purple : _pink);
			_leftTopics.push_back("-");
			_leftFlags.push_back(TTV_NONE);
			row++;
		}
	}

	row = 0; // Right UI Panel Switch

	// 7. Mission Weights
	const auto& missionWeights = rule->getMissionWeights();
	if (missionWeights.size() > 0)
	{
		_lstRight->addRow(1, tr("STR_MISSION_WEIGHTS").c_str());
		_lstRight->setRowColor(row, _white);
		_rightTopics.push_back("-");
		_rightFlags.push_back(TTV_NONE);
		row++;
		for (auto misIt = missionWeights.begin(); misIt != missionWeights.end(); ++misIt)
		{
			auto& missionSet = *misIt;
			if (missionSet.second->getChoices().size() > 0)
			{
				std::ostringstream rowSet;
				rowSet << "  ";
				if (missionSet.first == 0) rowSet << tr("STR_CASE_AT_START");
				else rowSet << tr("STR_CASE_AT_MONTH").arg(missionSet.first);
				_lstRight->addRow(1, rowSet.str().c_str());
				_lstRight->setRowColor(row, _blue);
				_rightTopics.push_back("-");
				_rightFlags.push_back(TTV_NONE);
				row++;
				bool isCurrValid = _currMonth >= (int)missionSet.first;
				bool isNextValid = (std::next(misIt) != missionWeights.end()
					&& _currMonth >= (int)std::next(misIt)->first);
				auto& choiceColor = isCurrValid && !isNextValid ? _purple : _pink;
				for (auto& missionChoice : missionSet.second->getChoices())
				{
					std::ostringstream rowOpt;
					rowOpt << "    ";
					strPush(rowOpt, missionChoice.first);
					rowOpt << ": ";
					rowOpt << missionChoice.second;
					_lstRight->addRow(1, rowOpt.str().c_str());
					_lstRight->setRowColor(row, choiceColor);
					_rightTopics.push_back("-");
					_rightFlags.push_back(TTV_NONE);
					row++;
				}
			}
		}
	}

	// 8. Region Weights
	const auto& regionWeights = rule->getRegionWeights();
	if (regionWeights.size() > 0)
	{
		if (missionWeights.size() > 0)
		{
			_lstRight->addRow(1, ""); _rightTopics.push_back("-");
			_rightFlags.push_back(TTV_NONE); row++;
		}
		_lstRight->addRow(1, tr("STR_MISSION_REGIONS").c_str());
		_lstRight->setRowColor(row, _white);
		_rightTopics.push_back("-");
		_rightFlags.push_back(TTV_NONE);
		row++;
		for (auto regIt = regionWeights.begin(); regIt != regionWeights.end(); ++regIt)
		{
			auto& regionSet = *regIt;
			if (regionSet.second->getChoices().size() > 0)
			{
				std::ostringstream rowSet;
				rowSet << "  ";
				if (regionSet.first == 0) rowSet << tr("STR_CASE_AT_START");
				else rowSet << tr("STR_CASE_AT_MONTH").arg(regionSet.first);
				_lstRight->addRow(1, rowSet.str().c_str());
				_lstRight->setRowColor(row, _blue);
				_rightTopics.push_back("-");
				_rightFlags.push_back(TTV_NONE);
				row++;
				bool isCurrValid = _currMonth >= (int)regionSet.first;
				bool isNextValid = (std::next(regIt) != regionWeights.end()
					&& _currMonth >= (int)std::next(regIt)->first);
				auto& choiceColor = isCurrValid && !isNextValid ? _purple : _pink;
				for (auto& regionChoice : regionSet.second->getChoices())
				{
					std::ostringstream rowOpt;
					rowOpt << "    ";
					strPush(rowOpt, regionChoice.first);
					rowOpt << ": ";
					rowOpt << regionChoice.second;
					_lstRight->addRow(1, rowOpt.str().c_str());
					_lstRight->setRowColor(row, choiceColor);
					_rightTopics.push_back("-");
					_rightFlags.push_back(TTV_NONE);
					row++;
				}
			}
		}
	}

	// 9. Race Weights
	const auto& raceWeights = rule->getRaceWeights();
	if (raceWeights.size() > 0)
	{
		if (regionWeights.size() > 0)
		{
			_lstRight->addRow(1, ""); _rightTopics.push_back("-");
			_rightFlags.push_back(TTV_NONE); row++;
		}
		_lstRight->addRow(1, tr("STR_MISSION_RACES").c_str());
		_lstRight->setRowColor(row, _white);
		_rightTopics.push_back("-");
		_rightFlags.push_back(TTV_NONE);
		row++;
		for (auto raceIt = raceWeights.begin(); raceIt != raceWeights.end(); ++raceIt)
		{
			auto& raceSet = *raceIt;
			if (raceSet.second->getChoices().size() > 0)
			{
				std::ostringstream rowSet;
				rowSet << "  ";
				if (raceSet.first == 0) rowSet << tr("STR_CASE_AT_START");
				else rowSet << tr("STR_CASE_AT_MONTH").arg(raceSet.first);
				_lstRight->addRow(1, rowSet.str().c_str());
				_lstRight->setRowColor(row, _blue);
				_rightTopics.push_back("-");
				_rightFlags.push_back(TTV_NONE);
				row++;
				bool isCurrValid = _currMonth >= (int)raceSet.first;
				bool isNextValid = (std::next(raceIt) != raceWeights.end()
					&& _currMonth >= (int)std::next(raceIt)->first);
				auto& choiceColor = isCurrValid && !isNextValid ? _purple : _pink;
				for (auto& raceChoice : raceSet.second->getChoices())
				{
					std::ostringstream rowOpt;
					rowOpt << "    ";
					strPush(rowOpt, raceChoice.first);
					rowOpt << ": ";
					rowOpt << raceChoice.second;
					_lstRight->addRow(1, rowOpt.str().c_str());
					_lstRight->setRowColor(row, choiceColor);
					_rightTopics.push_back("-");
					_rightFlags.push_back(TTV_NONE);
					row++;
				}
			}
		}
	}
}

/**
* Is month trigger for Arc/Event/Mission Script valid?
*/
bool TechTreeViewerState::isValidMonthTrigger(const RuleArcScript *ruleArc, const RuleEventScript *ruleEvent, const RuleMissionScript *ruleMission) const
{
	if (ruleArc != 0)
	{
		return (ruleArc->getFirstMonth() <= _currMonth &&
			(ruleArc->getLastMonth() >= _currMonth ||
				ruleArc->getLastMonth() == -1));
	}
	else if (ruleEvent != 0)
	{
		return (ruleEvent->getFirstMonth() <= _currMonth &&
			(ruleEvent->getLastMonth() >= _currMonth ||
				ruleEvent->getLastMonth() == -1));
	}
	else if (ruleMission != 0)
	{
		return (ruleMission->getFirstMonth() <= _currMonth &&
			(ruleMission->getLastMonth() >= _currMonth ||
				ruleMission->getLastMonth() == -1));
	}
	return false;
}

/**
* Is difficulty trigger for Arc/Event/Mission Script valid?
*/
bool TechTreeViewerState::isValidDiffTrigger(const RuleArcScript *ruleArc, const RuleEventScript *ruleEvent, const RuleMissionScript *ruleMission) const
{
	if (ruleArc != 0)
	{
		return (ruleArc->getMinDifficulty() <= _currDiff &&
			ruleArc->getMaxDifficulty() >= _currDiff);
	}
	else if (ruleEvent != 0)
	{
		return (ruleEvent->getMinDifficulty() <= _currDiff &&
			ruleEvent->getMaxDifficulty() >= _currDiff);
	}
	else if (ruleMission != 0)
	{
		return (ruleMission->getMinDifficulty() <= _currDiff);
	}
	return false;
}

/**
* Is score trigger for Arc/Event/Mission Script valid?
*/
bool TechTreeViewerState::isValidScoreTrigger(const RuleArcScript *ruleArc, const RuleEventScript *ruleEvent, const RuleMissionScript *ruleMission) const
{
	if (ruleArc != 0)
	{
		return (ruleArc->getMinScore() <= _currScore &&
			ruleArc->getMaxScore() >= _currScore);
	}
	else if (ruleEvent != 0)
	{
		return (ruleEvent->getMinScore() <= _currScore &&
			ruleEvent->getMaxScore() >= _currScore);
	}
	else if (ruleMission != 0)
	{
		return (ruleMission->getMinScore() <= _currScore &&
			ruleMission->getMaxScore() >= _currScore);
	}
	return false;
}

/**
* Is funds trigger for Arc/Event/Mission Script valid?
*/
bool TechTreeViewerState::isValidFundsTrigger(const RuleArcScript *ruleArc, const RuleEventScript *ruleEvent, const RuleMissionScript *ruleMission) const
{
	if (ruleArc != 0)
	{
		return (ruleArc->getMinFunds() <= _currFunds &&
			ruleArc->getMaxFunds() >= _currFunds);
	}
	else if (ruleEvent != 0)
	{
		return (ruleEvent->getMinFunds() <= _currFunds &&
			ruleEvent->getMaxFunds() >= _currFunds);
	}
	else if (ruleMission != 0)
	{
		return (ruleMission->getMinFunds() <= _currFunds &&
			ruleMission->getMaxFunds() >= _currFunds);
	}
	return false;
}

/**
* Is counter trigger for Arc/Event/Mission Script valid?
*/
bool TechTreeViewerState::isValidCounterTrigger(const RuleArcScript *ruleArc, const RuleEventScript *ruleEvent, const RuleMissionScript *ruleMission) const
{
	if (ruleArc != 0)
	{
		if (ruleArc->getCounterMin() > 0)
		{
			if (!ruleArc->getMissionVarName().empty() && ruleArc->getCounterMin() >
				_save->getAlienStrategy().getMissionsRun(ruleArc->getMissionVarName()))
				return false;
			if (!ruleArc->getMissionMarkerName().empty() && ruleArc->getCounterMin() >
				_save->getLastId(ruleArc->getMissionMarkerName()))
				return false;
		}
		if (ruleArc->getCounterMax() != -1)
		{
			if (!ruleArc->getMissionVarName().empty() && ruleArc->getCounterMax() <
				_save->getAlienStrategy().getMissionsRun(ruleArc->getMissionVarName()))
				return false;
			if (!ruleArc->getMissionMarkerName().empty() && ruleArc->getCounterMax() <
				_save->getLastId(ruleArc->getMissionMarkerName()))
				return false;
		}
		return true;
	}
	else if (ruleEvent != 0)
	{
		if (ruleEvent->getCounterMin() > 0)
		{
			if (!ruleEvent->getMissionVarName().empty() && ruleEvent->getCounterMin() >
				_save->getAlienStrategy().getMissionsRun(ruleEvent->getMissionVarName()))
				return false;
			if (!ruleEvent->getMissionMarkerName().empty() && ruleEvent->getCounterMin() >
				_save->getLastId(ruleEvent->getMissionMarkerName()))
				return false;
		}
		if (ruleEvent->getCounterMax() != -1)
		{
			if (!ruleEvent->getMissionVarName().empty() && ruleEvent->getCounterMax() <
				_save->getAlienStrategy().getMissionsRun(ruleEvent->getMissionVarName()))
				return false;
			if (!ruleEvent->getMissionMarkerName().empty() && ruleEvent->getCounterMax() <
				_save->getLastId(ruleEvent->getMissionMarkerName()))
				return false;
		}
		return true;
	}
	else if (ruleMission != 0)
	{
		if (ruleMission->getCounterMin() > 0)
		{
			if (!ruleMission->getMissionVarName().empty() && ruleMission->getCounterMin() >
				_save->getAlienStrategy().getMissionsRun(ruleMission->getMissionVarName()))
				return false;
			if (!ruleMission->getMissionMarkerName().empty() && ruleMission->getCounterMin() >
				_save->getLastId(ruleMission->getMissionMarkerName()))
				return false;
		}
		if (ruleMission->getCounterMax() != -1)
		{
			if (!ruleMission->getMissionVarName().empty() && ruleMission->getCounterMax() <
				_save->getAlienStrategy().getMissionsRun(ruleMission->getMissionVarName()))
				return false;
			if (!ruleMission->getMissionMarkerName().empty() && ruleMission->getCounterMax() <
				_save->getLastId(ruleMission->getMissionMarkerName()))
				return false;
		}
		return true;
	}
	return false;
}

/**
 * Are the defining triggers of the Arc valid?
 * @param string name of the Arc Script.
 */
bool TechTreeViewerState::isPossibleArc(const RuleArcScript* ruleArc) const
{
	if (isValidDiffTrigger(ruleArc) &&
		isValidMonthTrigger(ruleArc))
		return true;
	return false;
}

/**
 * Are the defining triggers of the Event valid?
 * @param string name of the Event Script.
 */
bool TechTreeViewerState::isPossibleEvent(const RuleEventScript* ruleEvent) const
{
	if (isValidDiffTrigger(0, ruleEvent) &&
		isValidMonthTrigger(0, ruleEvent))
		return true;
	return false;
}

/**
 * Are the defining triggers of the Mission valid?
 * @param string name of the Mission Script.
 */
bool TechTreeViewerState::isPossibleMission(const RuleMissionScript* ruleMission) const
{
	if (isValidDiffTrigger(0, 0, ruleMission) &&
		isValidMonthTrigger(0, 0, ruleMission))
		return true;
	return false;
}

/**
* Selects the topic.
* @param action Pointer to an action.
*/
void TechTreeViewerState::onSelectLeftTopic(Action *)
{
	int index = _lstLeft->getSelectedRow();
	if (_leftFlags[index] > TTV_NONE)
	{
		_history.push_back(std::make_pair(_selectedTopic, _selectedFlag));
		_selectedFlag = _leftFlags[index];
		_selectedTopic = _leftTopics[index];
		initLists();
	}
}

/**
* Selects the topic.
* @param action Pointer to an action.
*/
void TechTreeViewerState::onSelectRightTopic(Action *)
{
	int index = _lstRight->getSelectedRow();
	if (_rightFlags[index] > TTV_NONE)
	{
		_history.push_back(std::make_pair(_selectedTopic, _selectedFlag));
		_selectedFlag = _rightFlags[index];
		_selectedTopic = _rightTopics[index];
		initLists();
	}
}

/**
 * Selects the topic.
 * @param action Pointer to an action.
 */
void TechTreeViewerState::onSelectFullTopic(Action *)
{
	int index = _lstFull->getSelectedRow();
	if (_leftFlags[index] > TTV_NONE)
	{
		_history.push_back(std::make_pair(_selectedTopic, _selectedFlag));
		_selectedFlag = _leftFlags[index];
		_selectedTopic = _leftTopics[index];
		initLists();
	}
}

/**
* Changes the selected topic.
*/
void TechTreeViewerState::setSelectedTopic(const std::string &selectedTopic, TTVMode topicType)
{
	_history.push_back(std::make_pair(_selectedTopic, _selectedFlag));
	_selectedTopic = selectedTopic;
	_selectedFlag = topicType;
}

/**
 * Gets the color coding for the given research topic.
 */
Uint8 TechTreeViewerState::getResearchColor(const std::string &topic) const
{
	if (_disabledResearch.find(topic) != _disabledResearch.end())
	{
		return _grey; // disabled
	}
	if (_alreadyAvailableResearch.find(topic) == _alreadyAvailableResearch.end())
	{
		return _pink; // not discovered
	}
	return _purple; // discovered
}

/**
 * Gets the alternative color coding for the given research topic.
 */
Uint8 TechTreeViewerState::getAltResearchColor(const std::string &topic) const
{
	if (_disabledResearch.find(topic) != _disabledResearch.end())
	{
		return _grey; // disabled
	}
	if (_alreadyAvailableResearch.find(topic) == _alreadyAvailableResearch.end())
	{
		return _gold; // not discovered
	}
	return _white; // discovered
}

/**
* Is given research topic discovered/available?
*/
bool TechTreeViewerState::isDiscoveredResearch(const std::string &topic) const
{
	if (_alreadyAvailableResearch.find(topic) == _alreadyAvailableResearch.end())
	{
		return false;
	}
	return true;
}

/**
* Is given manufacture topic discovered/available?
*/
bool TechTreeViewerState::isDiscoveredManufacture(const std::string &topic) const
{
	if (_alreadyAvailableManufacture.find(topic) == _alreadyAvailableManufacture.end())
	{
		return false;
	}
	return true;
}

/**
* Is given base facility discovered/available?
*/
bool TechTreeViewerState::isDiscoveredFacility(const std::string &topic) const
{
	if (_alreadyAvailableFacilities.find(topic) == _alreadyAvailableFacilities.end())
	{
		return false;
	}
	return true;
}

/**
* Is given item protected by any research?
*/
bool TechTreeViewerState::isProtectedItem(const std::string &topic) const
{
	if (_protectedItems.find(topic) == _protectedItems.end())
	{
		return false;
	}
	return true;
}

/**
* Is given protected item discovered/available for both purchase and usage/equipment?
*/
bool TechTreeViewerState::isProtectedAndDiscoveredItem(const std::string &topic) const
{
	if (_alreadyAvailableItems.find(topic) == _alreadyAvailableItems.end())
	{
		return false;
	}
	return true;
}

/**
* Is given craft discovered/available?
*/
bool TechTreeViewerState::isDiscoveredCraft(const std::string &topic) const
{
	if (_alreadyAvailableCrafts.find(topic) == _alreadyAvailableCrafts.end())
	{
		return false;
	}
	return true;
}

/**
 * Are all basic triggers of the Arc valid?
 * @param string name of the Arc Script.
 */
bool TechTreeViewerState::isGuaranteedArc(const std::string &strArc) const
{
	const RuleArcScript *ruleArc =
		_game->getMod()->getArcScript(strArc);
	if (isValidDiffTrigger(ruleArc) &&
		isValidMonthTrigger(ruleArc) &&
		isValidScoreTrigger(ruleArc) &&
		isValidFundsTrigger(ruleArc))
		return true;
	return false;
}

/**
 * Are all basic triggers of the Event valid?
 * @param string name of the Event Script.
 */
bool TechTreeViewerState::isGuaranteedEvent(const std::string &strEvent) const
{
	const RuleEventScript *ruleEvent =
		_game->getMod()->getEventScript(strEvent);
	if (isValidDiffTrigger(0, ruleEvent) &&
		isValidMonthTrigger(0, ruleEvent) &&
		isValidScoreTrigger(0, ruleEvent) &&
		isValidFundsTrigger(0, ruleEvent))
		return true;
	return false;
}

/**
 * Are all basic triggers of the Mission valid?
 * @param string name of the Mission Script.
 */
bool TechTreeViewerState::isGuaranteedMission(const std::string &strMission) const
{
	const RuleMissionScript *ruleMission =
		_game->getMod()->getMissionScript(strMission);
	if (isValidDiffTrigger(0, 0, ruleMission) &&
		isValidMonthTrigger(0, 0, ruleMission) &&
		isValidScoreTrigger(0, 0, ruleMission) &&
		isValidFundsTrigger(0, 0, ruleMission))
		return true;
	return false;
}

/**
 * Processes string based on options and appends it to stream.
 * @param stream reference, where to new value will be pushed.
 * @param original string reference.
 */
void TechTreeViewerState::strPush(std::ostringstream& refStream, const std::string& origString)
{
	std::string newString = origString;
	size_t trLen = Options::oxceDataViewStringTrunc;
	if (Options::oxceDataViewStrTranslate) newString = tr(newString);
	if (Options::oxceDataViewCleanReplace) newString = cleanStr(newString);
	if (trLen == 0 || newString.size() <= trLen) refStream << newString;
	else refStream << newString.substr(newString.size() - trLen);
}

/**
 * Replaces all underscores with spaces for better word wrapping.
 * @param stream reference, where to new value will be pushed.
 * @param original string reference.
 * @return new string without underscores.
 */
const std::string TechTreeViewerState::cleanStr(const std::string& origString)
{
	std::string cleanString = origString;
	size_t pos = cleanString.find("STR_"); // Removing "STR_" from string.
	if (pos != std::string::npos && pos == 0) cleanString.erase(pos, 4);
	std::replace(cleanString.begin(), cleanString.end(), '_', ' ');
	// Data Viewer entries are unlikely to use '_' so it should be fine.
	return cleanString;
}

}
