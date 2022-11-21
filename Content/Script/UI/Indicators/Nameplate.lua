---@type W_Nameplate_C
local M = UnLua.Class()

function M:PawnReady()
    local Name = self.Pawn.PlayerState:GetPlayerName()
    self:InitializeNameplate(self.Pawn, Name)
end

function M:InitializeNameplate(Pawn, Name)
    self.Pawn = Pawn
    self.NameText = Name
    self.Indicator:SetDesiredVisibility(true)
    self:SetVisibility(UE.ESlateVisibility.Visible)

    -- Player Name
    self:SetNameText(self.NameText)

    -- Team Colors
    local AsyncAction = UE.UAsyncAction_ObserveTeam.ObserveTeam(self.Pawn)
    AsyncAction.OnTeamChanged:Add(self, function(_, bTeamSet, TeamId)
        if not bTeamSet then
            return
        end
        self.TeamId = TeamId
        self:SetTeamVisuals()
    end)
    AsyncAction:Activate()

    -- Health
    local HealthComponent = UE.ULyraHealthComponent.FindHealthComponent(self.Pawn)
    if HealthComponent then
        HealthComponent.OnHealthChanged:Add(self, self["Health Changed"])
        HealthComponent.OnDeathStarted:Add(self, self.DeathStarted)
        self:SetHealthBarPercent(HealthComponent:GetHealthNormalized())
    end

    self:UpdateVisibility()
end

M["Health Changed"] = function(self, HealthComponent, OldValue, NewValue, Instigator)
    local HealthPercent = HealthComponent:GetHealthNormalized()
    self:SetHealthBarPercent(HealthPercent)
end

function M:SetHealthBarPercent(HealthPercent)
    self.Health:GetDynamicMaterial():SetScalarParameterValue("Progress", HealthPercent)
end

function M:DeathStarted(OwningActor)
    self.Indicator:UnregisterIndicator()
end

function M:SetNameText(PlayerName)
    self.Name:SetText(PlayerName)
end

function M:BindIndicator(Indicator)
    self.Indicator = Indicator
    self.Pawn = Indicator:GetDataObject():Cast(UE.APawn)
    Indicator:SetDesiredVisibility(false)
    local PawnExtensionComponent = UE.ULyraPawnExtensionComponent.FindPawnExtensionComponent(self.Pawn)
    if PawnExtensionComponent:IsPawnReadyToInitialize() then
        self:PawnReady()
    else
        PawnExtensionComponent.BP_OnPawnReadyToInitialize:Add(self, self.PawnReady)
    end
end

function M:UnbindIndicator(Indicator)
    local HealthComponent = UE.ULyraHealthComponent.FindHealthComponent(self.Pawn)
    if HealthComponent then
        HealthComponent.OnHealthChanged:Remove(self, self["Health Changed"])
        HealthComponent.OnDeathStarted:Remove(self, self.DeathStarted)
    end

    local PawnExtensionComponent = UE.ULyraPawnExtensionComponent.FindPawnExtensionComponent(self.Pawn)
    PawnExtensionComponent.BP_OnPawnReadyToInitialize:Remove(self, self.PawnReady)
    self.Pawn = nil
    self.Indicator = nil
end

function M:UpdateVisibility()
    -- Ignore pawns controlled by us.
    if self.HideOwnerName and self.Pawn:GetController() == self:GetOwningPlayer() then
        self:SetVisibility(UE.ESlateVisibility.Collapsed)
        return
    end

    -- Ignore Different Teams
    if self.ShowTeamNamesOnly then
        ---@type ULyraTeamSubsystem
        local LyraTeamSubsystem = UE.USubsystemBlueprintLibrary.GetWorldSubsystem(self, UE.ULyraTeamSubsystem)
        if LyraTeamSubsystem:CompareTeams(self.Pawn, self:GetOwningPlayer()) then
            return
        end
        self:SetVisibility(UE.ESlateVisibility.Collapsed)
    end
end

return M
