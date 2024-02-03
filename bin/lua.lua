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

local apdu = {0, 0xA4, 4, 0, 0}
local ara = {9, 0xA0, 0, 0, 0x01, 0x51, 0x41, 0x43, 0x4c, 0}

table.move(ara, 1, #ara, #apdu + 1, apdu)

print(table.concat(ara, ', '))
print(table.concat(apdu, ', '))

resp = C_send_apdu(apdu)
