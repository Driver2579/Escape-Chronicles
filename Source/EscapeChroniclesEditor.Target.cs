// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class EscapeChroniclesEditorTarget : TargetRules
{
	public EscapeChroniclesEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("EscapeChronicles");
	}
}