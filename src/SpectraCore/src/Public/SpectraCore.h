#pragma once
#include "Instrumenta.h"

#ifndef SPECTRA_CORE
#define SPECTRA_CORE __declspec(dllexport)
#endif

void SPECTRA_CORE SpectraCoreInit();

namespace spectra::core {

	// Constants and macros for SpectraCore
	inline bool isSpectraCoreInitialized = false;
	inline instrumenta::Instrumentation& orchestrator = instrumenta::Instrumentation::getInstance();

#ifndef SIGN_EXTEND_4
#define SIGN_EXTEND_4(bits) ((bits & 0x00) ? (bits | 0xf0) : bits)
#endif

#ifndef BIT_MASK_4
#define BIT_MASK_4(value) (value & 0x0F)
#endif

#ifndef SPC_COMPONENT_IDENTIFIER
#define SPC_COMPONENT_IDENTIFIER "SpectraCore"
#endif

#ifndef SPC_CONSOLE_LOG_SINK_ID
#define SPC_CONSOLE_LOG_SINK_ID "SpectraCoreConsoleLogSink"
#endif

#ifndef SPC_FILE_LOG_SINK_ID
#define SPC_FILE_LOG_SINK_ID "SpectraCoreFileLogSink"
#endif

#ifndef SPC_LOG_INFO
#define SPC_LOG_INFO(subcomponent, message, tag, ...) \
	if(isSpectraCoreInitialized){ \
		orchestrator.getLogger("Base")->log(instrumenta::E_LogLevel::INFO_, SPC_COMPONENT_IDENTIFIER, subcomponent, message, tag, ##__VA_ARGS__); \
	}else throw std::runtime_error("SpectraCore is not initialized. Call SpectraCoreInit() first.");
#endif

#ifndef SPC_LOG_DEBUG
#define SPC_LOG_DEBUG(subcomponent, message, tag, ...) \
	if(isSpectraCoreInitialized){ \
		orchestrator.getLogger("Base")->log(instrumenta::E_LogLevel::DEBUG_, SPC_COMPONENT_IDENTIFIER, subcomponent, message, tag, ##__VA_ARGS__); \
	}else throw std::runtime_error("SpectraCore is not initialized. Call SpectraCoreInit() first.");
#endif

#ifndef SPC_LOG_WARNING
#define SPC_LOG_WARNING(subcomponent, message, tag, ...) \
	if(isSpectraCoreInitialized){ \
		orchestrator.getLogger("Base")->log(instrumenta::E_LogLevel::WARNING_, SPC_COMPONENT_IDENTIFIER, subcomponent, message, tag, ##__VA_ARGS__); \
	}else throw std::runtime_error("SpectraCore is not initialized. Call SpectraCoreInit() first.");
#endif

#ifndef SPC_LOG_ERROR
#define SPC_LOG_ERROR(subcomponent, message, tag, ...) \
	if(isSpectraCoreInitialized){ \	
		orchestrator.getLogger("Base")->log(instrumenta::E_LogLevel::ERROR_, SPC_COMPONENT_IDENTIFIER, subcomponent, message, tag, ##__VA_ARGS__); \
	}else throw std::runtime_error("SpectraCore is not initialized. Call SpectraCoreInit() first.");
#endif



}
