// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System;
using System.IO;
using EpicGames.Core;
using System.Collections.Generic;
using UnrealBuildBase;

public class LyraGameTarget : TargetRules
{
	public LyraGameTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;

		ExtraModuleNames.AddRange(new string[] { "LyraGame" });

		LyraGameTarget.ApplySharedLyraTargetSettings(this);
	}

	private static bool bHasWarnedAboutShared = false;

	internal static void ApplySharedLyraTargetSettings(TargetRules Target)
    {
		bool bIsTest = Target.Configuration == UnrealTargetConfiguration.Test;
		bool bIsShipping = Target.Configuration == UnrealTargetConfiguration.Shipping;
		bool bIsDedicatedServer = Target.Type == TargetType.Server;
		if (Target.BuildEnvironment == TargetBuildEnvironment.Unique)
		{
			Target.ShadowVariableWarningLevel = WarningLevel.Error;

			Target.bUseLoggingInShipping = true;

			if (bIsShipping && !bIsDedicatedServer)
			{
				// Make sure that we validate certificates for HTTPS traffic
				Target.bDisableUnverifiedCertificates = true;

				// Uncomment these lines to lock down the command line processing
				// This will only allow the specified command line arguments to be parsed
				//Target.GlobalDefinitions.Add("UE_COMMAND_LINE_USES_ALLOW_LIST=1");
				//Target.GlobalDefinitions.Add("UE_OVERRIDE_COMMAND_LINE_ALLOW_LIST=\"-space -separated -list -of -commands\"");

				// Uncomment this line to filter out sensitive command line arguments that you
				// don't want to go into the log file (e.g., if you were uploading logs)
				//Target.GlobalDefinitions.Add("FILTER_COMMANDLINE_LOGGING=\"-some_connection_id -some_other_arg\"");
			}

			if (bIsShipping || bIsTest)
			{
				// Disable reading generated/non-ufs ini files
				Target.bAllowGeneratedIniWhenCooked = false;
				Target.bAllowNonUFSIniWhenCooked = false;
			}

			if (Target.Type != TargetType.Editor)
			{
				// We don't use the path tracer at runtime, only for beauty shots, and this DLL is quite large
				Target.DisablePlugins.Add("OpenImageDenoise");
			}

			LyraGameTarget.ConfigureGameFeaturePlugins(Target);
		}
		else
        {
			// !!!!!!!!!!!! WARNING !!!!!!!!!!!!!
			// Any changes in here must not affect PCH generation, or the target
			// needs to be set to TargetBuildEnvironment.Unique

			// This only works in editor or Unique build environments
			if (Target.Type == TargetType.Editor)
			{
				LyraGameTarget.ConfigureGameFeaturePlugins(Target);
			}
			else
			{
				// Shared monolithic builds cannot enable/disable plugins or change any options because it tries to re-use the installed engine binaries
				if (!bHasWarnedAboutShared)
				{
					bHasWarnedAboutShared = true;
					Log.TraceWarning("LyraGameEOS and dynamic target options are disabled when packaging from an installed version of the engine");
				}
			}
		}		
	}

	static public bool ShouldEnableAllGameFeaturePlugins(TargetRules Target)
	{
		// Editor builds will build all game feature plugins, but it may or may not load them all.
		// This is so you can enable plugins in the editor without needing to compile code.
		if (Target.Type == TargetType.Editor)
		{
			return true;
		}

		// We always return true here because the example ConfigureGameFeaturePlugins()
		// doesn't have any additional logic to pick and choose which plugins to use.
		// If you add more complicated logic, you will want to return false here instead
		return true;
	}

	// Configures which game feature plugins we want to have enabled
	// This is a fairly simple implementation, but you might do things like build different
	// plugins based on the target release version of the current branch, e.g., enabling 
	// work-in-progress features in main but disabling them in the current release branch.
	static public void ConfigureGameFeaturePlugins(TargetRules Target)
	{
		Log.TraceInformationOnce("Compiling GameFeaturePlugins in branch {0}", Target.Version.BranchName);

		bool bBuildAllGameFeaturePlugins = ShouldEnableAllGameFeaturePlugins(Target);

		// Load all of the game feature .uplugin descriptors
		List<FileReference> CombinedPluginList = new List<FileReference>();

		List<DirectoryReference> GameFeaturePluginRoots = Unreal.GetExtensionDirs(Target.ProjectFile.Directory, Path.Combine("Plugins", "GameFeatures"));
		foreach (DirectoryReference SearchDir in GameFeaturePluginRoots)
		{
			CombinedPluginList.AddRange(PluginsBase.EnumeratePlugins(SearchDir));
		}

		if (CombinedPluginList.Count > 0)
		{
			Dictionary<string, JsonObject> AllPluginRootJsonObjectsByName = new Dictionary<string, JsonObject>();
			Dictionary<string, List<string>> AllPluginReferencesByName = new Dictionary<string, List<string>>();

			foreach (FileReference PluginFile in CombinedPluginList)
			{
				if (PluginFile != null && FileReference.Exists(PluginFile))
				{
					bool bEnabled = false;
					try
					{
						JsonObject RawObject = JsonObject.Read(PluginFile);
						AllPluginRootJsonObjectsByName.Add(PluginFile.GetFileNameWithoutExtension(), RawObject);

						// Validate that all GameFeaturePlugins are disabled by default
						bool bEnabledByDefault = false;
						if (!RawObject.TryGetBoolField("EnabledByDefault", out bEnabledByDefault) || bEnabledByDefault == true)
						{
							Log.TraceWarning("GameFeaturePlugin {0}, does not set EnabledByDefault to false. This is required for built-in GameFeaturePlugins.", PluginFile.GetFileNameWithoutExtension());
						}

						// Validate that all GameFeaturePlugins are set to explicitly loaded
						bool bExplicitlyLoaded = false;
						if (!RawObject.TryGetBoolField("ExplicitlyLoaded", out bExplicitlyLoaded) || bExplicitlyLoaded == false)
						{
							Log.TraceWarning("GameFeaturePlugin {0}, does not set ExplicitlyLoaded to true. This is required for GameFeaturePlugins.", PluginFile.GetFileNameWithoutExtension());
						}

						// You could read an additional field here that is project specific, e.g.,
						//string PluginReleaseVersion;
						//if (RawObject.TryGetStringField("MyProjectReleaseVersion", out PluginReleaseVersion))
						//{
						//		bEnabled = SomeFunctionOf(PluginReleaseVersion, CurrentReleaseVersion) || bBuildAllGameFeaturePlugins;
						//}

						if (bBuildAllGameFeaturePlugins)
						{
							// We are in a mode where we want all game feature plugins, except ones we can't load or compile
							bEnabled = true;
						}

						// Prevent using editor-only feature plugins in non-editor builds
						bool bEditorOnly = false;
						if (RawObject.TryGetBoolField("EditorOnly", out bEditorOnly))
						{
							if (bEditorOnly && (Target.Type != TargetType.Editor) && !bBuildAllGameFeaturePlugins)
							{
								// The plugin is editor only and we are building a non-editor target, so it is disabled
								bEnabled = false;
							}
						}
						else
						{
							// EditorOnly is optional
						}

						// some plugins should only be available in certain branches
						string RestrictToBranch;
						if (bEnabled && RawObject.TryGetStringField("RestrictToBranch", out RestrictToBranch))
						{
							if (!Target.Version.BranchName.Equals(RestrictToBranch, StringComparison.OrdinalIgnoreCase))
							{
								// The plugin is for a specific branch, and this isn't it
								bEnabled = false;
								Log.TraceVerbose("GameFeaturePlugin {0} was marked as restricted to other branches. Disabling.", PluginFile.GetFileNameWithoutExtension());
							}
							else
							{
								Log.TraceVerbose("GameFeaturePlugin {0} was marked as restricted to this branch. Leaving enabled.", PluginFile.GetFileNameWithoutExtension());
							}
						}

						// Plugins can be marked as NeverBuild which overrides the above
						bool bNeverBuild = false;
						if (bEnabled && RawObject.TryGetBoolField("NeverBuild", out bNeverBuild) && bNeverBuild)
						{
							// This plugin was marked to never compile, so don't
							bEnabled = false;
							Log.TraceVerbose("GameFeaturePlugin {0} was marked as NeverBuild, disabling.", PluginFile.GetFileNameWithoutExtension());
						}

						// Keep track of plugin references for validation later
						JsonObject[] PluginReferencesArray;
						if (RawObject.TryGetObjectArrayField("Plugins", out PluginReferencesArray))
						{
							foreach (JsonObject ReferenceObject in PluginReferencesArray)
							{
								bool bRefEnabled = false;
								if (ReferenceObject.TryGetBoolField("Enabled", out bRefEnabled) && bRefEnabled == true)
								{
									string PluginReferenceName;
									if (ReferenceObject.TryGetStringField("Name", out PluginReferenceName))
									{
										string ReferencerName = PluginFile.GetFileNameWithoutExtension();
										if (!AllPluginReferencesByName.ContainsKey(ReferencerName))
										{
											AllPluginReferencesByName[ReferencerName] = new List<string>();
										}
										AllPluginReferencesByName[ReferencerName].Add(PluginReferenceName);
									}
								}
							}
						}
					}
					catch (JsonParseException ParseException)
					{
						Log.TraceWarning("Failed to parse GameFeaturePlugin file {0}, disabling. Exception: {1}", PluginFile.GetFileNameWithoutExtension(), ParseException.Message);
						bEnabled = false;
					}

					// Print out the final decision for this plugin
					Log.TraceVerbose("ConfigureGameFeaturePlugins() has decided to {0} feature {1}", bEnabled ? "enable" : "disable", PluginFile.GetFileNameWithoutExtension());

					// Enable or disable it
					if (bEnabled)
					{
						Target.EnablePlugins.Add(PluginFile.GetFileNameWithoutExtension());
					}
					else
					{
						Target.DisablePlugins.Add(PluginFile.GetFileNameWithoutExtension());
					}
				}
			}

			// If you use something like a release version, consider doing a reference validation to make sure
			// that plugins with sooner release versions don't depend on content with later release versions
		}
	}
}
