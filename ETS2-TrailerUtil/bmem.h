#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include <vector>

#include <MinHook/MinHook.h>

#include "scs_logging.h"
using namespace scs_logging;

namespace bmem
{
	namespace
	{
		// custom byte type
		class pattern_byte
		{
		public:
			uint8_t byte;
			bool ignore;
		};

		inline uintptr_t moduleBase;
		inline uint64_t moduleSize;

		inline bool wasModuleSet = false;
	}

	static bool setModule(const char* module)
	{
		if (std::string(module) == "current")
		{
			if (!wasModuleSet)
				moduleBase = (uintptr_t)GetModuleHandleA(nullptr);
			else
				return true;
		}
		else
			moduleBase = (uintptr_t)GetModuleHandleA(module);


		if (!moduleBase)
		{
			printf("[BMEM] Failed to set module to: '%s'. Module not found! | Did you mean '%s.dll' or '%s.exe'?\n", module, module, module);
			return false;
		}

		const auto* header = (IMAGE_DOS_HEADER*)moduleBase;
		const auto* nt_header = (IMAGE_NT_HEADERS64*)((uint8_t*)header + header->e_lfanew);

		moduleSize = nt_header->OptionalHeader.SizeOfImage;

		printf("[BMEM] Module set to: '%s' (Base: 0x%llx, Size: %llu)\n", module, moduleBase, moduleSize);

		wasModuleSet = true;
		return true;
	}



	static uintptr_t patternScan(const char* patternSTR, const char* moduleToSet = "current")
	{
		if (!setModule(moduleToSet))
		{
			return 0;
		}

		std::vector<pattern_byte> pattern;

		std::istringstream stream(patternSTR);
		std::string token;
		while (stream >> token) {
			pattern_byte pbyte;

			if (token == "??" || token == "?") {
				pbyte.ignore = true;
			}
			else 
			{
				if (token.length() > 2 || token.length() < 2)
				{
					return 0;
				}

				pbyte.ignore = false;

				unsigned int byte;
				std::istringstream(token) >> std::hex >> byte;
				pbyte.byte = (uint8_t)byte;
			}

			pattern.push_back(pbyte);
		}


		if (pattern[0].ignore)
		{
			return 0;
		}


		bool foundFirstByte = false;
		int patternIndex = 0;
		uintptr_t patternStart = 0;
		for (uint64_t i = 0; i < moduleSize; i++)
		{
			uintptr_t currentAddress = moduleBase+i;
			uint8_t currentByte = *reinterpret_cast<uint8_t*>(currentAddress);
			
			if (!foundFirstByte)
			{
				if (currentByte == pattern[patternIndex].byte)
				{
					patternStart = currentAddress;
					patternIndex++;
					foundFirstByte = true;
				}
			}
			else
			{
				if (currentByte == pattern[patternIndex].byte) {
					patternIndex++;
				}
				else if (pattern[patternIndex].ignore)
				{
					patternIndex++;
				}
				else
				{
					patternStart = 0;
					patternIndex = 0;
					foundFirstByte = false;
				}

				if (patternIndex == pattern.size())
				{
					break;
				}
			}
		}

		return patternStart;
	}


	static uintptr_t relativeToAbsolute(uintptr_t address, int addressOffset, int instructionCount)
	{
		return (uintptr_t)(address + instructionCount + *reinterpret_cast<std::int32_t*>(address + addressOffset));
	}
	
	
	static bool isAddressValid(uintptr_t address, const char* moduleToSet = "current")
	{
		if (address > moduleBase + moduleSize)
			return false;

		if (address < moduleBase)
			return false;

		return true;
	}


	// returns: success | true = MH finished successfully | false = MH had a error
	static bool MH_Success(MH_STATUS result)
	{
		if (result == MH_OK) return true;

		if (result == MH_ERROR_ALREADY_INITIALIZED) scs_log(2, "[MinHook] MinHook is already initialized.");
		else if (result == MH_ERROR_NOT_INITIALIZED) scs_log(2, "[MinHook] MinHook is not initialized yet, or already uninitialized.");
		else if (result == MH_ERROR_ALREADY_CREATED) scs_log(2, "[MinHook] The hook for the specified target function is already created.");
		else if (result == MH_ERROR_NOT_CREATED) scs_log(2, "[MinHook] The hook for the specified target function is not created yet.");
		else if (result == MH_ERROR_ENABLED) scs_log(2, "[MinHook] The hook for the specified target function is already enabled.");
		else if (result == MH_ERROR_DISABLED) scs_log(2, "[MinHook] The hook for the specified target function is not enabled yet, or already disabled.");
		else if (result == MH_ERROR_NOT_EXECUTABLE) scs_log(2, "[MinHook] The specified pointer is invalid. It points the address of non-allocated and/or non-executable region.");
		else if (result == MH_ERROR_UNSUPPORTED_FUNCTION) scs_log(2, "[MinHook] The specified target function cannot be hooked.");
		else if (result == MH_ERROR_MEMORY_ALLOC) scs_log(2, "[MinHook] Failed to allocate memory.");
		else if (result == MH_ERROR_MEMORY_PROTECT) scs_log(2, "[MinHook] Failed to change the memory protection.");
		else if (result == MH_ERROR_MODULE_NOT_FOUND) scs_log(2, "[MinHook] The specified module is not loaded.");
		else if (result == MH_ERROR_FUNCTION_NOT_FOUND) scs_log(2, "[MinHook] The specified function is not found.");
		else if (result == MH_UNKNOWN) scs_log(2, "[MinHook] Unknown error.");

		return false;
	}
}