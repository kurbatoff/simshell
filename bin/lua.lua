--
--
--

print "Welcome to Lua world!"

C_connect_PCSC()
C_select_ISD()

local resp = {}

local apdu = {0, 0xA4, 4, 0 , 8, 0xA0, 0, 0, 1, 0x51, 0, 0, 0}
resp = C_send_apdu(apdu)
print(table.concat(resp, ', '))


local apdu = {0, 0xC0, 0, 0 , resp[2]}
resp = C_send_apdu(apdu)
--printf(sw)
print(table.concat(resp, ', '))

print "Congratulations!"

--
--
