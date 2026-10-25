#include "Core/LocaleFont.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <windows.h>

namespace Lang
{
	namespace
	{
		bool g_registered = false;

		// CJK glyph ranges needed for Simplified Chinese UI text.
		constexpr ImWchar kCjkRanges[] = {
			0x0020, 0x00FF,  // Basic Latin + Latin-1 Supplement
			0x2000, 0x206F,  // General Punctuation
			0x3000, 0x303F,  // CJK Symbols and Punctuation
			0x4E00, 0x9FFF,  // CJK Unified Ideographs
			0xFF00, 0xFFEF,  // Halfwidth and Fullwidth Forms
			0,
		};

		const char* kCjkFontIdentifier = "RM_CN_CJK";

		bool FileExists(const char* aPath)
		{
			if (!aPath) { return false; }
			const DWORD attributes = GetFileAttributesA(aPath);
			return attributes != INVALID_FILE_ATTRIBUTES &&
				(attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
		}

		// Candidate CJK font files. msyh.ttc is a collection: face 0 is Microsoft
		// YaHei, which ships with every supported Windows install and has full
		// Simplified Chinese coverage.
		struct Candidate
		{
			const char* Path;
			int FontNo;
		};

		const Candidate kCandidates[] = {
			{ "C:\\Windows\\Fonts\\msyh.ttc", 0 },
			{ "C:\\Windows\\Fonts\\msyh.ttf", 0 },
			{ "C:\\Windows\\Fonts\\simhei.ttf", 0 },
			{ "C:\\Windows\\Fonts\\simsun.ttc", 0 },
			{ "C:\\Windows\\Fonts\\Deng.ttf", 0 },
		};

		void FontReceiver(const char* aIdentifier, void* aFont)
		{
			// The merge is handled by Nexus using the config we passed; this
			// callback confirms the font was registered. Nothing else to do.
			(void)aIdentifier;
			(void)aFont;
		}
	}

	void RegisterCjkFont(AddonAPI* aApi)
	{
		if (g_registered || aApi == nullptr || aApi->Fonts.AddFromFile == nullptr)
		{
			return;
		}

		for (const Candidate& candidate : kCandidates)
		{
			if (!FileExists(candidate.Path))
			{
				continue;
			}

			ImFontConfig config;
			config.MergeMode = true;
			config.PixelSnapH = true;
			config.FontNo = candidate.FontNo;
			config.GlyphRanges = kCjkRanges;

			// Nexus owns/consumes the config synchronously during this call.
			aApi->Fonts.AddFromFile(
				kCjkFontIdentifier,
				16.0f,
				candidate.Path,
				&FontReceiver,
				&config);

			g_registered = true;
			return;
		}
	}
}
