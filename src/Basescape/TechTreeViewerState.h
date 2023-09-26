#pragma once
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
#include "../Engine/State.h"
#include <vector>
#include <map>
#include <string>
#include <unordered_set>

namespace OpenXcom
{

class TextButton;
class Window;
class Text;
class TextList;
class RuleResearch;
class RuleManufacture;
class RuleBaseFacility;
class RuleCraft;
class RuleArcScript;
class RuleEventScript;
class RuleMissionScript;
class SavedGame;

enum TTVMode { TTV_NONE, TTV_RESEARCH, TTV_MANUFACTURING, TTV_FACILITIES, TTV_ITEMS, TTV_CRAFTS, TTV_ARCS, TTV_EVENTS, TTV_MISSIONS };
enum GameDifficulty : int;

/**
 * TechTreeViewer screen, where you can browse the Tech Tree.
 */
class TechTreeViewerState : public State
{
private:
	TextButton *_btnOk, *_btnNew;
	Window *_window;
	Text *_txtTitle, *_txtSelectedTopic, *_txtProgress, *_txtCostIndicator;
	TextList *_lstLeft, *_lstRight, *_lstFull;
	Uint8 _purple, _pink, _blue, _white, _gold, _grey;
	std::string _selectedTopic;
	TTVMode _selectedFlag;
	std::vector<std::pair<std::string, TTVMode> > _history;
	std::vector<std::string> _leftTopics, _rightTopics;
	std::vector<TTVMode> _leftFlags, _rightFlags;
	std::unordered_set<std::string> _disabledResearch;
	std::unordered_set<std::string> _alreadyAvailableResearch, _alreadyAvailableManufacture, _alreadyAvailableFacilities, _alreadyAvailableCrafts;
	std::unordered_set<std::string> _protectedItems, _alreadyAvailableItems;
	std::unordered_set<std::string> _listArcScripts, _listEventScripts, _listMissionScripts;
	int _currMonth, _currScore;
	GameDifficulty _currDiff;
	int64_t _currFunds;
	SavedGame *_save;
	void initLists();
	void handleResearchData();
	void handleManufactureData();
	void handleFacilityData();
	void handleItemData();
	void handleCraftData();
	void handleArcScript();
	void handleEventScript();
	void handleMissionScript();
	bool isValidMonthTrigger(const RuleArcScript *ruleArc = 0, const RuleEventScript *ruleEvent = 0, const RuleMissionScript *ruleMission = 0) const;
	bool isValidDiffTrigger(const RuleArcScript *ruleArc = 0, const RuleEventScript *ruleEvent = 0, const RuleMissionScript *ruleMission = 0) const;
	bool isValidScoreTrigger(const RuleArcScript *ruleArc = 0, const RuleEventScript *ruleEvent = 0, const RuleMissionScript *ruleMission = 0) const;
	bool isValidFundsTrigger(const RuleArcScript *ruleArc = 0, const RuleEventScript *ruleEvent = 0, const RuleMissionScript *ruleMission = 0) const;
	bool isValidCounterTrigger(const RuleArcScript *ruleArc = 0, const RuleEventScript *ruleEvent = 0, const RuleMissionScript *ruleMission = 0) const;
	bool isPossibleArc(const RuleArcScript* ruleArc) const;
	bool isPossibleEvent(const RuleEventScript* ruleEvent) const;
	bool isPossibleMission(const RuleMissionScript* ruleMission) const;
	void onSelectLeftTopic(Action *action);
	void onSelectRightTopic(Action *action);
	void onSelectFullTopic(Action *action);
public:
	/// Creates the Tech Tree Viewer state.
	TechTreeViewerState(const RuleResearch *r = 0, const RuleManufacture *m = 0, const RuleBaseFacility *f = 0, const RuleCraft *c = 0,
		const RuleArcScript *as = 0, const RuleEventScript *es = 0, const RuleMissionScript *ms = 0);
	/// Cleans up the Tech Tree Viewer state.
	~TechTreeViewerState();
	/// Initializes the state.
	void init() override;
	/// Handler for clicking the OK button.
	void btnOkClick(Action *action);
	/// Handler for clicking the Back button.
	void btnBackClick(Action *action);
	/// Handler for clicking the New button.
	void btnNewClick(Action *action);
	/// Sets the selected topic.
	void setSelectedTopic(const std::string &selectedTopic, TTVMode topicType);
	/// Gets the color coding for the given research topic.
	Uint8 getResearchColor(const std::string &topic) const;
	/// Gets the alternative color coding for the given research topic.
	Uint8 getAltResearchColor(const std::string &topic) const;
	/// Is given research topic discovered/available?
	bool isDiscoveredResearch(const std::string &topic) const;
	/// Is given manufacture topic discovered/available?
	bool isDiscoveredManufacture(const std::string &topic) const;
	/// Is given base facility discovered/available?
	bool isDiscoveredFacility(const std::string &topic) const;
	/// Is given item protected by any research?
	bool isProtectedItem(const std::string &topic) const;
	/// Is given protected item discovered/available for both purchase and usage/equipment?
	bool isProtectedAndDiscoveredItem(const std::string &topic) const;
	/// Is given craft discovered/available for both purchase and usage/equipment?
	bool isDiscoveredCraft(const std::string &topic) const;
	/// Are all basic triggers of the Arc valid?
	bool isGuaranteedArc(const RuleArcScript* ruleArc) const;
	/// Are all basic triggers of the Event valid?
	bool isGuaranteedEvent(const RuleEventScript* ruleEvent) const;
	/// Are all basic triggers of the Mission valid?
	bool isGuaranteedMission(const RuleMissionScript* ruleMission) const;
	/// Appends to stream reverse-truncated string based on option value.
	void strCut(std::ostringstream& refStream, const std::string& origString);
	/// Replaces all underscores with spaces for better word wrapping.
	const std::string cleanStr(const std::string& origString);
};

}
