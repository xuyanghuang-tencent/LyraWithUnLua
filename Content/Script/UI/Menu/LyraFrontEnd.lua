---@type W_LyraFrontEnd_C
local M = UnLua.Class()

function M:Construct()
    self.StartGameButton.OnButtonBaseClicked:Add(self, self.StartGameButton_OnClicked)
    self.OptionsButton.OnButtonBaseClicked:Add(self, self.OptionsButton_OnClicked)
    self.CreditsButton.OnButtonBaseClicked:Add(self, self.CreditsButton_OnClicked)
    self.QuitGameButton.OnButtonBaseClicked:Add(self, self.QuitGameButton_OnClicked)
end

---@param self W_LyraFrontEnd_C
local function PushContentToLayerForPlayer(self, ClassPath)
    local Player = self:GetOwningPlayer()
    local WBP = UE.FSoftObjectPtr(UE.UClass.Load(ClassPath))
    local Tag = UE.FGameplayTag()
    Tag.TagName = "UI.Layer.Menu"
    UE.UAsyncAction_PushContentToLayerForPlayer.PushContentToLayerForPlayer(Player, WBP, Tag, true):Activate()
end

function M:StartGameButton_OnClicked()
    PushContentToLayerForPlayer(self, "/Game/UI/Menu/Experiences/W_ExperienceSelectionScreen.W_ExperienceSelectionScreen.W_ExperienceSelectionScreen_C")
end

function M:OptionsButton_OnClicked()
    PushContentToLayerForPlayer(self, "/Game/UI/Settings/W_LyraSettingScreen.W_LyraSettingScreen_C")
end

function M:CreditsButton_OnClicked()
    PushContentToLayerForPlayer(self, "/Game/UI/Credits/W_Credits.W_Credits_C")
end

function M:QuitGameButton_OnClicked()
    local AsyncAction = UE.UAsyncAction_ShowConfirmation.ShowConfirmationYesNo(self, "Exit Game", "Are you sure1111?")
    AsyncAction.OnResult:Add(self, function(_, Result)
        if Result == UE.ECommonMessagingResult.Confirmed then
            UE.UKismetSystemLibrary.QuitGame(self, nil, UE.EQuitPreference.Quit, false)
        end
    end)
    AsyncAction:Activate()
end

return M
