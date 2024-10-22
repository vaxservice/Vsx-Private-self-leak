#pragma once
#include "../DrawMenu.h"

#include <Windows.h>



bool is_bad_write_ptr(LPVOID ptr, UINT_PTR size)
{
	return ptr ? false : true;
}

bool valid_pointer(uintptr_t address)
{
	if (!is_bad_write_ptr((LPVOID)address, (UINT_PTR)8)) return TRUE;
	else return FALSE;
}

template<typename ReadT>
ReadT read(DWORD_PTR address, const ReadT& def = ReadT())
{
	if (valid_pointer(address)) {
		return *(ReadT*)(address);
	}
}



void silent(const FVector2D& BonePosition, AFortWeapon* CurrentWeapon, const std::string& CurrentWeaponName) {//, const std::string& CurrentWeaponName
	//FRotator BonePosition = ClampAngles(CalcAngle(Canmy::Loc, BonePosition));   idk
	static bool bInitCamera = true;

	if (!CurrentWeapon) return;
	if (CurrentWeaponName.empty()) return;

	if (bInitCamera)
	{
		int OriginalPitchMin = read<uintptr_t>((uintptr_t)CurrentWeapon + Offsets::AimPitchMin); //AimPitchMin
		int OriginalPitchMAX = read<uintptr_t>((uintptr_t)CurrentWeapon + Offsets::AimPitchMax); //AimPitchMax
	}


	if (GetAsyncKeyState(VK_RBUTTON))
	{
		//UD ASF
	}
}

void(*DrawTitleSafeAreaOriginal)(UGameViewportClient* Viewport, UCanvas* Canvas) = nullptr;
void DrawTitleSafeArea(UGameViewportClient* Viewport, UCanvas* Canvas)
{
	if (!Viewport || !Canvas)
		return DrawTitleSafeAreaOriginal(Viewport, Canvas);
	Variables::Canvas = Canvas;
	Variables::ScreenSize = FVector2D(double(Variables::Canvas->ClipX()), double(Variables::Canvas->ClipY()));
	Variables::ScreenCenter = FVector2D(Variables::ScreenSize.X / 2.0, Variables::ScreenSize.Y / 2.0);

	bool Update__GetDamageStartLocation__bSilentAimActive = false;
	FVector Update__GetDamageStartLocation__SilentLocationTarget = FVector();

	bool GetTargetingTransform_bPickaxeRangeActive = false;
	bool GetTargetingTransform_bShouldStartAtLocationTarget = false;
	FVector GetTargetingTransform_LocationTarget = FVector();

	bool Update_SpinningRightNow = false;
	static bool SpinningRightNow = false;

	bool IsCurrentlySickySilentAiming = false;

	bool UpdateGetTargetingSourceLocationbSilentAimActive = false;
	FVector UpdateGetTargetingSourceLocationSilentLocationTarget = FVector();

	bool UpdateGetWeaponTargetingTransformbSilentAimActive = false;
	FVector UpdateGetWeaponTargetingTransformSilentLocationTarget = FVector();

	bool GetPlayerViewPoint_bShouldStartAtLocationTarget = false;
	FVector GetPlayerViewPoint_SilentLocationTarget = FVector();

	bool Update_ShootOutOfAir = false;
	FVector Update_LocalHead = FVector();

	bool Update_FieldOfViewChanger = false;
	char UpdateMyTeam = char(1337);
	bool bInVehicle = false;

	UWorld* GWorld = UWorld::GetWorld();
	if (!GWorld) return DrawTitleSafeAreaOriginal(Viewport, Canvas);

	UGameInstance* Gameinstance = GWorld->OwningGameInstance();
	if (!Gameinstance) return DrawTitleSafeAreaOriginal(Viewport, Canvas);

	ULocalPlayer* LocalPlayer = Gameinstance->LocalPlayers()[0];
	if (!LocalPlayer) return DrawTitleSafeAreaOriginal(Viewport, Canvas);

	APlayerController* PlayerController = LocalPlayer->PlayerController();
	if (!PlayerController) return DrawTitleSafeAreaOriginal(Viewport, Canvas);
	Variables::PlayerController = PlayerController;

	APlayerCameraManager* PlayerCameraManager = PlayerController->PlayerCameraManager();
	if (!PlayerCameraManager) return DrawTitleSafeAreaOriginal(Viewport, Canvas);

	APlayerState* PlayerState = PlayerController->PlayerState();
	if (!PlayerState) return DrawTitleSafeAreaOriginal(Viewport, Canvas);

	APlayerPawn_Athena_C* AcknowledgedPawn = PlayerController->AcknowledgedPawn();
	Variables::AcknowledgedPawn = AcknowledgedPawn;

	Variables::FieldOfView = PlayerCameraManager->GetFOVAngle();
	Variables::CameraLocation = PlayerCameraManager->GetCameraLocation();
	Variables::CameraRotation = PlayerCameraManager->GetCameraRotation();

	if (Settings::Exploits::Hook)
	{
		Rehook(LocalPlayer);
	}

	if (Settings::Exploits::Freecam)
	{
		if (PlayerController->WasInputKeyJustPressed(Keys::Capslock))
		{
			Variables::GetCameraViewPoint::bFreecamActive = !Variables::GetCameraViewPoint::bFreecamActive;
		}

		double FreecamSpeed = Settings::Exploits::FreecamSpeed;

		FVector CameraForward = UKismetMathLibrary::GetForwardVector(Variables::CameraRotation) * FreecamSpeed;

		FRotator CameraRotationSideway = FRotator(Variables::CameraRotation.Pitch, Variables::CameraRotation.Yaw + 90.0, Variables::CameraRotation.Roll);
		FVector CameraForwardSideway = UKismetMathLibrary::GetForwardVector(CameraRotationSideway) * FreecamSpeed; CameraForwardSideway.Z = 0.0;

		//Forwards/Backwards
		if (PlayerController->IsInputKeyDown(Keys::W))
			Variables::GetCameraViewPoint::CachedFreecamLocation += CameraForward;
		else if (PlayerController->IsInputKeyDown(Keys::S))
			Variables::GetCameraViewPoint::CachedFreecamLocation -= CameraForward;

		//Left/Right
		if (PlayerController->IsInputKeyDown(Keys::A))
			Variables::GetCameraViewPoint::CachedFreecamLocation -= CameraForwardSideway;
		if (PlayerController->IsInputKeyDown(Keys::D))
			Variables::GetCameraViewPoint::CachedFreecamLocation += CameraForwardSideway;

		//Up/Down
		if (PlayerController->IsInputKeyDown(Keys::SpaceBar))
			Variables::GetCameraViewPoint::CachedFreecamLocation.Z += FreecamSpeed;
		else if (PlayerController->IsInputKeyDown(Keys::LeftShift))
			Variables::GetCameraViewPoint::CachedFreecamLocation.Z -= FreecamSpeed;
	}

	double SmallestDegrees = DBL_MAX;
	FVector SmallestDegreesOwnerWorldLocation = FVector();

	double ClosestDistanceToMyself = DBL_MAX;
	double ClosestDistanceToCenter = DBL_MAX;
	APlayerPawn_Athena_C* TargetPlayer = nullptr;

	double AimbotFOV = (Settings::Aimbot::FOV * Variables::ScreenSize.X / Variables::FieldOfView) / 2.0;

	if (AcknowledgedPawn)
	{
		UpdateMyTeam = AcknowledgedPawn->GetTeam();
		bInVehicle = AcknowledgedPawn->IsInVehicle();

		if (USkeletalMeshComponent* Mesh = AcknowledgedPawn->Mesh())
		{
			Update_LocalHead = Mesh->GetBoneLocation(Bones::head);
		}

		FVector StickyLocation = AcknowledgedPawn->K2_GetActorLocation(); StickyLocation.Z += 100.0;
		Variables::GetCameraViewPoint::StickyLocation = StickyLocation;

		if (Settings::Exploits::Daytime)
		{
			if (Settings::Exploits::Daytime == 1)
			{
				UFortKismetLibrary::SetTimeOfDay(GWorld, 9.f); //Day
			}
			else if (Settings::Exploits::Daytime == 2)
			{
				UFortKismetLibrary::SetTimeOfDay(GWorld, 1.f); //Night
			}
			else if (Settings::Exploits::Daytime == 3)
			{
				UFortKismetLibrary::SetTimeOfDay(GWorld, 3.f); //Dawn

			}
			else if (Settings::Exploits::Daytime == 4)
			{
				UFortKismetLibrary::SetTimeOfDay(GWorld, 21.f); //Dusk
			}

			UFortKismetLibrary::SetTimeOfDaySpeed(GWorld, 100.f);
		}

		if (AFortWeapon* CurrentWeapon = AcknowledgedPawn->CurrentWeapon())
		{
			bool CurrentWeaponIsA = UGameplayStatics::ObjectIsA(CurrentWeapon, Classes::FortWeaponClass);
			if (CurrentWeaponIsA)
			{
				if (Settings::Exploits::WeaponSuppress)
				{
					*(UObject**)(CurrentWeapon + 0x808) = nullptr;// 0x808
					*(UObject**)(CurrentWeapon + 0x828) = nullptr;// 0x828

					for (int zi = 0; zi < 3; zi++)
					{
						UObject** PrimaryFireSound = reinterpret_cast<UObject**>(CurrentWeapon + 0x8c8);//0x810
						PrimaryFireSound[zi] = nullptr;
					}

					for (int zi = 0; zi < 3; zi++)
					{
						UObject** PrimaryFireStopSound = reinterpret_cast<UObject**>(CurrentWeapon + 0x8e8);//0x830
						PrimaryFireStopSound[zi] = nullptr;
					}
				}

				bool CurrentWeaponIsARanged = UGameplayStatics::ObjectIsA(CurrentWeapon, Classes::RangedWeaponClass);
				if (CurrentWeaponIsARanged)
				{
					if (Settings::Exploits::NoSpread)
					{
						*(float*)(CurrentWeapon + 0x64) = 0.f;
					}

					if (Settings::Exploits::WeaponSuppress)
					{
						*(UObject**)(CurrentWeapon + 0x1b00) = nullptr;
						*(UObject**)(CurrentWeapon + 0x1b08) = nullptr;
						*(UObject**)(CurrentWeapon + 0x1b10) = nullptr;
						*(UObject**)(CurrentWeapon + 0x1b18) = nullptr;
						*(UObject**)(CurrentWeapon + 0x1b20) = nullptr;
						*(UObject**)(CurrentWeapon + 0x1b28) = nullptr;
						*(UObject**)(CurrentWeapon + 0x1b30) = nullptr;
						*(UObject**)(CurrentWeapon + 0x1b38) = nullptr;
						*(UObject**)(CurrentWeapon + 0x1b38) = nullptr;
					}
				}
			}
		}

		if (Settings::Environment::Pickup)
		{
			TArray<UObject*> PickupArray = UGameplayStatics::GetAllActorsOfClass(GWorld, Classes::PickupClass);
			for (int i = 0; i < PickupArray.Count; i++)
			{
				AFortPickup* PickupActor = reinterpret_cast<AFortPickup*>(PickupArray[i]);
				if (!PickupActor) continue;

				bool PickupIsA = UGameplayStatics::ObjectIsA(PickupActor, Classes::PickupClass);
				if (!PickupIsA) continue;

				FVector2D ScreenLocation = FVector2D();
				FVector WorldLocation = PickupActor->K2_GetActorLocation();
				if (PlayerController->ProjectWorldLocationToScreen(WorldLocation, &ScreenLocation))
				{
					int Distance = UKismetMathLibrary::Vector_Distance(Variables::CameraLocation, WorldLocation) * 0.01;
					if (ScreenLocation.X < 5.0 || ScreenLocation.X > Variables::ScreenSize.X - (5.0 * 2)) continue;
					if (ScreenLocation.Y < 5.0 || ScreenLocation.Y > Variables::ScreenSize.Y - (5.0 * 2)) continue;
					if (Distance > Settings::Environment::MaxDistance) continue;

					if (UFortItemDefinition* PickupItemDefinition = PickupActor->PrimaryPickupItemEntry_ItemDefinition())
					{
						FString DistplayName = PickupItemDefinition->DisplayName().Get();
						FString ConvertedText = UKismetStringLibrary::BuildString_Int(DistplayName, L" ", int(Distance), L"m");
						Wrapper::Text(ConvertedText, ScreenLocation, Custom::GetColorByTier(PickupItemDefinition->Tier()), true, false, false);
					}
				}
			}
		}

		if (Settings::Environment::Container)
		{
			TArray<UObject*> ContainerArray = UGameplayStatics::GetAllActorsOfClass(GWorld, Classes::ContainerClass);
			for (int i = 0; i < ContainerArray.Count; i++)
			{
				ABuildingContainer* ContainerActor = reinterpret_cast<ABuildingContainer*>(ContainerArray[i]);
				if (!ContainerActor) continue;

				bool ContainerIsA = UGameplayStatics::ObjectIsA(ContainerActor, Classes::ContainerClass);
				if (!ContainerIsA) continue;

				FVector2D ScreenLocation = FVector2D();
				FVector WorldLocation = ContainerActor->K2_GetActorLocation();
				if (PlayerController->ProjectWorldLocationToScreen(WorldLocation, &ScreenLocation))
				{
					double Distance = UKismetMathLibrary::Vector_Distance(Variables::CameraLocation, WorldLocation) * 0.01;
					if (ScreenLocation.X < 5.0 || ScreenLocation.X > Variables::ScreenSize.X - (5.0 * 2)) continue;
					if (ScreenLocation.Y < 5.0 || ScreenLocation.Y > Variables::ScreenSize.Y - (5.0 * 2)) continue;
					if (Distance > Settings::Environment::MaxDistance) continue;

					if (*(byte*)(ContainerActor + 0xd92) & (1 << 2)) continue; // bAlreadySearched

					FString ObjectName = UKismetSystemLibrary::GetObjectName(ContainerActor);
					if (UKismetStringLibrary::Contains(ObjectName, FString(L"Tiered_Chest"), true, false))
					{
						FString ConvertedText = UKismetStringLibrary::BuildString_Int(FString(L"Chest"), L" ", int(Distance), L"m");
						Wrapper::Text(ConvertedText, ScreenLocation, FLinearColor(1.f, 1.f, 0.f, 1.f), true, false, false);
					}
					else if (UKismetStringLibrary::Contains(ObjectName, FString(L"Tiered_Ammo"), true, false))
					{
						FString ConvertedText = UKismetStringLibrary::BuildString_Int(FString(L"Ammo"), L" ", int(Distance), L"m");
						Wrapper::Text(ConvertedText, ScreenLocation, FLinearColor(0.5f, 0.5f, 0.5f, 1.f), true, false, false);
					}
				}
			}
		}

		if (Settings::Environment::Trap)
		{
			TArray<UObject*> TrapArray = UGameplayStatics::GetAllActorsOfClass(GWorld, Classes::BuildingTrapClass);
			for (int i = 0; i < TrapArray.Count; i++)
			{
				ABuildingTrap* TrapActor = reinterpret_cast<ABuildingTrap*>(TrapArray[i]);
				if (!TrapActor) continue;

				bool ContainerIsA = UGameplayStatics::ObjectIsA(TrapActor, Classes::BuildingTrapClass);
				if (!ContainerIsA) continue;

				FVector2D ScreenLocation = FVector2D();
				FVector WorldLocation = TrapActor->K2_GetActorLocation();
				if (PlayerController->ProjectWorldLocationToScreen(WorldLocation, &ScreenLocation))
				{
					int Distance = UKismetMathLibrary::Vector_Distance(Variables::CameraLocation, WorldLocation) * 0.01;
					if (ScreenLocation.X < 5.0 || ScreenLocation.X > Variables::ScreenSize.X - (5.0 * 2)) continue;
					if (ScreenLocation.Y < 5.0 || ScreenLocation.Y > Variables::ScreenSize.Y - (5.0 * 2)) continue;
					if (Distance > Settings::Environment::MaxDistance) continue;

					FString ConvertedText = UKismetStringLibrary::BuildString_Double(L"Trap", L" [", Distance, L"m]");
					Wrapper::Text(ConvertedText, ScreenLocation, FLinearColor(0.6f, 0.6f, 0.6f, 1.f), true, false, false);
				}
			}
		}

		double FarestDistance = DBL_MIN;
		AFortProjectileBase* FarestTPProjectile = nullptr;

		TArray<UObject*> ProjectileArray = UGameplayStatics::GetAllActorsOfClass(GWorld, Classes::ProjectileClass);
		for (int i = 0; i < ProjectileArray.Count; i++)
		{
			AFortProjectileBase* ProjectileActor = reinterpret_cast<AFortProjectileBase*>(ProjectileArray[i]);
			if (!ProjectileActor) continue;

			bool ProjectileIsA = UGameplayStatics::ObjectIsA(ProjectileActor, Classes::ProjectileClass);
			if (!ProjectileIsA) continue;

			FVector2D ScreenLocation = FVector2D();
			FVector WorldLocation = ProjectileActor->K2_GetActorLocation();
			if (PlayerController->ProjectWorldLocationToScreen(WorldLocation, &ScreenLocation))
			{
				double Distance = UKismetMathLibrary::Vector_Distance(Variables::CameraLocation, WorldLocation) * 0.01;
				if (Distance < 1500.0)
				{
					if (ProjectileActor->GetOwnerWeapon() == AcknowledgedPawn->CurrentWeapon() && Distance > FarestDistance)
					{
						if (ProjectileActor->GetGameTimeSinceCreation() < 7.5f)
						{
							FarestTPProjectile = ProjectileActor;
							FarestDistance = Distance;
						}
					}
				}

				if (Settings::Environment::Projectile)
				{
					if (ScreenLocation.X < 5.0 || ScreenLocation.X > Variables::ScreenSize.X - (5.0 * 2)) continue;
					if (ScreenLocation.Y < 5.0 || ScreenLocation.Y > Variables::ScreenSize.Y - (5.0 * 2)) continue;
					if (Distance > Settings::Environment::MaxDistance) continue;

					FString ConvertedText = UKismetStringLibrary::BuildString_Int(FString(L"Projectile"), L" ", int(Distance), L"m");
					Wrapper::Text(ConvertedText, ScreenLocation, FLinearColor(0.3f, 0.3f, 0.3f, 1.f), true, false, false);
				}
			}
		}

		if (Settings::Exploits::ProjectileTeleport)
		{
			if (FarestTPProjectile)
			{
				if (UFortProjectileMovementComponent* ProjectileMovementComponent = FarestTPProjectile->ProjectileMovementComponent())
				{
					FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(Variables::CameraLocation, Variables::BulletTeleportHead);
					ProjectileMovementComponent->MoveInterpolationTarget(Variables::BulletTeleportHead, TargetRotation);
					ProjectileMovementComponent->StopMovementImmediately();
				}
			}
		}



		if (Settings::Exploits::SniperTp) {

			TArray<UObject*> projectile_array = UGameplayStatics::GetAllActorsOfClass((UObject*)GWorld, GameplayStatics); // GameplayStatics->
			for (int i = 0; i < projectile_array.Size(); i++) {
				if (!projectile_array.Valid(i)) continue;

				auto projectile = (AActor*)projectile_array[i];
				if (!projectile) continue;

				auto object_name = UKismetSystemLibrary::GetObjectName(projectile); //KismetSystemLibrary->

				if (!UKismetStringLibrary::Contains(object_name.c_str(), L"Bullet", false, false))//KismetStringLibrary->
					continue;

				FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(Variables::CameraLocation, Variables::BulletTeleportHead);
				projectile->K2_SetActorLocationAndRotation(Variables::BulletTeleportHead, TargetRotation);
			}

		}

		if (Settings::Exploits::RapitFire) 

			if (AcknowledgedPawn) {
				{
					auto CurrentWeapon = AcknowledgedPawn->CurrentWeapon();
					if (CurrentWeapon) {
						*(float*)(CurrentWeapon + 0xe34) = 0;
						*(float*)(CurrentWeapon + 0xe38) = 0;
					}
				}
			}

		if (Settings::Exploits::PlayerFly) {
			if (AcknowledgedPawn)
			{
				FVector CurrentLocation = AcknowledgedPawn->K2_GetActorLocation();

				double FlightSpeed = Settings::Exploits::CarSpeed;

				FVector CameraForward = UKismetMathLibrary::GetForwardVector(Variables::CameraRotation) * FlightSpeed;

				FRotator CameraRotationSideway = FRotator(Variables::CameraRotation.Pitch, Variables::CameraRotation.Yaw + 90.0, Variables::CameraRotation.Roll);
				FVector CameraForwardSideway = UKismetMathLibrary::GetForwardVector(CameraRotationSideway) * FlightSpeed; CameraForwardSideway.Z = 0.0;

				//Forwards/Backwards
				if (PlayerController->IsInputKeyDown(Keys::W))
					CurrentLocation += CameraForward;
				else if (PlayerController->IsInputKeyDown(Keys::S))
					CurrentLocation -= CameraForward;

				AcknowledgedPawn->K2_TeleportTo(CurrentLocation, Variables::CameraRotation);

				//Left/Right
				if (PlayerController->IsInputKeyDown(Keys::A))
					CurrentLocation -= CameraForwardSideway;
				else if (PlayerController->IsInputKeyDown(Keys::D))
					CurrentLocation += CameraForwardSideway;

				AcknowledgedPawn->K2_TeleportTo(CurrentLocation, Variables::CameraRotation);

				//Up/Down
				if (PlayerController->IsInputKeyDown(Keys::SpaceBar))
					CurrentLocation.Z += FlightSpeed;
				else if (PlayerController->IsInputKeyDown(Keys::LeftShift))
					CurrentLocation.Z -= FlightSpeed;

				AcknowledgedPawn->K2_TeleportTo(CurrentLocation, Variables::CameraRotation);
			}
		}

		if (Settings::Exploits::NoReload) {
			if (AcknowledgedPawn) {
				auto CurrentWeapon = AcknowledgedPawn->CurrentWeapon();
				if (CurrentWeapon)
				{
					bool bIsReloadingWeapon = *(bool*)(CurrentWeapon + Offsets::bIsReloadingWeapon); //0x358 = bIsReloadingWeapon
					auto Mesh = *(uintptr_t*)(AcknowledgedPawn + Offsets::mesh); // 0x318 = Mesh

					if (bIsReloadingWeapon)
					{
						*(float*)(Mesh + Offsets::GlobalAnimRateScale) = 999; // 0xA60 = GlobalAnimRateScale
					}
					else
					{
						*(float*)(Mesh + Offsets::GlobalAnimRateScale) = 1; // 0xA60 = GlobalAnimRateScale
					}
				}
			}
		}

		if (Settings::Exploits::infinitesprint) {
			if (AcknowledgedPawn) {
				if (GetAsyncKeyState(VK_LSHIFT)) {//&& GetAsyncKeyState(0x57) & 0x8000
					*(float*) AcknowledgedPawn + 0x1e8a, true;//bIsTacticalSprinting   *(float*)
				}
			}
		}

		if (Settings::Exploits::doublepump) {
			if (AcknowledgedPawn) {
				auto CurrentWeapon = AcknowledgedPawn->CurrentWeapon();
				if (CurrentWeapon) {
					*(bool*)(CurrentWeapon + 0x1471) = true;//bIgnoreTryToFireSlotCooldownRestriction
				}
			}
		}

		if (Settings::Exploits::shootinair) {
			if (AcknowledgedPawn) {
				*(float*)AcknowledgedPawn + 0x55d8, true;//bAdsWhileNotOnGround   *(float*)
			}
		}

		if (Settings::Exploits::norecoil) {
			*(float*)(PlayerController + 0x68) = -1;//CustomTimeDilation
		}

		if (Settings::Exploits::insrevive)
		{
			if (AcknowledgedPawn)
			{
				*(float*)(AcknowledgedPawn + 0x4bb0) = 12;//ReviveFromDBNOTime   1
			}
		}

		if (Settings::Exploits::tptoplayer && PlayerController->IsInputKeyDown(Keys::Capslock)) {
			
					auto locations = AcknowledgedPawn->K2_GetActorLocation();
					auto rotation = Variables::CameraRotation;
					AcknowledgedPawn->K2_TeleportTo(locations, rotation);
				
		}

		if (Settings::Exploits::CarFly)
		{
			/*if (AFortAthenaVehicle* CurrentVehicle = AcknowledgedPawn->CurrentVehicle())
			{
				FVector CurrentLocation = CurrentVehicle->K2_GetActorLocation();
				auto current_rotation = Variables::CameraRotation;
				if (PlayerController->IsInputKeyDown(Keys::W)) {
					CurrentLocation.X = CurrentLocation.X - Settings::Exploits::CarSpeed;
					CurrentVehicle->K2_TeleportTo(CurrentLocation, current_rotation);
				}
				if (PlayerController->IsInputKeyDown(Keys::A)) {
					CurrentLocation.Y = CurrentLocation.Y + Settings::Exploits::CarSpeed;
					CurrentVehicle->K2_TeleportTo(CurrentLocation, current_rotation);
				}
				if (PlayerController->IsInputKeyDown(Keys::S)) {
					CurrentLocation.X = CurrentLocation.X + Settings::Exploits::CarSpeed;
					CurrentVehicle->K2_TeleportTo(CurrentLocation, current_rotation);
				}
				if (PlayerController->IsInputKeyDown(Keys::D)) {
					CurrentLocation.Y = CurrentLocation.Y - Settings::Exploits::CarSpeed;
					CurrentVehicle->K2_TeleportTo(CurrentLocation, current_rotation);
				}
				if (PlayerController->IsInputKeyDown(Keys::SpaceBar)) {
					CurrentLocation.Z = CurrentLocation.Z + Settings::Exploits::CarSpeed;
					CurrentVehicle->K2_TeleportTo(CurrentLocation, current_rotation);
				}
			}*/

				if (AFortAthenaVehicle* CurrentVehicle = AcknowledgedPawn->CurrentVehicle())
				{
					FVector CurrentLocation = CurrentVehicle->K2_GetActorLocation();

					double FlightSpeed = Settings::Exploits::CarSpeed;

					FVector CameraForward = UKismetMathLibrary::GetForwardVector(Variables::CameraRotation) * FlightSpeed;

					FRotator CameraRotationSideway = FRotator(Variables::CameraRotation.Pitch, Variables::CameraRotation.Yaw + 90.0, Variables::CameraRotation.Roll);
					FVector CameraForwardSideway = UKismetMathLibrary::GetForwardVector(CameraRotationSideway) * FlightSpeed; CameraForwardSideway.Z = 0.0;

					//Forwards/Backwards
					if (PlayerController->IsInputKeyDown(Keys::W))
						CurrentLocation += CameraForward;
					else if (PlayerController->IsInputKeyDown(Keys::S))
						CurrentLocation -= CameraForward;

					CurrentVehicle->K2_TeleportTo(CurrentLocation, Variables::CameraRotation);

					//Left/Right
					if (PlayerController->IsInputKeyDown(Keys::A))
						CurrentLocation -= CameraForwardSideway;
					else if (PlayerController->IsInputKeyDown(Keys::D))
						CurrentLocation += CameraForwardSideway;

					CurrentVehicle->K2_TeleportTo(CurrentLocation, Variables::CameraRotation);

					//Up/Down
					if (PlayerController->IsInputKeyDown(Keys::SpaceBar))
						CurrentLocation.Z += FlightSpeed;
					else if (PlayerController->IsInputKeyDown(Keys::LeftShift))
						CurrentLocation.Z -= FlightSpeed;

					CurrentVehicle->K2_TeleportTo(CurrentLocation, Variables::CameraRotation);
				}
		}

		if (Settings::Exploits::CameraFOVChanger) {
			if (AcknowledgedPawn) {
				PlayerController->FOV((float)Settings::Exploits::CameraFOV);
			}
		}

		if (Settings::Environment::SupplyDrop)
		{
			TArray<UObject*> SupplyDropArray = UGameplayStatics::GetAllActorsOfClass(GWorld, Classes::SupplyDropClass);
			for (int i = 0; i < SupplyDropArray.Count; i++)
			{
				AFortAthenaSupplyDrop* SupplyDropActor = reinterpret_cast<AFortAthenaSupplyDrop*>(SupplyDropArray[i]);
				if (!SupplyDropActor) continue;

				bool SupplyDropIsA = UGameplayStatics::ObjectIsA(SupplyDropActor, Classes::SupplyDropClass);
				if (!SupplyDropIsA) continue;

				FVector2D ScreenLocation = FVector2D();
				FVector WorldLocation = SupplyDropActor->K2_GetActorLocation();
				if (PlayerController->ProjectWorldLocationToScreen(WorldLocation, &ScreenLocation))
				{
					double Distance = UKismetMathLibrary::Vector_Distance(Variables::CameraLocation, WorldLocation) * 0.01;
					if (ScreenLocation.X < 5.0 || ScreenLocation.X > Variables::ScreenSize.X - (5.0 * 2)) continue;
					if (ScreenLocation.Y < 5.0 || ScreenLocation.Y > Variables::ScreenSize.Y - (5.0 * 2)) continue;
					if (Distance > Settings::Environment::MaxDistance) continue;

					Wrapper::Text(FString(L"SupplyDrop"), ScreenLocation, FLinearColor(0.678f, 0.847f, 0.902f, 1.0f), true, false, false);
				}
			}
		}

		if (Settings::Environment::Weakspot)
		{
			double ClosestDistanceToCenter = DBL_MAX;
			ABuildingWeakSpot* TargetWeakSpot = nullptr;

			TArray<UObject*> WeakspotArray = UGameplayStatics::GetAllActorsOfClass((UObject*)GWorld, Classes::WeakspotClass);
			for (int i = 0; i < WeakspotArray.Count; i++)
			{
				ABuildingWeakSpot* WeakspotActor = reinterpret_cast<ABuildingWeakSpot*>(WeakspotArray[i]); // ABuildingWeakSpot
				if (!WeakspotActor) continue;

				bool WeakspotIsA = UGameplayStatics::ObjectIsA(WeakspotActor, Classes::WeakspotClass);
				if (!WeakspotIsA) continue;

				FVector2D ScreenLocation = FVector2D();
				FVector WorldLocation = WeakspotActor->K2_GetActorLocation();
				if (PlayerController->ProjectWorldLocationToScreen(WorldLocation, &ScreenLocation))
				{
					double Distance = UKismetMathLibrary::Vector_Distance(Variables::CameraLocation, WorldLocation) * 0.01;
					if (ScreenLocation.X < 5.0 || ScreenLocation.X > Variables::ScreenSize.X - (5.0 * 2)) continue;
					if (ScreenLocation.Y < 5.0 || ScreenLocation.Y > Variables::ScreenSize.Y - (5.0 * 2)) continue;
					if (Distance > 10.0) continue;

					if (*(byte*)(WeakspotActor + 0x2c0) & (1 << 0)) continue; // bHit
					if (!(*(byte*)(WeakspotActor + 0x2c0) & (1 << 2))) continue; // bActive

					Wrapper::Text(FString(L"Weakspot"), ScreenLocation, FLinearColor(1.f, 0.f, 0.f, 1.f), true, false, false);

					double DistanceToCenter = UKismetMathLibrary::Vector_Distance2D(Variables::ScreenCenter, ScreenLocation);
					if (DistanceToCenter < ClosestDistanceToCenter)
					{
						ClosestDistanceToCenter = DistanceToCenter;
						TargetWeakSpot = WeakspotActor;
					}
				}
			}

			if (TargetWeakSpot)
			{
				if (AFortWeapon* CurrentWeapon = AcknowledgedPawn->CurrentWeapon())
				{
					if (UGameplayStatics::ObjectIsA(CurrentWeapon, Classes::FortWeaponClass))
					{
						bool IsHoldingPickaxe = UGameplayStatics::ObjectIsA(CurrentWeapon, Classes::PickaxeClass);
						{
							FVector WorldLocation = TargetWeakSpot->K2_GetActorLocation();
							FVector2D ScreenLocation = FVector2D();
							if (PlayerController->ProjectWorldLocationToScreen(WorldLocation, &ScreenLocation))
							{
								if (PlayerController->IsInputKeyDown(Keys::RightMouseButton))
								{
									if (Settings::Aimbot::AimType == 1)
									{
										FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(Variables::CameraLocation, WorldLocation);

										Custom::AddInput(GWorld, PlayerController, Variables::CameraRotation, TargetRotation, Settings::Aimbot::Interpolate, Settings::Aimbot::Constant, Settings::Aimbot::Speed);
									}
								}
							}
						}
					}
				}
			}
		}

		if (Settings::Environment::Vehicle)
		{
			TArray<UObject*> VehicleArray = UGameplayStatics::GetAllActorsOfClass(GWorld, Classes::VehicleClass);
			for (int i = 0; i < VehicleArray.Count; i++)
			{
				AFortAthenaVehicle* VehicleActor = reinterpret_cast<AFortAthenaVehicle*>(VehicleArray[i]);
				if (!VehicleActor) continue;

				bool VehicleIsA = UGameplayStatics::ObjectIsA(VehicleActor, Classes::VehicleClass);
				if (!VehicleIsA) continue;

				FVector2D ScreenLocation = FVector2D();
				FVector WorldLocation = VehicleActor->K2_GetActorLocation();
				if (PlayerController->ProjectWorldLocationToScreen(WorldLocation, &ScreenLocation))
				{
					double Distance = UKismetMathLibrary::Vector_Distance(Variables::CameraLocation, WorldLocation) * 0.01;
					if (ScreenLocation.X < 5.0 || ScreenLocation.X > Variables::ScreenSize.X - (5.0 * 2)) continue;
					if (ScreenLocation.Y < 5.0 || ScreenLocation.Y > Variables::ScreenSize.Y - (5.0 * 2)) continue;
					if (Distance > Settings::Environment::MaxDistance) continue;

					Wrapper::Text(FString(L"Vehicle"), ScreenLocation, FLinearColor(1.f, 0.f, 0.f, 1.f), true, false, false);
				}
			}
		}
	}

	TArray<UObject*> PlayerList = UGameplayStatics::GetAllActorsOfClass(GWorld, Classes::PlayerClass);
	for (int32_t i = 0; i < PlayerList.Count; i++)
	{
		APlayerPawn_Athena_C* Player = reinterpret_cast<APlayerPawn_Athena_C*>(PlayerList[i]);
		if (!Player)
			continue;

		bool PlayerCheck = UGameplayStatics::ObjectIsA(Player, Classes::PlayerClass);
		if (PlayerCheck)
		{
			if (Player == AcknowledgedPawn) continue;

			bool Wound = Player->IsDBNO();

			bool Dead = Player->IsDead();
			if (Dead) continue;

			char Team = Player->GetTeam();
			bool Teammate = UpdateMyTeam != char(1337) && UpdateMyTeam == Team;

			bool Visable = UFortKismetLibrary::CheckLineOfSightToActorWithChannel(Variables::CameraLocation, Player, ECollisionChannel::ECC_Visibility, nullptr);

			USkeletalMeshComponent* Mesh = Player->Mesh();
			if (!Mesh) continue;

			if (Settings::Player::Wireframe)
			{
				int DepthStencil = 11;

				Mesh->SetCustomDepthStencilValue(DepthStencil);

				TArray<USkeletalMeshComponent*> SkeletalMeshes = Player->SkeletalMeshes();
				for (int si = 0; si < SkeletalMeshes.Count; si++)
				{
					USkeletalMeshComponent* SkeletalMesh = SkeletalMeshes[si];
					if (!SkeletalMesh) continue;

					SkeletalMesh->SetRenderCustomDepth(true);
					SkeletalMesh->SetCustomDepthStencilValue(DepthStencil);
				}

				//UMaterial* MaterialObj = (UMaterial*)UObject::FindObject(L"Material M_Elimination_DigitizeCubes.M_Elimination_DigitizeCubes");
				//*(byte*)(MaterialObj + 0x1a8) |= 1 << 0; // bDisableDepthTest
				//printf("bDisableDepthTest: %i\n", MaterialObj + 0x1a8);

				//TArray<UMaterialInterface*> Materials = Mesh->GetMaterials();
				//for (int j = 0; j < Materials.Count; j++) {
				//	Mesh->SetMaterial(j, (UMaterialInterface*)MaterialObj);
				//}

				TArray<UMaterialInstanceDynamic*> MaterialInstances = Player->PawnMaterials_ALL();
				for (int mi = 0; mi < MaterialInstances.Count; mi++)
				{
					UMaterialInstanceDynamic* MaterialInstance = MaterialInstances[mi];
					if (!MaterialInstance) continue;

					MaterialInstance->SetVectorParameterValue(Variables::MaterialParameter4, Settings::Colors::SkinColor);

					UMaterial* Material = MaterialInstance->GetMaterial();
					if (!Material) continue;

					*(byte*)(Material + 0x1c0) |= 1 << 6; // Wireframe
					*(byte*)(Material + 0x1b0) |= 1 << 0; // bDisableDepthTest
				}

				Mesh->SetRenderCustomDepth(true);

			}

			FLinearColor BoxColor = Settings::Colors::BoxInVisible;
			FLinearColor SkeletonColor = Settings::Colors::SkeletonInVisible;
			FLinearColor SnaplineColor = Settings::Colors::SkeletonInVisible;

			if (Visable)
			{
				BoxColor = Settings::Colors::BoxVisible;
				SkeletonColor = Settings::Colors::SkeletonVisible;
				SnaplineColor = Settings::Colors::SnaplineVisible;
			}

			if (Teammate)
			{
				BoxColor = Settings::Colors::TeammateColor;
				SkeletonColor = Settings::Colors::TeammateColor;
				SnaplineColor = Settings::Colors::TeammateColor;
			}

			FVector Root = Mesh->GetBoneLocation(Bones::Root);
			double Distance = UKismetMathLibrary::Vector_Distance(Variables::CameraLocation, Root) * 0.01;

			if (Settings::Player::Minimap && AcknowledgedPawn)
			{
				if (UFortClientSettingsRecord* ClientSettingsRecord = LocalPlayer->ClientSettingsRecord())
				{
					float HUDScale = ClientSettingsRecord->HUDScale();

					double RadarSize = (15.625 * Variables::ScreenCenter.X * double(HUDScale) / 100.0) * 2;

					double RadarPositionOffset = RadarSize / 30.0;
					FVector2D RadarPosition = FVector2D(Variables::ScreenSize.X - RadarSize - RadarPositionOffset, RadarPositionOffset);

					Wrapper::Rect(RadarPosition, RadarPosition + FVector2D(RadarSize, RadarSize), FLinearColor(0.f, 0.f, 0.f, 1.f), 1.f);

					Radar::Add(Root, Update_LocalHead, BoxColor, RadarPosition, FVector2D(RadarSize, RadarSize), 5.0);
				}
			}

			FVector Head = Mesh->GetBoneLocation(Bones::head);
			FVector2D HeadScreen = FVector2D();
			if (!PlayerController->ProjectWorldLocationToScreen(Head, &HeadScreen)) continue;

			if (HeadScreen.X < 5.0 || HeadScreen.X > Variables::ScreenSize.X - (5.0 * 2)) continue;
			if (HeadScreen.Y < 5.0 || HeadScreen.Y > Variables::ScreenSize.Y - (5.0 * 2)) continue;

			if (UCharacterMovementComponent* MovementComponent = Player->CharacterMovement())
			{
				Update_ShootOutOfAir = MovementComponent->MovementMode() == EMovementMode::MOVE_Falling;
			}

			bool AimbotVisable = (Settings::Aimbot::VisableOnly && Visable) || !Settings::Aimbot::VisableOnly;
			bool AimbotKnocked = (Settings::Aimbot::SkipKnocked && !Wound) || !Settings::Aimbot::SkipKnocked;
			bool AimbotValid = (AimbotVisable && AimbotKnocked && !Teammate) && Distance < Settings::Aimbot::MaxDistance;
			if (AimbotValid) {
				if (Distance < ClosestDistanceToMyself) {
					double DistanceToCenter = UKismetMathLibrary::Vector_Distance2D(Variables::ScreenCenter, HeadScreen);
					if (DistanceToCenter < ClosestDistanceToCenter) {
						ClosestDistanceToMyself = Distance;
						ClosestDistanceToCenter = DistanceToCenter;
						TargetPlayer = Player;
					}
				}
			}

			if (Distance > Settings::Player::MaxDistance) continue;

			if (Settings::Player::Target && Update_LocalHead)
			{
				FVector EnemyCameraLocation = Mesh->GetBoneLocation(Bones::Camera);

				FRotator PerfectRotation = UKismetMathLibrary::FindLookAtRotation(EnemyCameraLocation, Update_LocalHead);
				FRotator CurrentRotation = Player->K2_GetActorRotation();

				double Degrees = PerfectRotation.Yaw - CurrentRotation.Yaw;

				if (Degrees < 0) Degrees *= -1;

				if (Degrees < SmallestDegrees)
				{
					SmallestDegreesOwnerWorldLocation = Head;
					SmallestDegrees = Degrees;
				}
			}

			FVector2D RootScreen;
			if (!PlayerController->ProjectWorldLocationToScreen(Root, &RootScreen)) continue;

			FVector Root2 = FVector(Root.X, Root.Y, Root.Z + 6.0);
			FVector2D RootScreen2;
			if (!PlayerController->ProjectWorldLocationToScreen(Root2, &RootScreen2)) continue;

			FVector Head2 = FVector(Head.X, Head.Y, Head.Z + 18.0);
			FVector2D HeadScreen2;
			if (!PlayerController->ProjectWorldLocationToScreen(Head2, &HeadScreen2)) continue;

			FVector Neck = Mesh->GetBoneLocation(Bones::neck_02);
			FVector2D NeckScreen = FVector2D();
			if (!PlayerController->ProjectWorldLocationToScreen(Neck, &NeckScreen)) continue;

			FVector Chest = Mesh->GetBoneLocation(Bones::spine_05);
			FVector2D ChestScreen = FVector2D();
			if (!PlayerController->ProjectWorldLocationToScreen(Chest, &ChestScreen)) continue;

			FVector Pelvis = Mesh->GetBoneLocation(Bones::pelvis);
			FVector2D PelvisScreen = FVector2D();
			if (!PlayerController->ProjectWorldLocationToScreen(Pelvis, &PelvisScreen)) continue;

			FVector RShoulder = Mesh->GetBoneLocation(Bones::upperarm_r);
			FVector2D RShoulderScreen = FVector2D();
			if (!PlayerController->ProjectWorldLocationToScreen(RShoulder, &RShoulderScreen)) continue;

			FVector RElbow = Mesh->GetBoneLocation(Bones::lowerarm_r);
			FVector2D RElbowScreen = FVector2D();
			if (!PlayerController->ProjectWorldLocationToScreen(RElbow, &RElbowScreen)) continue;

			FVector RElbow2 = FVector(RElbow.X + 8.0, RElbow.Y, RElbow.Z);
			FVector2D RElbowScreen2 = FVector2D();
			if (!PlayerController->ProjectWorldLocationToScreen(RElbow2, &RElbowScreen2)) continue;

			FVector RHand = Mesh->GetBoneLocation(Bones::hand_r);
			FVector2D RHandScreen = FVector2D();
			if (!PlayerController->ProjectWorldLocationToScreen(RHand, &RHandScreen)) continue;

			FVector RThigh = Mesh->GetBoneLocation(Bones::thigh_r);
			FVector2D RThighScreen = FVector2D();
			if (!PlayerController->ProjectWorldLocationToScreen(RThigh, &RThighScreen)) continue;

			FVector RCalf = Mesh->GetBoneLocation(Bones::calf_r);
			FVector2D RCalfScreen = FVector2D();
			if (!PlayerController->ProjectWorldLocationToScreen(RCalf, &RCalfScreen)) continue;

			FVector RKnee = Mesh->GetBoneLocation(Bones::calf_twist_01_r);
			FVector2D RKneeScreen = FVector2D();
			if (!PlayerController->ProjectWorldLocationToScreen(RKnee, &RKneeScreen)) continue;

			FVector RFoot = Mesh->GetBoneLocation(Bones::foot_r);
			FVector2D RFootScreen = FVector2D();
			if (!PlayerController->ProjectWorldLocationToScreen(RFoot, &RFootScreen)) continue;

			FVector LShoulder = Mesh->GetBoneLocation(Bones::upperarm_l);
			FVector2D LShoulderScreen = FVector2D();
			if (!PlayerController->ProjectWorldLocationToScreen(LShoulder, &LShoulderScreen)) continue;

			FVector LElbow = Mesh->GetBoneLocation(Bones::lowerarm_l);
			FVector2D LElbowScreen = FVector2D();
			if (!PlayerController->ProjectWorldLocationToScreen(LElbow, &LElbowScreen)) continue;

			FVector LElbow2 = FVector(LElbow.X + 8.0, LElbow.Y, LElbow.Z);
			FVector2D LElbowScreen2 = FVector2D();
			if (!PlayerController->ProjectWorldLocationToScreen(LElbow2, &LElbowScreen2)) continue;

			FVector LHand = Mesh->GetBoneLocation(Bones::hand_l);
			FVector2D LHandScreen = FVector2D();
			if (!PlayerController->ProjectWorldLocationToScreen(LHand, &LHandScreen)) continue;

			FVector LThigh = Mesh->GetBoneLocation(Bones::thigh_l);
			FVector2D LThighScreen = FVector2D();
			if (!PlayerController->ProjectWorldLocationToScreen(LThigh, &LThighScreen)) continue;

			FVector LCalf = Mesh->GetBoneLocation(Bones::calf_l);
			FVector2D LCalfScreen = FVector2D();
			if (!PlayerController->ProjectWorldLocationToScreen(LCalf, &LCalfScreen)) continue;

			FVector LKnee = Mesh->GetBoneLocation(Bones::calf_twist_01_l);
			FVector2D LKneeScreen = FVector2D();
			if (!PlayerController->ProjectWorldLocationToScreen(LKnee, &LKneeScreen)) continue;

			FVector LFoot = Mesh->GetBoneLocation(Bones::foot_l);
			FVector2D LFootScreen = FVector2D();
			if (!PlayerController->ProjectWorldLocationToScreen(LFoot, &LFootScreen)) continue;

			FVector2D BonesToCheck[] = { RootScreen, RootScreen2, HeadScreen2, NeckScreen, ChestScreen, PelvisScreen, RShoulderScreen, RElbowScreen, RElbowScreen2, RHandScreen, RCalfScreen, RKneeScreen, RFootScreen, LShoulderScreen, LElbowScreen, LElbowScreen2, LHandScreen, LCalfScreen, LKneeScreen, LFootScreen };

			double MostLeft = DBL_MAX;
			double MostRight = DBL_MIN;
			double MostTop = DBL_MAX;
			double MostBottom = DBL_MIN;

			for (int ci = 0; ci < sizeof(BonesToCheck) / sizeof(FVector2D); ci++)
			{
				FVector2D CurrentBone = BonesToCheck[ci];

				if (CurrentBone.X < MostLeft)
					MostLeft = CurrentBone.X;

				if (CurrentBone.X > MostRight)
					MostRight = CurrentBone.X;

				if (CurrentBone.Y < MostTop)
					MostTop = CurrentBone.Y;

				if (CurrentBone.Y > MostBottom)
					MostBottom = CurrentBone.Y;
			}

			double ActorHeight = MostBottom - MostTop;
			double ActorWidth = MostRight - MostLeft;

			double DistanceDifference = 225.0 - Distance;
			double DistanceOffset = DistanceDifference * 0.03;
			double CornerWidth = ActorWidth / 4;
			double CornerHeight = ActorHeight / 3;
			double ThreeDimensionalWidth = ActorWidth / 3;

			double TopTextOffset = 22.0;
			double BottomTextOffset = 7.0;

			FVector2D BottomMiddle = FVector2D(MostLeft + (ActorWidth / 2.0), MostBottom);

			if (Settings::Player::Skeleton)
			{
				Wrapper::Line(HeadScreen, NeckScreen, SkeletonColor, 5.f);
				Wrapper::Line(NeckScreen, ChestScreen, SkeletonColor, 5.f);
				Wrapper::Line(ChestScreen, PelvisScreen, SkeletonColor, 5.f);

				Wrapper::Line(ChestScreen, RShoulderScreen, SkeletonColor, 5.f);
				Wrapper::Line(RShoulderScreen, RElbowScreen, SkeletonColor, 5.f);
				Wrapper::Line(RElbowScreen, RHandScreen, SkeletonColor, 5.f);

				Wrapper::Line(ChestScreen, LShoulderScreen, SkeletonColor, 5.f);
				Wrapper::Line(LShoulderScreen, LElbowScreen, SkeletonColor, 5.f);
				Wrapper::Line(LElbowScreen, LHandScreen, SkeletonColor, 5.f);

				Wrapper::Line(PelvisScreen, RThighScreen, SkeletonColor, 5.f);
				Wrapper::Line(RThighScreen, RCalfScreen, SkeletonColor, 5.f);
				Wrapper::Line(RCalfScreen, RKneeScreen, SkeletonColor, 5.f);
				Wrapper::Line(RKneeScreen, RFootScreen, SkeletonColor, 5.f);

				Wrapper::Line(PelvisScreen, LThighScreen, SkeletonColor, 5.f);
				Wrapper::Line(LThighScreen, LCalfScreen, SkeletonColor, 5.f);
				Wrapper::Line(LCalfScreen, LKneeScreen, SkeletonColor, 5.f);
				Wrapper::Line(LKneeScreen, LFootScreen, SkeletonColor, 5.f);
			}

			if (Settings::Player::BoxType == 1)
			{
				Wrapper::Line(FVector2D(MostLeft - DistanceOffset, MostTop - DistanceOffset), FVector2D(MostRight + DistanceOffset, MostTop - DistanceOffset), BoxColor, 3.5f);
				Wrapper::Line(FVector2D(MostLeft - DistanceOffset, MostBottom + DistanceOffset), FVector2D(MostRight + DistanceOffset, MostBottom + DistanceOffset), BoxColor, 3.5f);
				Wrapper::Line(FVector2D(MostLeft - DistanceOffset, MostBottom + DistanceOffset), FVector2D(MostLeft - DistanceOffset, MostTop - DistanceOffset), BoxColor, 3.5f);
				Wrapper::Line(FVector2D(MostRight + DistanceOffset, MostTop - DistanceOffset), FVector2D(MostRight + DistanceOffset, MostBottom + DistanceOffset), BoxColor, 3.5f);
			}
			else if (Settings::Player::BoxType == 2)
			{
				Wrapper::Line(FVector2D(MostLeft - DistanceOffset, MostTop - DistanceOffset), FVector2D(MostLeft - DistanceOffset + CornerWidth, MostTop - DistanceOffset), BoxColor, 3.f);
				Wrapper::Line(FVector2D(MostLeft - DistanceOffset, MostTop - DistanceOffset), FVector2D(MostLeft - DistanceOffset, MostTop - DistanceOffset + CornerHeight), BoxColor, 3.f);

				Wrapper::Line(FVector2D(MostRight + DistanceOffset, MostTop - DistanceOffset), FVector2D(MostRight + DistanceOffset - CornerWidth, MostTop - DistanceOffset), BoxColor, 3.f);
				Wrapper::Line(FVector2D(MostRight + DistanceOffset, MostTop - DistanceOffset), FVector2D(MostRight + DistanceOffset, MostTop - DistanceOffset + CornerHeight), BoxColor, 3.f);

				Wrapper::Line(FVector2D(MostLeft - DistanceOffset, MostBottom + DistanceOffset), FVector2D(MostLeft - DistanceOffset + CornerWidth, MostBottom + DistanceOffset), BoxColor, 3.f);
				Wrapper::Line(FVector2D(MostLeft - DistanceOffset, MostBottom + DistanceOffset), FVector2D(MostLeft - DistanceOffset, MostBottom + DistanceOffset - CornerHeight), BoxColor, 3.f);

				Wrapper::Line(FVector2D(MostRight + DistanceOffset, MostBottom + DistanceOffset), FVector2D(MostRight + DistanceOffset - CornerWidth, MostBottom + DistanceOffset), BoxColor, 3.f);
				Wrapper::Line(FVector2D(MostRight + DistanceOffset, MostBottom + DistanceOffset), FVector2D(MostRight + DistanceOffset, MostBottom + DistanceOffset - CornerHeight), BoxColor, 3.f);
			}

			if (Settings::Player::Lines)
			{
				Wrapper::Line(FVector2D(Variables::ScreenSize.X / 2, Variables::ScreenSize.Y), RootScreen, SnaplineColor, 2.0f);
			}

			if (Settings::Player::Distance)
			{
				FString ConvertedText = UKismetStringLibrary::BuildString_Int(L"", L"", int(Distance), L"m");
				Wrapper::Text(ConvertedText, FVector2D(BottomMiddle.X, MostTop - TopTextOffset), FLinearColor(1.f, 1.f, 1.f, 1.f), true, false, false);
				TopTextOffset += 14.0;
			}

			if (Settings::Player::Name)
			{
				if (APlayerState* PlayerState = Player->PlayerState())
				{
					if (FString Username = PlayerState->GetPlayerName())
					{
						FString ConvertedText = UKismetStringLibrary::BuildString_Int(Username, L" [", PlayerState->SeasonLevelUIDisplay(), L"]");
						Wrapper::Text(ConvertedText, FVector2D(BottomMiddle.X, MostTop - TopTextOffset), FLinearColor(1.f, 1.f, 0.f, 1.f), true, false, false);
						TopTextOffset += 14.0;
					}
				}
			}

			if (Settings::Player::Weapon)
			{
				if (AFortWeapon* CurrentWeapon = Player->CurrentWeapon())
				{
					if (UFortItemDefinition* WeaponData = CurrentWeapon->WeaponData())
					{
						if (UGameplayStatics::ObjectIsA(CurrentWeapon, Classes::FortWeaponClass))
						{
							bool IsHoldingPickaxe = UGameplayStatics::ObjectIsA(CurrentWeapon, Classes::PickaxeClass);
							if (IsHoldingPickaxe)
							{
								Wrapper::Text(L"Harvesting Tool", FVector2D(BottomMiddle.X, MostBottom + BottomTextOffset), FLinearColor(1.f, 1.f, 1.f, 1.f), true, false, false);
							}
							else
							{
								FString DisplayName = WeaponData->DisplayName().Get();
								Wrapper::Text(DisplayName, FVector2D(BottomMiddle.X, MostBottom + BottomTextOffset), Custom::GetColorByTier(WeaponData->Tier()), true, false, false);
								BottomTextOffset += 14.0;

								int CurrentAmmo = CurrentWeapon->GetMagazineAmmoCount();
								int MaxAmmo = CurrentWeapon->GetBulletsPerClip();
								bool Reloading = CurrentWeapon->IsReloading();

								//reload bar vars
								double CurrentPercentage = double((double)CurrentAmmo / (double)MaxAmmo);
								double BarWidth = 120;
								double BarHeight = 13;

								//reload bar slider colors
								float green = 255.f * CurrentPercentage;
								float red = 255.f - green;
								red /= 255.f;
								green /= 255.f;

								FLinearColor BarColor = FLinearColor(red, green, 0.f, 1.f);
								if (Reloading)
								{
									CurrentPercentage = CurrentWeapon->GetReloadProgress();

									BarColor = FLinearColor(1.f, 0.f, 1.f, 1.f);
								}

								//reload bar main
								Wrapper::RectFilled(FVector2D(BottomMiddle.X - BarWidth / 2.2, MostBottom + BottomTextOffset), FVector2D(BarWidth, BarHeight), FLinearColor(0.03f, 0.03f, 0.03f, 1.f));

								//reload bar slider
								Wrapper::RectFilled(FVector2D(BottomMiddle.X - BarWidth / 2.2, MostBottom + BottomTextOffset), FVector2D(BarWidth * CurrentPercentage, BarHeight), BarColor);
							}
						}
					}
				}
			}

			if (Settings::Player::ViewAngle)
			{
				if (USceneComponent* RootComponent = Player->RootComponent())
				{
					FVector2D RotationScreen = FVector2D();
					FRotator Rotation = RootComponent->RelativeRotation() * 360.0;
					FVector RotationWorld = FVector(Head + Rotation.Euler()) / 2.0;
					if (PlayerController->ProjectWorldLocationToScreen(RotationWorld, &RotationScreen))
					{
						Wrapper::Line(HeadScreen, RotationScreen, SnaplineColor, 1.0f);
					}
				}
			}
		}
	}

	if (AcknowledgedPawn)
	{
		if (Settings::Aimbot::AimType)
		{
			if (Settings::Aimbot::DrawFOV)
			{
				Wrapper::Circle(Variables::ScreenCenter, Settings::Colors::FieldOfView, AimbotFOV, 128.0);
			}

			FKey AimbotKey = Keys::RightMouseButton;

			if (Settings::Aimbot::KeyType == 1)
				AimbotKey = Keys::LeftMouseButton;
			else if (Settings::Aimbot::KeyType == 2)
				AimbotKey = Keys::LeftShift;

			if (APlayerPawn_Athena_C* Target = TargetPlayer)
			{
				if (USkeletalMeshComponent* Mesh = Target->Mesh())
				{
					if (AFortWeapon* CurrentWeapon = AcknowledgedPawn->CurrentWeapon())
					{
						bool IsHoldingWeapon = UGameplayStatics::ObjectIsA(CurrentWeapon, Classes::RangedWeaponClass);
						if (IsHoldingWeapon)
						{
							bool IsHoldingPickaxe = UGameplayStatics::ObjectIsA(CurrentWeapon, Classes::PickaxeClass);
							if (!IsHoldingPickaxe)
							{
								static Bones TargetBoneIndex = Bones::head;

								if (Settings::Aimbot::BoneType == 1)
									TargetBoneIndex = Bones::neck_02;
								else if (Settings::Aimbot::BoneType == 2)
									TargetBoneIndex = Bones::spine_05;
								else if (Settings::Aimbot::BoneType == 3) {
									static double LastBoneChangeTime = 0.00;
									const Bones BoneArray[] = {
										Bones::neck_01,
										Bones::neck_02,
										Bones::spine_01,
										Bones::spine_02,
										Bones::spine_03,
										Bones::clavicle_r,
										Bones::clavicle_l
									};

									const int NumBones = sizeof(BoneArray) / sizeof(Bones);

									if (UKismetSystemLibrary::GetGameTimeInSeconds(GWorld) - LastBoneChangeTime >= 1.00)
									{
										int32_t RandomIndex = UKismetMathLibrary::RandomIntegerInRange(0, NumBones - 1);
										TargetBoneIndex = BoneArray[RandomIndex];

										LastBoneChangeTime = UKismetSystemLibrary::GetGameTimeInSeconds(GWorld);
									}
								}


								FVector2D TargetScreen = FVector2D();
								FVector TargetWorld = Mesh->GetBoneLocation(TargetBoneIndex);
								if (PlayerController->ProjectWorldLocationToScreen(TargetWorld, &TargetScreen))
								{
									if (Custom::InCircle(Variables::ScreenCenter.X, Variables::ScreenCenter.Y, AimbotFOV, TargetScreen.X, TargetScreen.Y))
									{
										Variables::BulletTeleportHead = TargetWorld;

										if (Settings::Aimbot::TargetLine)
										{
											FVector2D MuzzleScreen = FVector2D();
											FVector Muzzle = CurrentWeapon->GetMuzzleLocation();
											if (PlayerController->ProjectWorldLocationToScreen(Muzzle, &MuzzleScreen))
											{
												Wrapper::Line(MuzzleScreen, TargetScreen, Settings::Colors::TargetLine, 1.f);
											}
										}

										/*if (Settings::Aimbot::Triggerbot && PlayerController->IsInputKeyDown(AimbotKey))
										{
											for (int i = 0; i < CurrentWeapon->AmmoCount(); i++)
											{
												reinterpret_cast<void(__cdecl*)(AFortWeapon*, uintptr_t, void*)>(DoSpoofCall)(CurrentWeapon, SpoofCode, (void*)(GameBase + Offsets::Fire));
											}
										}*/


										if (Settings::Aimbot::Triggerbot)
										{
											bool TriggerbotAimkey = (Settings::Aimbot::TriggerBot::OnlyOnAimKey && PlayerController->IsInputKeyDown(AimbotKey)) || !Settings::Aimbot::TriggerBot::OnlyOnAimKey;
											bool TriggerbotTargeting = (Settings::Aimbot::TriggerBot::FireOnlyOnAimIn && *(byte*)(CurrentWeapon + 0xd30) & (1 << 5)) || !Settings::Aimbot::TriggerBot::FireOnlyOnAimIn;
											bool TriggerbotProjectile = (Settings::Aimbot::TriggerBot::NonProjectileWeapons && !CurrentWeapon->IsProjectileWeapon()) || !Settings::Aimbot::TriggerBot::NonProjectileWeapons;
											if (TriggerbotAimkey && TriggerbotTargeting && TriggerbotProjectile)
											{
												for (int i = 0; i < CurrentWeapon->AmmoCount(); i++)
												{
													static float OldLastFireAbilityTime = 0.0f;
													printf("LastFireAbilityTime: %f\n", CurrentWeapon->LastFireAbilityTime());
													printf("OldLastFireAbilityTime: %f\n", OldLastFireAbilityTime);
													if (CurrentWeapon->LastFireAbilityTime() != OldLastFireAbilityTime)
													{
														reinterpret_cast<void(__cdecl*)(AFortWeapon*, uintptr_t, void*)>(DoSpoofCall)(CurrentWeapon, SpoofCode, (void*)(GameBase + Offsets::Fire));

														OldLastFireAbilityTime = CurrentWeapon->LastFireAbilityTime();
													}
												}
											}
										}


										static float Update_ActorTimeDilation = 0.f;
										if (Settings::Aimbot::Backtrack)
										{
											if (Target->GetActorTimeDilation() > 0.f)
											{
												Update_ActorTimeDilation = Target->GetActorTimeDilation();
												*(float*)(Target + 0x68) = 0.f;
											}
										}


										if (Settings::Aimbot::AimType == 1)
										{
											if (PlayerController->IsInputKeyDown(AimbotKey))
											{
												FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(Variables::CameraLocation, TargetWorld);

												if (Settings::Aimbot::PredictProjectiles)
												{
													FVector TargetForward = UKismetMathLibrary::GetForwardVector(TargetRotation);
													float ChargePercent = CurrentWeapon->GetChargePercent();
													float ProjectileSpeed = CurrentWeapon->GetProjectileSpeed(ChargePercent);

													double ProjectileGravity = 3.0; // Hardcoded for life
													double TimeToTarget = ClosestDistanceToMyself / ProjectileSpeed;
													FVector CurrentVelocity = TargetForward * double(ProjectileSpeed);
													printf("ClosestDistanceToMyself: %d\n", ClosestDistanceToMyself);

													TargetWorld += FVector(CurrentVelocity.X * TimeToTarget, CurrentVelocity.Y * TimeToTarget, CurrentVelocity.Z * TimeToTarget + (UKismetMathLibrary::abs(ProjectileGravity * -980.0) * (TimeToTarget * TimeToTarget)) * 0.5);
												}

												if (Settings::Aimbot::Shake)
												{
													double CurrentTime = UGameplayStatics::GetTimeSeconds(GWorld);

													double ShakeFactor = UKismetMathLibrary::sin(CurrentTime * Settings::Aimbot::ShakeSpeed);

													double MaxPitchShake = 5.0;

													double PitchShake = MaxPitchShake * ShakeFactor;

													TargetRotation.Pitch += PitchShake;
												}

												Custom::AddInput(GWorld, PlayerController, Variables::CameraRotation, TargetRotation, Settings::Aimbot::Interpolate, Settings::Aimbot::Constant, Settings::Aimbot::Speed);
											}
										}

										else if (Settings::Aimbot::AimType == 2)
										{
											Update__GetDamageStartLocation__bSilentAimActive = true;
											Update__GetDamageStartLocation__SilentLocationTarget = TargetWorld;

											/*UpdateGetWeaponTargetingTransformbSilentAimActive = true;
											UpdateGetWeaponTargetingTransformSilentLocationTarget = TargetWorld;*/

											//FVector2D MuzzleScreen = FVector2D();
											//FVector Muzzle = CurrentWeapon->GetMuzzleLocation();
											//if (PlayerController->ProjectWorldLocationToScreen(Muzzle, &MuzzleScreen))
											//{
											//	silent(MuzzleScreen, CurrentWeapon, 0xa68);//Variables::BulletTeleportHead
											//}






											/*if (PlayerController->IsInputKeyDown(AimbotKey))
											{
												FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(Variables::CameraLocation, TargetWorld);

												Custom::Silentaimtest(GWorld, PlayerController, Variables::CameraRotation, TargetRotation, Settings::Aimbot::Constant);
											}*/
										}
										else if (Settings::Aimbot::AimType == 3)
										{

											IsCurrentlySickySilentAiming = true;

											GetPlayerViewPoint_bShouldStartAtLocationTarget = true;
											GetPlayerViewPoint_SilentLocationTarget = TargetWorld;

											if (Settings::Aimbot::StickySilent::Spinbot)
											{
												Update_SpinningRightNow = true;

												PlayerController->AddYawInput((float)Settings::Aimbot::StickySilent::SpinSpeed);
											}
										}
									}
								}
							}

						}
					}
				}
			}
		}

		if (!IsCurrentlySickySilentAiming && SpinningRightNow)
		{
			SpinningRightNow = false;

			Custom::AddInput(GWorld, PlayerController, Variables::CameraRotation, Variables::GetDamageStartLocation::LastTargetRotation, false, false, 0.0);
		}

		SpinningRightNow = Update_SpinningRightNow;


		Variables::GetDamageStartLocation::bSilentAimActive = UpdateGetWeaponTargetingTransformbSilentAimActive;
		Variables::GetDamageStartLocation::SilentLocationTarget = UpdateGetWeaponTargetingTransformSilentLocationTarget;

		Variables::GetPlayerViewPoint::bShouldStartAtLocationTarget = GetPlayerViewPoint_bShouldStartAtLocationTarget;
		Variables::GetPlayerViewPoint::SilentLocationTarget = GetPlayerViewPoint_SilentLocationTarget;


		Variables::GetDamageStartLocation::bSilentAimActive = Update__GetDamageStartLocation__bSilentAimActive;
		Variables::GetDamageStartLocation::SilentLocationTarget = Update__GetDamageStartLocation__SilentLocationTarget;

		Variables::GetTargetingTransform::bPickaxeRangeActive = GetTargetingTransform_bPickaxeRangeActive;
		Variables::GetTargetingTransform::bShouldStartAtLocationTarget = GetTargetingTransform_bShouldStartAtLocationTarget;
		Variables::GetTargetingTransform::LocationTarget = GetTargetingTransform_LocationTarget;

		Variables::GetPlayerViewPoint::bShouldStartAtLocationTarget = GetPlayerViewPoint_bShouldStartAtLocationTarget;
		Variables::GetPlayerViewPoint::SilentLocationTarget = GetPlayerViewPoint_SilentLocationTarget;

		if (Settings::Player::Target)
		{
			FVector2D BarSize = FVector2D(400, 20);
			FVector2D BarPosition = FVector2D(Variables::ScreenCenter.X - (BarSize.X / 2), Variables::ScreenCenter.Y - 270);

			Wrapper::RectFilled(BarPosition, BarSize, FLinearColor(0.04f, 0.04f, 0.04f, 1.f));

			if (SmallestDegreesOwnerWorldLocation && SmallestDegrees < 90)
			{
				double Swapped = 90 - SmallestDegrees;
				double Percentage = (Swapped / 90);

				double WorldDistance = UKismetMathLibrary::Vector_Distance(Variables::CameraLocation, SmallestDegreesOwnerWorldLocation) * 0.01;

				FString ConvertedString = UKismetStringLibrary::BuildString_Int(FString(L"You're being TARGETED: "), FString(L""), int(Percentage * 100), FString(L"%"), true);
				ConvertedString = UKismetStringLibrary::BuildString_Int(ConvertedString, L" (", int(WorldDistance), L"m)");

				Wrapper::Text(ConvertedString, FVector2D(Variables::ScreenCenter.X, Variables::ScreenCenter.Y - 300), FLinearColor(1.f, float(1.f - Percentage), 0.f, 1.f), true, true, false);

				double CalculatedX = BarSize.X * Percentage;

				for (double CurrentX = 0.0; CurrentX < CalculatedX; CurrentX += 1.0)
				{
					double CurrentPercentage = CurrentX / BarSize.X;

					float Red = 1.f * CurrentPercentage;
					float Green = 1.f - Red;

					Wrapper::Line(FVector2D(BarPosition.X + CurrentX, BarPosition.Y), FVector2D(BarPosition.X + CurrentX, BarPosition.Y + BarSize.Y), FLinearColor(Red, Green, 0.f, 1.f), 1.f);
				}
			}

			Wrapper::Rect(BarPosition, BarSize, FLinearColor(0.f, 0.f, 0.f, 1.f), 1.f);
		}
	}

	if (Settings::Misc::CrosshairMode)
	{
		if (Settings::Misc::CrosshairMode == 1)
		{
			Wrapper::Line(FVector2D(Variables::ScreenCenter.X - 10, Variables::ScreenCenter.Y), FVector2D(Variables::ScreenCenter.X + 10, Variables::ScreenCenter.Y), FLinearColor(0.6f, 0.6f, 0.6f, 1.f), 1.f);
			Wrapper::Line(FVector2D(Variables::ScreenCenter.X, Variables::ScreenCenter.Y - 10), FVector2D(Variables::ScreenCenter.X, Variables::ScreenCenter.Y + 10), FLinearColor(0.6f, 0.6f, 0.6f, 1.f), 1.f);
		}
		else if (Settings::Misc::CrosshairMode == 2)
		{
			double crosshair_outer = double(14);
			double crosshair_inner = double(8);

			Wrapper::Line(FVector2D(Variables::ScreenCenter.X - crosshair_outer, Variables::ScreenCenter.Y), FVector2D(Variables::ScreenCenter.X - crosshair_inner, Variables::ScreenCenter.Y), FLinearColor(0.6f, 0.6f, 0.6f, 1.f), 2.f);
			Wrapper::Line(FVector2D(Variables::ScreenCenter.X + crosshair_inner, Variables::ScreenCenter.Y), FVector2D(Variables::ScreenCenter.X + crosshair_outer, Variables::ScreenCenter.Y), FLinearColor(0.6f, 0.6f, 0.6f, 1.f), 2.f);
			Wrapper::Line(FVector2D(Variables::ScreenCenter.X, Variables::ScreenCenter.Y - crosshair_outer), FVector2D(Variables::ScreenCenter.X, Variables::ScreenCenter.Y - crosshair_inner), FLinearColor(0.6f, 0.6f, 0.6f, 1.f), 2.f);
			Wrapper::Line(FVector2D(Variables::ScreenCenter.X, Variables::ScreenCenter.Y + crosshair_inner), FVector2D(Variables::ScreenCenter.X, Variables::ScreenCenter.Y + crosshair_outer), FLinearColor(0.6f, 0.6f, 0.6f, 1.f), 2.f);
		}
		else if (Settings::Misc::CrosshairMode == 3)
		{
			Wrapper::Circle(Variables::ScreenCenter, FLinearColor(0.6f, 0.6f, 0.6f, 1.f), 5.0, 38.0, true);
		}
		else if (Settings::Misc::CrosshairMode == 4)
		{
			Wrapper::Line(FVector2D(Variables::ScreenCenter.X - 10, Variables::ScreenCenter.Y - 10), FVector2D(Variables::ScreenCenter.X + 10, Variables::ScreenCenter.Y + 10), FLinearColor(0.6f, 0.6f, 0.6f, 1.f), 1.f);
			Wrapper::Line(FVector2D(Variables::ScreenCenter.X - 10, Variables::ScreenCenter.Y + 10), FVector2D(Variables::ScreenCenter.X + 10, Variables::ScreenCenter.Y - 10), FLinearColor(0.6f, 0.6f, 0.6f, 1.f), 1.f);

			Wrapper::Line(FVector2D(Variables::ScreenCenter.X - 10, Variables::ScreenCenter.Y - 10), FVector2D(Variables::ScreenCenter.X - 5, Variables::ScreenCenter.Y - 5), FLinearColor(0.f, 0.f, 0.f, 1.f), 2.f);
			Wrapper::Line(FVector2D(Variables::ScreenCenter.X - 10, Variables::ScreenCenter.Y + 10), FVector2D(Variables::ScreenCenter.X - 5, Variables::ScreenCenter.Y + 5), FLinearColor(0.f, 0.f, 0.f, 1.f), 2.f);
			Wrapper::Line(FVector2D(Variables::ScreenCenter.X + 10, Variables::ScreenCenter.Y + 10), FVector2D(Variables::ScreenCenter.X + 5, Variables::ScreenCenter.Y + 5), FLinearColor(0.f, 0.f, 0.f, 1.f), 2.f);
			Wrapper::Line(FVector2D(Variables::ScreenCenter.X + 10, Variables::ScreenCenter.Y - 10), FVector2D(Variables::ScreenCenter.X + 5, Variables::ScreenCenter.Y - 5), FLinearColor(0.f, 0.f, 0.f, 1.f), 2.f);
		}
	}

	if (Variables::PlayerController->WasInputKeyJustPressed(Keys::Insert)) Variables::DrawMenu = !Variables::DrawMenu;
	if (Variables::DrawMenu) Framework::DrawMenu();
	Wrapper::Render();
	Wrapper::ClearTargets();

	return DrawTitleSafeAreaOriginal(Viewport, Canvas);
}