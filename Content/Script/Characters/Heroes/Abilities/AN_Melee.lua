---@type AN_Melee_C
local M = UnLua.Class()

---@param MeshComp USkeletalMeshComponent
---@param Animation UAnimSequenceBase
---@param EventReference FAnimNotifyEventReference
---@return boolean
function M:Received_Notify(MeshComp, Animation, EventReference)
    local Actor = MeshComp:GetOwner()
    local EventTag = UE.FGameplayTag()
    EventTag.TagName = "GameplayEvent.MeleeHit"
    local Data = UE.FGameplayEventData()
    UE.UAbilitySystemBlueprintLibrary.SendGameplayEventToActor(Actor, EventTag, Data)
    return false
end

return M