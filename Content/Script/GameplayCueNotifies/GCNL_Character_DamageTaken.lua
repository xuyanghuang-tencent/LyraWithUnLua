---@type GCNL_Character_DamageTaken_C
local M = UnLua.Class()

---@param Character ACharacter
---@param Montage UAnimMontage
function M:MontageHit_RPC(Character, Montage)
    Character:PlayAnimMontage(Montage, 1.0, nil)
end

return M
