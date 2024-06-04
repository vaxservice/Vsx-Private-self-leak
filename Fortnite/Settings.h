#pragma once

namespace Settings
{
	namespace Aimbot
	{
		bool Triggerbot = false;
		bool Interpolate = false;
		bool Constant = false;
		bool TargetLine = false;
		double Speed = 10.0;
		bool PredictProjectiles = false;
		double FOV = 50.0;
		bool DrawFOV = false;
		double MaxDistance = 100.0;
		int AimType = 0;
		bool MagicBullet = false;
		int BoneType = 0;
		int KeyType = 0;
		bool Backtrack = false;
		bool SkipKnocked = false;
		bool VisableOnly = false;
		bool Shake = false;
		double ShakeSpeed = 2.0;

		namespace StickySilent
		{
			bool Spinbot = false;
			bool StickyCameraSpin = false;
			bool SwitchPitch = false;
			double SpinSpeed = 5.0;
		}

		namespace TriggerBot
		{
			bool OnlyOnAimKey = true;
			bool FireOnlyOnAimIn = false;
			bool NonProjectileWeapons = false;
			double FireRate = 5.0;
		}

	}

	namespace Player
	{
		int BoxType = 1;
		bool Skeleton = false;
		bool Lines = false;
		bool Name = false;
		bool Weapon = false;
		bool Distance = false;
		bool ViewAngle = false;
		bool Wireframe = false;
		int Visibility = 100;
		bool Minimap = false;
		bool Target = false;

		// Player 2
		bool ShowBots = false;
		bool ShowTeam = false;

		double MaxDistance = 350.0;
	}

	namespace Environment
	{
		bool Weakspot = false;
		bool Projectile = false;
		bool Pickup = false;
		bool Container = false;
		bool Trap = false;
		bool Vehicle = false;
		bool SupplyDrop = false;
		bool WeakspotAim = false;
		double MaxDistance = 100.0;
	}

	namespace Exploits
	{
		bool Hook = false;
		bool ProjectileTeleport = false;
		bool SniperTp = false;
		bool MagicBullet = false;
		bool NoReload = false;
		bool infinitesprint = false;
		bool doublepump = false;
		bool shootinair = false;
		bool norecoil = false;
		bool insrevive = false;
		bool tptoplayer = false;
		bool CarFly = false;
		bool PlayerFly = false;
		bool RapitFire = false;
		bool PlayerFlyOld = false;
		double CarSpeed = 100.0;
		double PlayerFlightSpeed = 50.0;
		bool CameraFOVChanger = false; 
		double CameraFOV = 120.0;
		bool UnlockFireRate = false;
		bool NoSpread = false;
		bool WeaponSuppress = false;
		bool Freecam = false; //Todo: disable input while freecam
		double FreecamSpeed = 5.0;
		int Daytime = 0; //0 = Disabled, 1 = Day, 2 = Night, 3 = Dawn, 4 = Dusk
	}

	namespace Misc
	{
		int CrosshairMode = 1;
		bool BulletTraces = false;
	}

	namespace Colors
	{
		FLinearColor TargetLine = FLinearColor(1.f, 0.f, 0.f, 1.f);
		FLinearColor FieldOfView = FLinearColor(1.f, 1.f, 1.f, 1.f);
		FLinearColor BulletColor = FLinearColor(1.f, 0.f, 0.f, 1.f);
		FLinearColor TeammateColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.f);
		FLinearColor BoxVisible = FLinearColor(1.f, 0.f, 0.f, 1.f);
		FLinearColor BoxInVisible = FLinearColor(0.f, 1.f, 1.f, 1.f);
		FLinearColor SkeletonVisible = FLinearColor(1.f, 0.f, 0.f, 1.f);
		FLinearColor SkeletonInVisible = FLinearColor(0.f, 1.f, 1.f, 1.f);
		FLinearColor SnaplineVisible = FLinearColor(1.f, 0.f, 0.f, 1.f);
		FLinearColor SnaplineInVisible = FLinearColor(0.f, 1.f, 1.f, 1.f);
		FLinearColor SkinColor = FLinearColor((const float)0.f, 2.f, 2.f, 2.f);
	}
}