#include "GameFramework/AsyncAction_ListenForGameplayMessage.h"

#include "LowLevel.h"
#include "UnLuaEx.h"

struct UAsyncAction_ListenForGameplayMessage_PrivateAccess : UCancellableAsyncAction
{
	FAsyncGameplayMessageDelegate OnMessageReceived;

	const void* ReceivedMessagePayloadPtr = nullptr;

	TWeakObjectPtr<UWorld> WorldPtr;
	FGameplayTag ChannelToRegister;
	TWeakObjectPtr<UScriptStruct> MessageStructType = nullptr;
	EGameplayMessageMatch MessageMatchType = EGameplayMessageMatch::ExactMatch;

	FGameplayMessageListenerHandle ListenerHandle;
};

static int UAsyncAction_ListenForGameplayMessageLib_GetPayload(lua_State* L)
{
	const auto Top = lua_gettop(L);
	if (Top < 1)
		return luaL_error(L, "invalid parameters");

	auto ProxyObject = (UAsyncAction_ListenForGameplayMessage*)UnLua::GetUObject(L, 1);
	if (!ProxyObject)
		return luaL_error(L, "invalid parameters");

	const auto PrivateAccess = (UAsyncAction_ListenForGameplayMessage_PrivateAccess*)ProxyObject;
	const auto Struct = PrivateAccess->MessageStructType.Get();
	if (!Struct)
		return 0;

	FString MetatableName;
	if (Struct->IsNative())
		MetatableName = FString::Printf(TEXT("%s%s"), Struct->GetPrefixCPP(), *Struct->GetName());
	else
		MetatableName = Struct->GetPathName();

	UnLua::PushPointer(L, (void*)PrivateAccess->ReceivedMessagePayloadPtr, TCHAR_TO_UTF8(*MetatableName), true);
	return 1;
}

luaL_Reg UAsyncAction_ListenForGameplayMessageLib[] = {
	{"GetPayload", UAsyncAction_ListenForGameplayMessageLib_GetPayload},
	{nullptr, nullptr}
};

BEGIN_EXPORT_REFLECTED_CLASS(UAsyncAction_ListenForGameplayMessage)
	ADD_LIB(UAsyncAction_ListenForGameplayMessageLib)
END_EXPORT_CLASS()

IMPLEMENT_EXPORTED_CLASS(UAsyncAction_ListenForGameplayMessage)
