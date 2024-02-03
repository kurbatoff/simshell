--
--
--

print "ARA update reresh tag"

--/send 80E2900002F200



print "ARA delete all"

local deleteall = {0x80, 0xE2, 0x90, 0x00, 2, 0xF1, 0x00}
resp = C_send_apdu(deleteall)



print "ARA update reresh tag"

local refresh = {0x80, 0xE2, 0x90, 0x00, 2, 0xF2, 0x00}
resp = C_send_apdu(refresh)
