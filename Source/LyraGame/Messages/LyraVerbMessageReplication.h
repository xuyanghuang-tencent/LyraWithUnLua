// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "LyraVerbMessage.h"
#include "LyraVerbMessageReplication.generated.h"

struct FLyraVerbMessageReplication;

/**
 * Represents one verb message
 */
USTRUCT(BlueprintType)
struct FLyraVerbMessageReplicationEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FLyraVerbMessageReplicationEntry()
	{}

	FLyraVerbMessageReplicationEntry(const FLyraVerbMessage& InMessage)
		: Message(InMessage)
	{
	}

	FString GetDebugString() const;

private:
	friend FLyraVerbMessageReplication;

	UPROPERTY()
	FLyraVerbMessage Message;
};

/** Container of verb messages to replicate */
USTRUCT(BlueprintType)
struct FLyraVerbMessageReplication : public FFastArraySerializer
{
	GENERATED_BODY()

	FLyraVerbMessageReplication()
	{
	}

public:
	void SetOwner(UObject* InOwner) { Owner = InOwner; }

	// Broadcasts a message from server to clients
	void AddMessage(const FLyraVerbMessage& Message);

	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FLyraVerbMessageReplicationEntry, FLyraVerbMessageReplication>(CurrentMessages, DeltaParms, *this);
	}

private:
	void RebroadcastMessage(const FLyraVerbMessage& Message);

private:
	// Replicated list of gameplay tag stacks
	UPROPERTY()
	TArray<FLyraVerbMessageReplicationEntry> CurrentMessages;
	
	// Owner (for a route to a world)
	UPROPERTY()
	UObject* Owner = nullptr;
};

template<>
struct TStructOpsTypeTraits<FLyraVerbMessageReplication> : public TStructOpsTypeTraitsBase2<FLyraVerbMessageReplication>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
