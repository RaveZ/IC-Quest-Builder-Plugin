// Copyright 2024 Ivan Chandra. All Rights Reserved.

#include "Quest_System_Runtime.h"


IMPLEMENT_MODULE(FQuest_System_RuntimeModule, Quest_System_Runtime)

void FQuest_System_RuntimeModule::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
}


void FQuest_System_RuntimeModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}