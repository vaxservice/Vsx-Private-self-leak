#pragma once

void(*FireSingleOriginal)(AFortWeapon*);
void FireSingle(AFortWeapon* FortWeapon)
{
	if (Settings::Misc::BulletTraces)
	{
		FVector CameraForward = UKismetMathLibrary::GetForwardVector(Variables::CameraRotation);

		FVector Muzzle = FortWeapon->GetMuzzleLocation();
		FVector MaxLocation = Variables::CameraLocation + (CameraForward * 20000.0); //200m

		FVector BulletImpact = MaxLocation;
		FHitResult OutHit;
		TArray<AActor*> ActorsToIgnore;

		if (UWorld* GWorld = UWorld::GetWorld())
		{
			if (UKismetSystemLibrary::LineTraceSingle(GWorld, Muzzle, MaxLocation, ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore, EDrawDebugTrace::ForDuration, OutHit, false))
			{
				BulletImpact = OutHit.ImpactPoint;
			}

			if (Variables::LastBulletIndexPushed > 4)
			{
				Variables::LastBulletIndexPushed = 0;
			}

			Variables::Last5BulletImpacts[Variables::LastBulletIndexPushed] = BulletImpact;
			Variables::Last5BulletImpactsMuzzle[Variables::LastBulletIndexPushed] = Muzzle;

			Variables::LastBulletIndexPushed += 1;
		}
	}

	reinterpret_cast<void* (__cdecl*)(AFortWeapon*, uintptr_t, void*)>(DoSpoofCall)(FortWeapon, SpoofCode, FireSingleOriginal);
}