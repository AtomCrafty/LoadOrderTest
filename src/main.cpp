#include <spdlog/sinks/basic_file_sink.h>

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
	case SKSE::MessagingInterface::kDataLoaded:
	{
		ProcessLoadedPlugins();
		ProcessDataDirectory();
		break;
	}

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
