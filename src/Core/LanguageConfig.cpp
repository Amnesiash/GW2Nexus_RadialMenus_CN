#include "Language.h"

#include <fstream>
#include <string>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include <filesystem>
namespace fs = std::filesystem;

namespace Lang
{
	namespace
	{
		std::string g_configPath;

		std::string ConfigFilePath()
		{
			if (g_configPath.empty()) { return {}; }
			return (fs::path(g_configPath) / "config.json").string();
		}
	}

	void SetConfigPath(const std::string& aDirectory)
	{
		g_configPath = aDirectory;
	}

	void LoadConfig()
	{
		const std::string path = ConfigFilePath();
		if (path.empty()) { return; }

		std::ifstream file(path);
		if (!file.is_open()) { return; }

		try
		{
			const json data = json::parse(file);
			if (data.contains("language"))
			{
				SetLanguage(FromSettingsValue(data["language"].get<int>()));
			}
		}
		catch (const json::exception&)
		{
			// Corrupt config: ignore and keep the default language.
		}
	}

	void SaveConfig()
	{
		const std::string path = ConfigFilePath();
		if (path.empty()) { return; }

		try
		{
			json data;
			data["language"] = ToSettingsValue(Current());

			std::ofstream file(path, std::ios::trunc);
			if (file.is_open())
			{
				file << data.dump(4);
			}
		}
		catch (const json::exception&)
		{
			// Best-effort persistence; ignore write failures.
		}
	}
}
