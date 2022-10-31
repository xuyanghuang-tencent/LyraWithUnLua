---@type GA_Weapon_Fire_C
local M = UnLua.Class()

function M:K2_ActivateAbility()
    if self:IsLocallyControlled() then
        self:StartRangedWeaponTargeting()
    end

    local Proxy = UE.UAbilityTask_PlayMontageAndWait.CreatePlayMontageAndWaitProxy(self, nil, self.CharacterFireMontage, self.AutoRate, nil, false, 1.0, 0)
    local Handler = function()
        self:K2_EndAbility()
    end
    Proxy.OnCompleted:Add(self, Handler)
    Proxy.OnInterrupted:Add(self, Handler)
    Proxy.OnCancelled:Add(self, Handler)
    Proxy:ReadyForActivation()

    UE.UKismetSystemLibrary.K2_SetTimerDelegate({ self, self.FireComplete }, self.FireDelayTimeSecs, false, 0, 0)
end

function M:FireComplete()
    self:K2_EndAbility()
end

return M
