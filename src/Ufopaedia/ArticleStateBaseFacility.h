#pragma once
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
#include "ArticleState.h"

namespace OpenXcom
{
	class Game;
	class Surface;
	class Text;
	class TextList;
	class ArticleDefinitionBaseFacility;

	/**
	 * ArticleStateBaseFacility has a caption, text, preview image and a stats block.
	 * The facility image is found using the RuleBasefacility class.
	 */

	class ArticleStateBaseFacility : public ArticleState
	{
	public:
		ArticleStateBaseFacility(ArticleDefinitionBaseFacility *article_defs, std::shared_ptr<ArticleCommonState> state);
		virtual ~ArticleStateBaseFacility();
		/// Gets width of the text (in pixels) from stream.
		int getTextWidth(std::ostringstream *strStream) const;
		/// Adds row to the stats list based on received parameters.
		void addToStatList(std::ostringstream *sStream, std::ostringstream *vStream, int colStat, int colValue, int row);

	protected:
		Surface *_image;
		Text *_txtTitle;
		Text *_txtInfo;
		TextList *_lstInfo;
	};
}
