---@type GA_Hero_Death_C
local M = UnLua.Class()

---@param EventData FGameplayEventData
function M:K2_ActivateAbilityFromEvent(EventData)
    self:SetCameraMode(self.CameraMode)

    local Tag = UE.FGameplayTag()
    Tag.TagName = "GameplayCue.Character.Death"

    local Params = UE.FGameplayCueParameters()
    Params.NormalizedMagnitude = EventData.EventMagnitude
    Params.RawMagnitude = EventData.EventMagnitude
    Params.EffectContext = EventData.ContextHandle
    Params.MatchedTagName = EventData.EventTag
    Params.OriginalTag = EventData.EventTag
    Params.AggregatedSourceTags = EventData.InstigatorTags
    Params.AggregatedTargetTags = EventData.TargetTags
    Params.Instigator = EventData.Instigator
    Params.SourceObject = EventData.Target

    self:K2_ExecuteGameplayCueWithParams(Tag, Params)

    local Task = UE.UAbilityTask_WaitDelay.WaitDelay(self, self.Duration)
    Task.OnFinish:Add(self, function()
        self:K2_EndAbility()
    end)
    Task:ReadyForActivation()
end

return M
