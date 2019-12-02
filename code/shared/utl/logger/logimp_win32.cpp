
// Copyright (C) Force67 2019
// TODO: move this in dcore?

#include <Windows.h>

#include "logger.h"

namespace utl
{
	class fileOut : public logBase
	{
		FILE* handle{ nullptr };
		size_t bytes_written{ 0 };

	public:
		explicit fileOut(const std::wstring& filename)
		{
			_wfopen_s(&handle, filename.c_str(), L"w");
		}

		const char* getName() override {
			return "fileOut Sink";
		}

		void write(const logEntry& entry) override
		{
			constexpr std::size_t MAX_BYTES_WRITTEN = 50 * 1024L * 1024L;

			// prevent spam
			if (!handle || bytes_written > MAX_BYTES_WRITTEN)
				return;

			auto msg = formatLogEntry(entry).append(1, '\n');
			bytes_written += fwrite(static_cast<const void*>(msg.c_str()), msg.length(), 1, handle);

			if (entry.log_level >= logLevel::Error) {
				fflush(handle);
			}
		}
	};

	static void PrintMessage(const logEntry& entry)
	{
		auto str = formatLogEntry(entry);
		puts(str.c_str());
	}

	class conOut_Win : public logBase
	{

	public:
		const char* getName() override {
			return "ConOut sink";
		}

		void write(const logEntry& entry) override
		{
			HANDLE console_handle = GetStdHandle(STD_ERROR_HANDLE);
			if (console_handle == INVALID_HANDLE_VALUE) {
				return;
			}

			CONSOLE_SCREEN_BUFFER_INFO original_info = {};
			GetConsoleScreenBufferInfo(console_handle, &original_info);

			WORD color = 0;
			switch (entry.log_level) {
			case logLevel::Trace: // Grey
				color = FOREGROUND_INTENSITY;
				break;
			case logLevel::Debug: // Cyan
				color = FOREGROUND_GREEN | FOREGROUND_BLUE;
				break;
			case logLevel::Info: // Bright gray
				color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
				break;
			case logLevel::Warning: // Bright yellow
				color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
				break;
			case logLevel::Error: // Bright red
				color = FOREGROUND_RED | FOREGROUND_INTENSITY;
				break;
			case logLevel::Critical: // Bright magenta
				color = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
				break;
				//case LogLevel::Count:
				//	UNREACHABLE();
			}

			SetConsoleTextAttribute(console_handle, color);
			PrintMessage(entry);
			SetConsoleTextAttribute(console_handle, original_info.wAttributes);
		}
	};

	class dbgOut_Win32 : public logBase
	{
	public:
		const char* getName() override {
			return "dbgOut sink";
		}

		void write(const logEntry& entry) override
		{
			auto str = formatLogEntry(entry);
			OutputDebugStringA(str.c_str());
		}
	};

	void createLogger(const wchar_t *name)
	{
		// and create a log console
		::AllocConsole();
		::AttachConsole(GetCurrentProcessId());
		::SetConsoleTitleW(L"conOut");

		FILE* file = nullptr;
		freopen_s(&file, "CON", "w", stdout);
		freopen_s(&file, "CONIN$", "r", stdin);

		// attach the sinks to the log system
		addLogSink(std::make_unique<fileOut>(name));
		addLogSink(std::make_unique<conOut_Win>());

		if (IsDebuggerPresent())
			addLogSink(std::make_unique<dbgOut_Win32>());
	}
}