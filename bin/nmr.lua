--
--
--
print " Load and install NMR by Lua!"

C_execute("/card")
C_execute("auth")

C_execute("delete -r A0000007400000000100")

C_upload("cap/tracker.cap")

C_install("A0000007400000000100", "A000000740000000010000", "A00000074041", "C900")

