--
--
--
print " Hello from inside Lua!"

-- Mac:
-- print " Привет!"

local w = 1
local q = 1 + w
local a = 1 + q

print(a, q, w, "..", "Go!!")


--C_upload("M:\\eclipse.oxygen\\ISDR\\bin\\org\\intergalaxy\\rsp\\javacard\\rsp.cap")

--C_install("A0000005591010FFFFFFFF8900000000", "A0000005591010FFFFFFFF8900000100", "", "C900")

C_install("A0000005591010FFFFFFFF8900000000", "A0000005591010FFFFFFFF8900000100", "A0000005591010FFFFFFFF8900000100", "C900")
C_install("A0000005591010FFFFFFFF8900000000", "A0000005591010FFFFFFFF8900000200", "A0000005591010FFFFFFFF8900000200", "C900")

