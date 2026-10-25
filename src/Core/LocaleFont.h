#pragma once

#include "Nexus/Nexus.h"

namespace Lang
{
	///----------------------------------------------------------------------------------------------------
	/// RegisterCjkFont:
	/// 	Merges a Simplified Chinese fallback font (Microsoft YaHei) into the
	/// 	active UI font's glyph atlas so that Simplified Chinese text renders as
	/// 	glyphs instead of tofu boxes. Safe to call once during addon load; it is
	/// 	a no-op if the font file is missing or the atlas cannot be updated.
	///----------------------------------------------------------------------------------------------------
	void RegisterCjkFont(AddonAPI* aApi);
}
