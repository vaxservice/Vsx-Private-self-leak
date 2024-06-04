#pragma once
#include "includes.h"

namespace Hook
{
	struct Discord
	{
		static void DiscordHook(void* Target, void* HookFn, void** Original)
		{
			reinterpret_cast<void* (__fastcall*)(void*, void*, void**)>(DiscordHook64 + 0x53990)(Target, HookFn, Original);
			reinterpret_cast<void* (__fastcall*)(void*)>(DiscordHook64 + 0x54310)(Target);
			reinterpret_cast<void* (*)()>(DiscordHook64 + 0x54400)();
		}
	};
}