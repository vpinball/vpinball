'Last Updated in VBS v3.13
' - Fixed Labeling Errors

Option Explicit
Dim keyShowOpts,keyShowKeys,keyReset,keyFrame,keyDoubleSize,keyShowDips,keyAddBall
Dim keyBangBack,keyInsertCoin1,keyInsertCoin2,keyInsertCoin3,keyInsertCoin4,keySlamDoorHit
Dim keyCoinDoor,keyCancel,keyDown,keyUp,keyEnter,keyAdvance,keyUpDown,keyCPUDiag,keySoundDiag
Dim keyMasterEnter,keyHiScoreReset,keySelfTest,keyBlack,keyGreen,keyRed,keyKPGame,keyKPEnter,keyVPMVolume
Dim keyUpperLeft,keyUpperRight,keyFront,keyRules,keyJoyUp,keyJoyDown,keyJoyLeft,keyJoyRight
Dim keyFire1,keyFire2,keyGameSpecific1,keyGameSpecific2,keyGameSpecific3,keyGameSpecific4
'
' This is a list of standard keys used by *.vbs files
'
keyShowOpts     = 59  '(F1)    Show options
keyShowKeys     = 60  '(F2)    Show Keys
keyReset        = 61  '(F3)    Reset Emulation
keyFrame        = 62  '(F4)    Toggle Window Lock
keyDoubleSize   = 63  '(F5)    Toggle displaysize
keyShowDips     = 64  '(F6)    Show Dip Switch / Options Menu
keyAddBall      = 48  '(B)     Add extra ball
keyBangBack     = 20  '(T)     Bang Back

keyInsertCoin1   = 4   '(3)    Insert coin in slot 1
keyInsertCoin2   = 5   '(4)    Insert coin in slot 2
keyInsertCoin3   = 6   '(5)    Insert coin in slot 3
keyInsertCoin4   = 7   '(6)    Insert coin in slot 4
keySlamDoorHit   = 199 '(Home) Slam Tilt
keyCoinDoor      = 207 '(End)  Opening/Closing Coin Door

keyCancel        = 8   '(7)    Coin Door Cancel Button (WPC)
keyDown          = 9   '(8)    Coin Door Down Button (WPC)
keyUp            = 10  '(9)    Coin Door Up Button (WPC)
keyEnter         = 11  '(0)    Coin Door Enter Button (WPC)
keyAdvance       = 9   '(8)    Coin Door Advance Button
keyUpDown        = 8   '(7)    Coin Door Up/Down Button
keyCPUDiag       = 10  '(9)    CPU Diagnostic Button
keySoundDiag     = 11  '(0)    Sound CPU Diagnostic Button
keyMasterEnter   = 201 '(PgUp) Master Enter
keyHiScoreReset  = 7   '(6)    Reset Hiscores
keySelfTest      = 8   '(7)
keyBlack         = 8   '(7)    Sega/Data East/Stern
keyGreen         = 9   '(8)    Sega/Data East/Stern
keyRed           = 10  '(9)    Sega/Data East/Stern
keyKPGame        = 7   '(6)    Bally 6803
keyKPEnter       = 156
keyVPMVolume     = 88  '(F12)  Added Control to set VPM Volume
'
' Optional keys
' These are not used nor handled by the vbs files
' They are only here to provide consistant keys for the games.
'
keyUpperLeft     = 30  '(A)    Upper Left Flipper button
keyUpperRight    = 40  '(')    Upper Right Flipper button
keyFront         = 3   '(2)    Front button (e.g. buy-in)
keyRules         = 19  '(R)    Show rules
keyJoyUp         = 200 '(Up arrow)
keyJoyDown       = 208 '(Down arrow)
keyJoyLeft       = 203 '(Left arrow)
keyJoyRight      = 205 '(Right arrow)
keyFire1         = 29  '(Left CTRL)
keyFire2         = 184 '(Right Alt)
keyGameSpecific1 = 16  '(Q)
keyGameSpecific2 = 17  '(W)
keyGameSpecific3 = 18  '(E)
keyGameSpecific4 = 19  '(R)
