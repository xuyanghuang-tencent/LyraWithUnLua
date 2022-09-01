#include "LyraLuaModuleLocator.h"

FString ULyraLuaModuleLocator::Locate(const UObject* Object)
{
	const auto Class = Object->IsA<UClass>() ? static_cast<const UClass*>(Object) : Object->GetClass();
	const auto Key = Class->GetFName();

#if !WITH_EDITOR
		if (const auto Cached = Cache.Find(Key))
			return *Cached;
#endif

	FString ModuleName;
	if (Class->IsNative())
	{
		// C++类型直接用类名做Lua绑定
		ModuleName = Class->GetName();
	}
	else
	{
		// 蓝图类使用资源路径，剔除掉/Game前缀，以及资源名的W_和B_前缀
		ModuleName = Class->GetPackage()->GetName();
		const auto ChopCount = ModuleName.Find(TEXT("/"), ESearchCase::IgnoreCase, ESearchDir::FromStart, 1) + 1;
		ModuleName = ModuleName.Replace(TEXT("/"), TEXT(".")).RightChop(ChopCount);

		TArray<FString> Parts;
		ModuleName.ParseIntoArray(Parts, TEXT("."));
		if (Parts.Num() > 0)
		{
			auto& LastPart = Parts[Parts.Num() - 1];
			static const auto PrefixToRemove = {"W_", "B_", "BP_"};
			for (const auto ToRemove : PrefixToRemove)
			{
				if (LastPart.StartsWith(ToRemove))
				{
					LastPart.RightChopInline(strlen(ToRemove));
					break;
				}
			}
			ModuleName = FString::Join(Parts, TEXT("."));
		}
	}
	Cache.Add(Key, ModuleName);
	return ModuleName;
}
