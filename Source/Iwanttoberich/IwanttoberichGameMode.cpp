// Copyright Epic Games, Inc. All Rights Reserved.

#include "IwanttoberichGameMode.h"
#include "IwanttoberichCharacter.h"
#include "UObject/ConstructorHelpers.h"

AIwanttoberichGameMode::AIwanttoberichGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
