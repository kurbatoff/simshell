--
--
--
local theRule = "2205957841BE9CE168EFD9F881D1B4807346FE15"


local function hex2array(str)
  local arr = {}

  str = string.gsub(str, "%s+", "")

  for i = 1, #str, 2 do
    arr[#arr+1] = tonumber(str:sub(i, i+1), 16)
  end
  return arr
end


print "ARA managed by Lua"

--[[
print "Select ARA"

local apdu = {0, 0xA4, 4, 0, 0}
local ara = {9, 0xA0, 0, 0, 0x01, 0x51, 0x41, 0x43, 0x4c, 0}

table.move(ara, 1, #ara, #apdu + 1, apdu)

--print(table.concat(apdu, ', '))

--resp = C_send_apdu(apdu)
--print "Authenticate to ARA"
]]


--apdu = hex2array("80E2 9000 26 F0 24 E2 22 E1 18 4F00 C1 14 2205957841BE9CE168EFD9F881D1B4807346FE15 E306D00101D10101")


--                          V     V     V     V          V
apdu = hex2array("80E2 9000 26 F0 24 E2 22 E1 18 4F00 C1 14 2205957841BE9CE168EFD9F881D1B4807346FE15 E306D00101D10101")


resp = C_send_apdu(apdu)

print "Ok."