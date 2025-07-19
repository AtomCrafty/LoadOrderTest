#include <spdlog/sinks/basic_file_sink.h>

void CheckEngineFixesPatch()
{
	// https://github.com/aers/EngineFixesSkyrim64/blob/f592d56e5bd156e84c046e6a22f5e5ef4448e79a/src/patches/miscpatches.cpp#L197
	// This function in the 1.6.629 - 1.6.640 build of SSE Engine Fixes (and possibly other builds)
	// patches ReadPluginList to immediately return false. This prevents any plugins from loading on 1.6.1170.

	REL::Relocation<const unsigned char> ReadPluginList(RELOCATION_ID(13650, 13758));

	constexpr unsigned char patch[] = { 0xB0, 0x00, 0xC3 };

	if (memcmp(reinterpret_cast<void*>(ReadPluginList.address()), patch, sizeof patch) != 0) {
		logger::info("ReadPluginList patch not detected");
	}
	else {
		logger::info("ReadPluginList patch detected");
		logger::info("You most likely have an incorrect version of SSE Engine Fixes installed");
	}

	logger::info("");
}

void ProcessPluginsTxt()
{
	REL::Relocation<const char*> AppDataFolder(RELOCATION_ID(524595, 411235));

	std::ifstream is{ std::filesystem::path(AppDataFolder.get()) / "Plugins.txt" };
	std::string line{};

	int totalPlugins = 0;
	int enabledPlugins = 0;

	if (is.is_open()) {
		logger::info("Mods according to Plugins.txt:");

		while (std::getline(is, line)) {
			if (line.empty() || line.starts_with('#')) {
				continue;
			}

			totalPlugins++;

			if (line.starts_with('*')) {
				logger::info("* {}", line.c_str() + 1);
				enabledPlugins++;
			}
			else {
				logger::info("  {}", line);
			}
		}
	}
	else {
		logger::info("Unable to read Plugins.txt");
	}

	logger::info("");
	logger::info("{} of {} plugins enabled", enabledPlugins, totalPlugins);

	static int lastEnabledPlugins = enabledPlugins;

	if (lastEnabledPlugins != enabledPlugins) {
		logger::info("WARNING: Enabled plugin count has changed since last scan");
	}

	lastEnabledPlugins = enabledPlugins;

	logger::info("");
}

void ProcessLoadedPlugins()
{
	const auto data = RE::TESDataHandler::GetSingleton();

	logger::info("Loaded plugins:");
	for (size_t i = 0; i < data->GetLoadedModCount(); i++) {
		logger::info("  {:02X}     {}", i, data->GetLoadedMods()[i]->fileName);
	}

	for (size_t i = 0; i < data->GetLoadedLightModCount(); i++) {
		logger::info("  FE {:03X} {}", i, data->GetLoadedLightMods()[i]->fileName);
	}

	logger::info("");
}

void ProcessDataDirectory()
{
	logger::info("Working directory: {}", std::filesystem::current_path().string());
	logger::info("Virtual data folder contents:");
	for (auto& entry : std::filesystem::directory_iterator("Data")) {
		if (!entry.is_regular_file()) {
			continue;
		}

		if (entry.path().extension() != ".esm" &&
			entry.path().extension() != ".esp" &&
			entry.path().extension() != ".esl") {
			continue;
		}

		logger::info("  {}", entry.path().filename().string());
	}

	logger::info("");
}

void OnSKSEMessage(SKSE::MessagingInterface::Message* msg)
{
	switch (msg->type) {
	case SKSE::MessagingInterface::kInputLoaded:
		logger::info("----------------[kInputLoaded]----------------");
		CheckEngineFixesPatch();
		ProcessPluginsTxt();
		break;

	case SKSE::MessagingInterface::kDataLoaded:
		logger::info("----------------[kDataLoaded]----------------");
		ProcessPluginsTxt();
		ProcessLoadedPlugins();
		ProcessDataDirectory();
		break;

	default:
		break;
	}
}

void InitializeLog(spdlog::level::level_enum level = spdlog::level::info)
{
	auto path = logger::log_directory();
	if (!path) {
		SKSE::stl::report_and_fail("Failed to find standard logging directory");
	}

	*path /= std::format("LoadOrderTest.log");
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
	auto log = std::make_shared<spdlog::logger>("global log", std::move(sink));

	log->set_level(level);
	log->flush_on(level);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("%v");
}

extern "C" __declspec(dllexport) bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* skse)
{
	InitializeLog();
	SKSE::Init(skse, false);
	return SKSE::GetMessagingInterface()->RegisterListener(OnSKSEMessage);
}
