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
#include "CraftWeaponsState.h"
#include <sstream>
#include "../Engine/Game.h"
#include "../Mod/Mod.h"
#include "../Engine/LocalizedText.h"
#include "../Engine/Options.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"
#include "../Interface/Text.h"
#include "../Interface/TextList.h"
#include "../Savegame/Craft.h"
#include "../Savegame/CraftWeapon.h"
#include "../Mod/RuleCraft.h"
#include "../Mod/RuleCraftWeapon.h"
#include "../Savegame/ItemContainer.h"
#include "../Savegame/Base.h"
#include "../Savegame/SavedGame.h"
#include "../Ufopaedia/Ufopaedia.h"
#include "../Menu/ErrorMessageState.h"
#include "../Mod/RuleInterface.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Craft Weapons window.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 * @param craft ID of the selected craft.
 * @param weapon ID of the selected weapon.
 */
CraftWeaponsState::CraftWeaponsState(Base *base, size_t craft, size_t weapon) : _base(base), _craft(base->getCrafts()->at(craft)), _weapon(weapon)
{
	_screen = false;

	// Create objects
	_window = new Window(this, 220, 160, 50, 20, POPUP_BOTH);
	_btnCancel = new TextButton(140, 16, 90, 156);
	_txtTitle = new Text(208, 17, 56, 28);
	_txtArmament = new Text(76, 9, 66, 52);
	_txtQuantity = new Text(50, 9, 140, 52);
	_txtAmmunition = new Text(68, 17, 200, 44);
	_lstWeapons = new TextList(188, 64, 58, 68);
	_txtCurrentWeapon = new Text(188, 9, 66, 140);

	// Set palette
	setInterface("craftWeapons");

	add(_window, "window", "craftWeapons");
	add(_btnCancel, "button", "craftWeapons");
	add(_txtTitle, "text", "craftWeapons");
	add(_txtArmament, "text", "craftWeapons");
	add(_txtQuantity, "text", "craftWeapons");
	add(_txtAmmunition, "text", "craftWeapons");
	add(_lstWeapons, "list", "craftWeapons");
	add(_txtCurrentWeapon, "text", "craftWeapons");

	centerAllSurfaces();

	// Set up objects
	setWindowBackground(_window, "craftWeapons");

	_btnCancel->setText(tr("STR_CANCEL_UC"));
	_btnCancel->onMouseClick((ActionHandler)&CraftWeaponsState::btnCancelClick);
	_btnCancel->onKeyboardPress((ActionHandler)&CraftWeaponsState::btnCancelClick, Options::keyCancel);

	_txtTitle->setBig();
	_txtTitle->setAlign(ALIGN_CENTER);
	_txtTitle->setText(tr("STR_SELECT_ARMAMENT"));

	_txtArmament->setText(tr("STR_ARMAMENT"));

	_txtQuantity->setText(tr("STR_QUANTITY_UC"));

	_txtAmmunition->setText(tr("STR_AMMUNITION_AVAILABLE"));
	_txtAmmunition->setWordWrap(true);
	_txtAmmunition->setVerticalAlign(ALIGN_BOTTOM);

	const std::string slotName = _craft->getRules()->getWeaponSlotString(weapon);
	CraftWeapon *current = _craft->getWeapons()->at(_weapon);
	if (current != 0)
	{
		_txtCurrentWeapon->setText(tr(slotName).arg(tr(current->getRules()->getType())));
	}
	else
	{
		_txtCurrentWeapon->setText(tr(slotName).arg(tr("STR_NONE_UC")));
	}

	_lstWeapons->setColumns(3, 94, 50, 36);
	_lstWeapons->setSelectable(true);
	_lstWeapons->setBackground(_window);
	_lstWeapons->setMargin(8);

	_lstWeapons->addRow(1, tr("STR_NONE_UC").c_str());
	_weapons.push_back(0);

	for (auto& craftWeaponType : _game->getMod()->getCraftWeaponsList())
	{
		RuleCraftWeapon *w = _game->getMod()->getCraftWeapon(craftWeaponType);
		const RuleCraft *c = _craft->getRules();
		bool isResearched = true;
		if (w->getClipItem())
		{
			isResearched = _game->getSavedGame()->isResearched(w->getClipItem()->getRequirements());
		}
		if (isResearched && _base->getStorageItems()->getItem(w->getLauncherItem()) > 0 && c->isValidWeaponSlot(weapon, w->getWeaponType()))
		{
			_weapons.push_back(w);
			std::ostringstream ss, ss2;
			ss << _base->getStorageItems()->getItem(w->getLauncherItem());
			if (w->getClipItem())
			{
				ss2 << _base->getStorageItems()->getItem(w->getClipItem());
			}
			else
			{
				ss2 << tr("STR_NOT_AVAILABLE");
			}
			_lstWeapons->addRow(3, tr(w->getType()).c_str(), ss.str().c_str(), ss2.str().c_str());
		}
	}
	_lstWeapons->onMouseClick((ActionHandler)&CraftWeaponsState::lstWeaponsClick);
	_lstWeapons->onMouseClick((ActionHandler)&CraftWeaponsState::lstWeaponsMiddleClick, SDL_BUTTON_MIDDLE);
}

/**
 *
 */
CraftWeaponsState::~CraftWeaponsState()
{

}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void CraftWeaponsState::btnCancelClick(Action *)
{
	_game->popState();
}

/**
 * Equips the weapon on the craft and returns to the previous screen.
 * @param action Pointer to an action.
 */
void CraftWeaponsState::lstWeaponsClick(Action *)
{
	bool sizeChanged = false;
	CraftWeapon *current = _craft->getWeapons()->at(_weapon);

	const RuleCraftWeapon* refWeapon = _weapons[_lstWeapons->getSelectedRow()];
	const RuleCraftWeapon* currWeapon = current ? current->getRules() : nullptr;

	// Validate craft size after refit.
	{
		int refWeaponCraftSize = refWeapon ? refWeapon->getBonusStats().craftSize : 0;
		int currWeaponCraftSize = currWeapon ? currWeapon->getBonusStats().craftSize : 0;
		int diffCraftSize = (refWeaponCraftSize - currWeaponCraftSize);
		if (diffCraftSize)
		{
			// Check, if there is suitable hangar slot for the new craft size.
			int newCraftSize = _craft->getCraftSize() + diffCraftSize;
			if (!_base->allowCraftRefit(_craft, newCraftSize))
			{
				_game->popState();
				_game->pushState(new ErrorMessageState(
					tr("STR_NO_FREE_HANGARS_FOR_REFIT"),
					_palette,
					_game->getMod()->getInterface("craftWeapons")->getElement("window")->color,
					"BACK14.SCR",
					_game->getMod()->getInterface("craftWeapons")->getElement("palette")->color)
				);
				return;
			}

			// Check, if game rules allow craft to change its classification.
			bool classChanged = _game->getMod()->getCraftClassFromSize(newCraftSize) !=
				_game->getMod()->getCraftClassFromSize(_craft->getCraftSize());
			if (classChanged && !_game->getMod()->getCraftsCanChangeClass())
			{
				_game->popState();
				_game->pushState(new ErrorMessageState(
					tr("STR_NO_CRAFT_CLASS_CHANGE"),
					_palette,
					_game->getMod()->getInterface("craftWeapons")->getElement("window")->color,
					"BACK14.SCR",
					_game->getMod()->getInterface("craftWeapons")->getElement("palette")->color)
				);
				return;
			}

			// Notify game to sync hangar slots, since craft size got changed.
			sizeChanged = true;
		}
	}

	// Validate soldier capacity after refit.
	{
		int refCapBonusSoldiers = refWeapon ? refWeapon->getBonusStats().soldiers : 0;
		int currCapBonusSoldiers = currWeapon ? currWeapon->getBonusStats().soldiers : 0;
		int diffSoldiers = (refCapBonusSoldiers - currCapBonusSoldiers);
		if (diffSoldiers)
		{
			if ((_craft->getMaxUnits() - _craft->getSpaceUsed() + diffSoldiers) < 0)
			{
				_game->popState();
				_game->pushState(new ErrorMessageState(
					tr("STR_NOT_ENOUGH_CARGO_SPACE"),
					_palette,
					_game->getMod()->getInterface("craftWeapons")->getElement("window")->color,
					"BACK14.SCR",
					_game->getMod()->getInterface("craftWeapons")->getElement("palette")->color)
				);
				return;
			}
		}
	}

	// Validate vehicle capacity after refit.
	{
		int refCapBonusVehicles = refWeapon ? refWeapon->getBonusStats().vehicles : 0;
		int currCapBonusVehicles = currWeapon ? currWeapon->getBonusStats().vehicles : 0;
		int diffVehicles = (refCapBonusVehicles - currCapBonusVehicles);
		if (diffVehicles)
		{
			if ((_craft->getMaxVehiclesAndLargeSoldiers() - _craft->getNumVehiclesAndLargeSoldiers() + diffVehicles) < 0)
			{
				_game->popState();
				_game->pushState(new ErrorMessageState(
					tr("STR_NOT_ENOUGH_HWP_CAPACITY"),
					_palette,
					_game->getMod()->getInterface("craftWeapons")->getElement("window")->color,
					"BACK14.SCR",
					_game->getMod()->getInterface("craftWeapons")->getElement("palette")->color)
				);
				return;
			}
		}
	}

	// Remove current weapon
	if (current != 0)
	{
		_base->getStorageItems()->addItem(current->getRules()->getLauncherItem());
		_base->getStorageItems()->addItem(current->getRules()->getClipItem(), current->getClipsLoaded());
		_craft->addCraftStats(-current->getRules()->getBonusStats());
		// Make sure any extra shield is removed from craft too when the shield capacity decreases (exploit protection)
		_craft->setShield(_craft->getShield());
		delete current;
		_craft->getWeapons()->at(_weapon) = 0;
	}

	// Equip new weapon
	if (_weapons[_lstWeapons->getSelectedRow()] != 0)
	{
		CraftWeapon *sel = new CraftWeapon(_weapons[_lstWeapons->getSelectedRow()], 0);
		_craft->addCraftStats(sel->getRules()->getBonusStats());
		_base->getStorageItems()->removeItem(sel->getRules()->getLauncherItem());
		_craft->getWeapons()->at(_weapon) = sel;
	}

	// Wrap up and finish function execution.
	if (sizeChanged)
		_base->syncCraftSlots();
	_craft->checkup();
	_game->popState();
}

/**
* Opens the corresponding Ufopaedia article.
* @param action Pointer to an action.
*/
void CraftWeaponsState::lstWeaponsMiddleClick(Action *)
{
	RuleCraftWeapon *rule = _weapons[_lstWeapons->getSelectedRow()];

	if (rule != 0)
	{
		std::string articleId = rule->getType();
		Ufopaedia::openArticle(_game, articleId);
	}
}

}
