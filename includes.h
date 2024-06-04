#pragma once
#include <Windows.h>
#include <iostream>
#include <cstdint>
#include <array>
#include <future>
#include <winternl.h>

uintptr_t GameBase = 0;

namespace Offsets
{
	int GWorld = 0x12BA7050;
	int GObjects = 0x12B99C10;
	int GNames = 0x12D85B40;

	// Functions
	int StaticFindObject = 0x146D58C;//0x1466A10
	int GetBoneMatrix = 0x1EB8938; //0x1782D94
	int GetWeaponStats = 0x1863350;
	int Fire = 0x285D0DC;
	int Malloc = 0xC4663D; //0xc378bd
	int Free = 0xC4668D;  //0xc3790d

	// Virtual Indexs
	int DrawTitleSafeArea = 0x388; // updated
	int ProcessEvent = 0x268; // updated   0x268 / 0x8
	//int GetMaterial = 0x2D0 / 0x8; // updated
	int GetMaterial = 0x2D0;
	int GetPlayerViewPoint = 0x7F8 / 0x8; // updated   0x7F8 / 0x8
	int GetCameraViewPoint = 0x2C8 / 0x8; // updated   0x2C8 / 0x8
	int GetViewPoint = 0x2C8 / 0x8; // updated 0x2C8
	int IsAllowedToRender = 0x118 / 0x8; // updated
	int FireSingle = 0x888; // updated
	int TryToFire = 0x548; // updated
	int GetDamageStartLocation = 0x7C8 / 0x8; // updated
	int GetFiringRate = 0x9E0; // updated
	int bIsReloadingWeapon = 0x388;
	int GlobalAnimRateScale = 0xA18;
	int mesh = 0x318;
	int AimPitchMin = 0x1810;
	int AimPitchMax = 0x1814;
}

#define SpoofCode 0x13041052

#include <Helper/Safety/SKCrypter.h>
#include <Helper/Custom/NoCRT.h>

#include <Helper/Custom/Map.h>
#include <Helper/Custom/Vector.h>
#include <Helper/PEB.h>
#include <Helper/SysCall.h>
#include <Helper/SpoofCall.h>

#include <Helper/Custom/Module.h>
#include <Helper/VTableHook.h>

#include <Fortnite/SDK/Structs.h>
#include <Fortnite/SDK/Structs.h>
#include <Fortnite/SDK/Classes.h>
#include <Fortnite/Settings.h>

namespace Variables
{
	UCanvas* Canvas = nullptr;
	APlayerController* PlayerController = nullptr;
	APlayerPawn_Athena_C* AcknowledgedPawn = nullptr;
	FName MaterialParameter = FName(0);
	FName MaterialParameter2 = FName(0);
	FName MaterialParameter3 = FName(0);
	FName MaterialParameter4 = FName(0);
	FName MaterialParameter5 = FName(0);
	FName MaterialParameter6 = FName(0);

	const wchar_t* BuildTime = nullptr;

	//Variables
	static bool DrawMenu = true;
	static FVector2D ScreenCenter = FVector2D();
	static FVector2D ScreenSize = FVector2D();

	//Camera
	static FVector CameraLocation = FVector();
	static FRotator CameraRotation = FRotator();
	static float FieldOfView = 0.f;

	//Bullet tracer
	static int LastBulletIndexPushed = 0;
	static FVector Last5BulletImpacts[5];
	static FVector Last5BulletImpactsMuzzle[5];

	//Exploits
	static FVector BulletTeleportHead = FVector();

	namespace GetDamageStartLocation
	{
		static bool bSilentAimActive = false;
		static FVector SilentLocationTarget = FVector();
		static FRotator LastTargetRotation = FRotator();
	}

	namespace GetTargetingTransform
	{
		static bool bPickaxeRangeActive = false;
		static bool bShouldStartAtLocationTarget = false;
		static FVector LocationTarget = FVector();
	}

	namespace GetPlayerViewPoint
	{
		static bool bShouldStartAtLocationTarget = false;
		static FVector SilentLocationTarget = FVector();
		static FRotator LastTargetRotation = FRotator();
		static FVector OriginalLocation = FVector();
		static FRotator OriginalRotation = FRotator();
	}
	namespace GetCameraViewPoint
	{
		static bool bFreecamActive = false;

		static FRotator LastStickyRotation = FRotator();
		static FVector StickyLocation = FVector();
		static FVector CachedFreecamLocation = FVector();
	}
}

namespace Keys
{
	FKey LeftMouseButton;
	FKey RightMouseButton;
	FKey Insert;
	FKey F2;
	FKey F8;
	FKey W;
	FKey A;
	FKey S;
	FKey D;
	FKey SpaceBar;
	FKey LeftShift;
	FKey Capslock;
}

namespace Classes
{
	UObject* PlayerClass = nullptr;
	UObject* PickupClass = nullptr;
	UObject* ContainerClass = nullptr;
	UObject* ProjectileClass = nullptr;
	UObject* WeakspotClass = nullptr;
	UObject* BuildingTrapClass = nullptr;
	UObject* VehicleClass = nullptr;
	UObject* MarkerClass = nullptr;
	UObject* SupplyDropClass = nullptr;
	UObject* RangedWeaponClass = nullptr;
	UObject* FortWeaponClass = nullptr;
	UObject* PickaxeClass = nullptr;
	UObject* LocalPlayer = nullptr;
	UObject* PlayerController = nullptr;
	UObject* PlayerCameraManager = nullptr;
}

enum Bones : int {
	Root = 0,
	pelvis = 2,
	spine_01 = 3,
	spine_02 = 4,
	spine_03 = 5,
	spine_04 = 6,
	spine_05 = 7,
	clavicle_l = 8,
	upperarm_l = 9,
	lowerarm_l = 10,
	hand_l = 33,
	clavicle_r = 37,
	upperarm_r = 38,
	lowerarm_r = 65,
	hand_r = 62,
	neck_01 = 66,
	neck_02 = 67,
	head = 68,
	thigh_l = 71,
	calf_l = 72,
	calf_twist_01_l = 73,
	calf_twist_02_l = 74,
	foot_l = 76,
	thigh_r = 78,
	calf_r = 79,
	calf_twist_01_r = 80,
	calf_twist_02_r = 81,
	foot_r = 83,
	camera_root = 92,
	Camera = 93,
	attach_fp = 94,
};

const wchar_t* GetBuildTime()
{
	const char* BuildTimeA = __TIME__;
	const int BuildTimeALength = __strlen(BuildTimeA);
	wchar_t* WideArray = new wchar_t[BuildTimeALength + 1];

	for (int i = 0; i < BuildTimeALength; i++)
	{
		WideArray[i] = static_cast<wchar_t>(BuildTimeA[i]);
	}

	WideArray[BuildTimeALength] = L'\0';

	return WideArray;
}

#include <Fortnite/Hooks/GetDamageStartLocation.h>
#include <Fortnite/Hooks/GetPlayerViewPoint.h>
#include <Fortnite/Hooks/GetCameraViewPoint.h>
#include <Fortnite/Hooks/GetViewPoint.h>
#include <Fortnite/Hooks/FireSingle.h>
#include <Fortnite/Hooks/GetFiringRate.h>
#include <Fortnite/Hooks/TryToFire.h>

#include <Fortnite/Hook.h>
#include <Fortnite/Hooks/DrawTitleSafeArea.h>