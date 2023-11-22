--
--
--

--C_connect_PCSC()

C_select_ISD()

local apdu = {}
local resp = {}
local memf, cod, cor

apdu = {0x80, 0xCA, 0, 0x2F,0}
resp = C_send_apdu(apdu)

apdu = {0, 0xC0, 0, 0 , resp[2]}
resp = C_send_apdu(apdu)

memf = resp[3] * 256 * 256 * 256 + resp[4] * 256 * 256 + resp[5] * 256 + resp[6]
cod = resp[7] * 256 + resp[8]
cor = resp[9] * 256 + resp[10]

print("Free user flash:", memf)
print(" Clear on Deselect:", cod)
print(" Clear on Reset:", cor)

