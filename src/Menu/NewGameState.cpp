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
#include "NewGameState.h"
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Interface/TextButton.h"
#include "../Interface/ToggleTextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Geoscape/GeoscapeState.h"
#include "../Geoscape/BuildNewBaseState.h"
#include "../Geoscape/BaseNameState.h"
#include "../Basescape/PlaceLiftState.h"
#include "../Engine/Options.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Base.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Difficulty window.
 * @param game Pointer to the core game.
 */
NewGameState::NewGameState()
{
	_customBaseMode = Options::oxceStartingBaseSelection;

	// Create objects
	if (_customBaseMode)
	{
		_window = new Window(this, 300, 200, 10, 0, POPUP_VERTICAL);
		_winBaseSel = new Window(this, 125, 155, 185, 0, POPUP_NONE);
		_winBaseDesc = new Window(this, 300, 50, 10, 150, POPUP_NONE);
		_btnBeginner = new TextButton(160, 16, 20, 21);
		_btnExperienced = new TextButton(160, 16, 20, 39);
		_btnVeteran = new TextButton(160, 16, 20, 57);
		_btnGenius = new TextButton(160, 16, 20, 75);
		_btnSuperhuman = new TextButton(160, 16, 20, 93);
		_btnIronman = new ToggleTextButton(78, 16, 20, 111);
		_btnOk = new TextButton(78, 16, 20, 129);
		_btnCancel = new TextButton(78, 16, 102, 129);
		_txtTitle = new Text(160, 9, 20, 9);
		_txtIronman = new Text(83, 24, 102, 107);
		_txtBaseTitle = new Text(105, 9, 195, 9);
		_txtBaseDesc = new Text(280, 33, 20, 159);
		_lstBaseSets = new TextList(115, 120, 190, 25);
	}
	else
	{
		_window = new Window(this, 192, 180, 64, 10, POPUP_VERTICAL);
		_btnBeginner = new TextButton(160, 18, 80, 32);
		_btnExperienced = new TextButton(160, 18, 80, 52);
		_btnVeteran = new TextButton(160, 18, 80, 72);
		_btnGenius = new TextButton(160, 18, 80, 92);
		_btnSuperhuman = new TextButton(160, 18, 80, 112);
		_btnIronman = new ToggleTextButton(78, 18, 80, 138);
		_btnOk = new TextButton(78, 16, 80, 164);
		_btnCancel = new TextButton(78, 16, 162, 164);
		_txtTitle = new Text(192, 9, 64, 20);
		_txtIronman = new Text(90, 24, 162, 135);
	}

	switch (_game->getMod()->getStartingDifficulty())
	{
	case 0:
		_difficulty = _btnBeginner;
		break;
	case 1:
		_difficulty = _btnExperienced;
		break;
	case 2:
		_difficulty = _btnVeteran;
		break;
	case 3:
		_difficulty = _btnGenius;
		break;
	case 4:
		_difficulty = _btnSuperhuman;
		break;
	default:
		_difficulty = _btnBeginner;
		break;
	}

	// Set palette
	setInterface("newGameMenu");

	add(_window, "window", "newGameMenu");
	if (_customBaseMode)
	{
		add(_winBaseSel, "window", "newGameMenu");
		add(_winBaseDesc, "window", "newGameMenu");
	}
	add(_btnBeginner, "button", "newGameMenu");
	add(_btnExperienced, "button", "newGameMenu");
	add(_btnVeteran, "button", "newGameMenu");
	add(_btnGenius, "button", "newGameMenu");
	add(_btnSuperhuman, "button", "newGameMenu");
	add(_btnIronman, "ironman", "newGameMenu");
	add(_btnOk, "button", "newGameMenu");
	add(_btnCancel, "button", "newGameMenu");
	add(_txtTitle, "text", "newGameMenu");
	add(_txtIronman, "ironman", "newGameMenu");
	if (_customBaseMode)
	{
		add(_txtBaseTitle, "text", "newGameMenu");
		add(_txtBaseDesc, "text", "newGameMenu");
		add(_lstBaseSets, "list", "newGameMenu");
	}

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "newGameMenu");
	if (_customBaseMode)
	{
		setWindowBackground(_winBaseSel, "newGameMenu");
		setWindowBackground(_winBaseDesc, "newGameMenu");
	}

	_btnBeginner->setText(tr("STR_1_BEGINNER"));
	_btnBeginner->setGroup(&_difficulty);

	_btnExperienced->setText(tr("STR_2_EXPERIENCED"));
	_btnExperienced->setGroup(&_difficulty);

	_btnVeteran->setText(tr("STR_3_VETERAN"));
	_btnVeteran->setGroup(&_difficulty);

	_btnGenius->setText(tr("STR_4_GENIUS"));
	_btnGenius->setGroup(&_difficulty);

	_btnSuperhuman->setText(tr("STR_5_SUPERHUMAN"));
	_btnSuperhuman->setGroup(&_difficulty);

	_btnIronman->setText(tr("STR_IRONMAN"));

	_btnOk->setText(tr("STR_OK"));
	_btnOk->onMouseClick((ActionHandler)&NewGameState::btnOkClick);
	_btnOk->onKeyboardPress((ActionHandler)&NewGameState::btnOkClick, Options::keyOk);

	_btnCancel->setText(tr("STR_CANCEL"));
	_btnCancel->onMouseClick((ActionHandler)&NewGameState::btnCancelClick);
	_btnCancel->onKeyboardPress((ActionHandler)&NewGameState::btnCancelClick, Options::keyCancel);

	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr("STR_SELECT_DIFFICULTY_LEVEL"));

	_txtIronman->setWordWrap(true);
	_txtIronman->setVerticalAlign(ALIGN_MIDDLE);
	_txtIronman->setText(tr("STR_IRONMAN_DESC"));

	if (_customBaseMode)
	{
		_txtBaseTitle->setAlign(ALIGN_CENTER);
		_txtBaseTitle->setText(tr("STR_STARTING_BASE_LIST"));

		_txtBaseDesc->setWordWrap(true);
		_txtBaseDesc->setText(tr("STR_INIT_BASE_DEFAULT_DESC"));

		_lstBaseSets->setColumns(1, 95);
		_lstBaseSets->setMargin(5);
		_lstBaseSets->setSelectable(true);
		_lstBaseSets->setScrolling(true, -13);
		_lstBaseSets->setBackground(_window);
		_lstBaseSets->onMouseClick((ActionHandler)&NewGameState::lstBaseSetsClick);

		NewGameState::initList(0);
	}
}

/**
 *
 */
NewGameState::~NewGameState()
{

}

/**
 * Shows the crafts in a list at specified offset/scroll.
 */
void NewGameState::initList(size_t scrl)
{
	_lstBaseSets->clearList();

	// Adding default starting base to the list
	_lstBaseSets->addRow(1, tr(
		_game->getMod()->getDefaultStartingBaseSet()->Name).c_str());
	_lstBaseSets->setRowColor(0, _lstBaseSets->getSecondaryColor());
	_lastRow = 0;

	// Adding all other starting bases to the list
	for (const auto& baseSet : _game->getMod()->getStartingBaseSetsList())
	{
		_lstBaseSets->addRow(1, tr(_game->getMod()->getStartingBaseSet(baseSet)->Name).c_str());
	}

	// Setting the initial scroll position
	if (scrl) _lstBaseSets->scrollTo(scrl);
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void NewGameState::btnOkClick(Action *)
{
	GameDifficulty diff = DIFF_BEGINNER;
	if (_difficulty == _btnBeginner)
	{
		diff = DIFF_BEGINNER;
	}
	else if (_difficulty == _btnExperienced)
	{
		diff = DIFF_EXPERIENCED;
	}
	else if (_difficulty == _btnVeteran)
	{
		diff = DIFF_VETERAN;
	}
	else if (_difficulty == _btnGenius)
	{
		diff = DIFF_GENIUS;
	}
	else if (_difficulty == _btnSuperhuman)
	{
		diff = DIFF_SUPERHUMAN;
	}

	// Reset touch flags
	_game->resetTouchButtonFlags();

	// Switch to custom starting base, if relevant
	if (_customBaseMode && _lastRow > 0)
	{
		_game->getMod()->setStartingBase(
			_game->getMod()->getStartingBaseSet(
				_game->getMod()->getStartingBaseSetsList()
				.at(_lastRow - 1)));
	}
	else
	{
		_game->getMod()->setStartingBase(
			_game->getMod()->getDefaultStartingBaseSet());
	}

	SavedGame *save = _game->getMod()->newSave(diff);
	save->setDifficulty(diff);
	save->setIronman(_btnIronman->getPressed());
	_game->setSavedGame(save);

	GeoscapeState *gs = new GeoscapeState;
	_game->setState(gs);
	gs->init();

	auto* base = _game->getSavedGame()->getBases()->back();
	if (base->getMarker() != -1)
	{
		// location known already
		base->calculateServices(save);

		// center and rotate 35 degrees down (to see the base location while typoing its name)
		gs->getGlobe()->center(base->getLongitude(), base->getLatitude() + 0.61);

		if (base->getName().empty())
		{
			// fixed location, custom name
			_game->pushState(new BaseNameState(base, gs->getGlobe(), true, true));
		}
		else if (Options::customInitialBase)
		{
			// fixed location, fixed name
			_game->pushState(new PlaceLiftState(base, gs->getGlobe(), true));
		}
	}
	else
	{
		// custom location, custom name
		_game->pushState(new BuildNewBaseState(base, gs->getGlobe(), true));
	}
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void NewGameState::btnCancelClick(Action *)
{
	_game->setSavedGame(0);
	_game->popState();
}

/**
 * Selects starting base from the highlighted row.
 * @param action Pointer to an action.
 */
void NewGameState::lstBaseSetsClick(Action *)
{
	if (!_customBaseMode) return;
	auto row = _lstBaseSets->getSelectedRow();

	if (row == 0)
	{
		_txtBaseDesc->setText(tr(
			_game->getMod()->getDefaultStartingBaseSet()->Name + "_DESC"));
	}
	else
	{
		_txtBaseDesc->setText(tr(_game->getMod()->getStartingBaseSet(
			_game->getMod()->getStartingBaseSetsList().at(row - 1))->Name + "_DESC"));
	}

	_lstBaseSets->setRowColor(_lastRow, _lstBaseSets->getColor());
	_lstBaseSets->setRowColor(row, _lstBaseSets->getSecondaryColor());
	_lastRow = row;
}

}
