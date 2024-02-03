--
--
--

print "ARA update reresh tag"

--/send 80E2900002F200

local refresh = {0x80, 0xE2, 0x90, 0x00, 2, 0xF2, 0x00}
resp = C_send_apdu(refresh)


print "ARA read all rulles"

--/send 80CAFF4000
--/send 00c0000000

local readall = {0x80, 0xCA, 0xFF, 0x40, 0}
resp = C_send_apdu(readall)

local apdu = {0, 0xC0, 0, 0 , resp[2]}
resp = C_send_apdu(apdu)
