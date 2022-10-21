---@type W_ActionTouchButton_C
local M = UnLua.Class()

function M:Construct()
    self:UpdateCoolDownPercent(1.0)
    if self.HasCooldown then
        self:PlayAnimationForward(self.CooldownInactiveToActive)
    else
        self:ResetMaterials()
    end
end

function M:PreConstruct()
    self:SetButtonStyle()
    self:ResetMaterials()
end

function M:OnInitialized()
    if UE.UBlueprintGameplayTagLibrary.IsGameplayTagValid(self.DurationMessageTag) then
        self:ListenForDuration()
    end
end

function M:ListenForDuration()
    local AsyncAction = UE.UAsyncAction_ListenForGameplayMessage.ListenForGameplayMessages(self, self.DurationMessageTag, UE.FLyraInteractionDurationMessage)
    AsyncAction.OnMessageReceived:Add(self, function(_, ProxyObject, ActualChannel)
        local Payload = ProxyObject:GetPayload()
        self.Duration = Payload.Duration
        self.StartTime = UE.UGameplayStatics.GetTimeSeconds(self)

        if self:IsAnimationPlaying(self.CooldownInactiveToActive) then
            self:StopAnimation(self.CooldownInactiveToActive)
        end

        self:UpdateCoolDownPercent(0)
        self:PlayAnimationForward(self.CooldownInactiveToActive)
        self.Charging = true
    end)
    AsyncAction:Activate()
end

function M:BP_OnClicked()
    ---@type UEnhancedInputLocalPlayerSubsystem
    local EnhancedInputLocalPlayerSubsystem = UE.USubsystemBlueprintLibrary.GetLocalPlayerSubsystem(self, UE.UEnhancedInputLocalPlayerSubsystem)
    EnhancedInputLocalPlayerSubsystem:InjectInputVectorForAction(self.AssociatedAction, UE.FVector(1, 0, 0), {}, {})
    if self.HasCooldown or self.IsToggleButton then
        return
    end

    if self:IsAnimationPlayingForward(self.OnClickedTouchInput) then
        self:StopAnimation(self.OnClickedTouchInput)
    end
    self:PlayAnimationForward(self.OnClickedTouchInput)
end

return M
