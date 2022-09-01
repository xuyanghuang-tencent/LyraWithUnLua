---@type W_LyraGameMenu_C
local M = UnLua.Class()

function M:Construct()
    self.OptionsButtons.OnButtonBaseClicked:Add(self, self.OptionsButtons_OnClick)
    self.QuitButton_EditorOnly.OnButtonBaseClicked:Add(self, self.QuitButton_EditorOnly_OnClick)
    self.ReturnButton.OnButtonBaseClicked:Add(self, self.ReturnButton_OnClick)
end

function M:BP_OnActivated()
    self:PlayAnimationForward(self.OnActivated)
end

function M:OptionsButtons_OnClick()
    local Player = self:GetOwningPlayer()
    local WBP = UE.FSoftObjectPtr(UE.UClass.Load("/Game/UI/Settings/W_LyraSettingScreen.W_LyraSettingScreen_C"))
    local Tag = UE.FGameplayTag()
    Tag.TagName = "UI.Layer.Menu"
    local AsyncAction = UE.UAsyncAction_PushContentToLayerForPlayer.PushContentToLayerForPlayer(Player, WBP, Tag, true)
    AsyncAction.BeforePush:Add(self, function()
        self:DeactivateWidget()
    end)
    AsyncAction:Activate()
end

function M:QuitButton_EditorOnly_OnClick()
    UE.UKismetSystemLibrary.QuitGame(self, nil, UE.EQuitPreference.Quit, false)
end

function M:ReturnButton_OnClick()
    local AsyncAction = UE.UAsyncAction_ShowConfirmation.ShowConfirmationYesNo(self, "Exit Game", "Are you sure?")
    AsyncAction.OnResult:Add(self, function(_, Result)
        if Result == UE.ECommonMessagingResult.Confirmed then
            local Options = "Experience=" .. self.FrontEndExperienceReference.PrimaryAssetName
            UE.UGameplayStatics.OpenLevelBySoftObjectPtr(self, self.FrontEndMapReference, true, Options)
        end
    end)
    AsyncAction:Activate()
end

return M
