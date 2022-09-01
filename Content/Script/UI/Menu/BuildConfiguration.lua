---@type W_BuildConfiguration_C
local M = UnLua.Class()

function M:Construct()
    local BuildVer = UE.UKismetSystemLibrary.GetBuildVersion() .. "lua"
    local BuildConfig = UE.UKismetSystemLibrary.GetBuildConfiguration()
    local Text = string.format("%s (%s)", BuildVer, BuildConfig)
    self.BuildVersionTextWidget:SetText(Text)
end

return M
