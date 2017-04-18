'Last Updated in VBS v3.13
' - Fixed Labeling Errors

'Key codes that can be used in the mappings below:
'0          11      L CONTROL       29
'1          2       LEFT ARROW      203
'2          3       L SHIFT         42
'3          4       L WIN           219
'4          5       M               50
'5          6       MINUS           12
'6          7       MULTIPLY        55
'7          8       N               49
'8          9       NUM LOCK        69
'9          10      NUMPAD 0        82
'A          30      NUMPAD 1        79
'ADD        78      NUMPAD 2        80
'APOSTROPHE 40      NUMPAD 3        81
'AT         145     NUMPAD 4        75
'AX         150     NUMPAD 5        76
'B          48      NUMPAD 6        77
'BACK       14      NUMPAD 7        71
'BACKSLASH  43      NUMPAD 8        72
'BACKSPACE  14      NUMPAD 9        73
'C          46      NUMPAD COMMA    179
'CALCULATOR 161     NUMPAD ENTER    156
'CAPS LOCK  58      NUMPAD EQUALS   141
'COLON      146     NUMPAD MINUS    74
'COMMA      51      NUMPAD PERIOD   83
'D          32      NUMPAD PLUS     78
'DECIMAL    83      NUMPAD SLASH    181
'DELETE     211     NUMPAD ASTERISK 55
'DIVIDE     181     O               24
'DOWN       208     P               25
'DOWN ARROW 208     PAUSE           197
'E          18      PERIOD          52
'END        207     PAGE DOWN       209
'EQUALS     13      PAGE UP         201
'ESCAPE     1       Q               16
'F          33      R               19
'F1         59      R ALT           184
'F2         60      R BRACKET       27
'F3         61      R CTRL          157
'F4         62      RETURN          28
'F5         63      RIGHT           205
'F6         64      RIGHT ARROW     205
'F7         65      R MENU          184
'F8         66      R SHIFT         54
'F9         67      R WIN           220
'F10        68      S               31
'F11        87      SCROLL          70
'F12        88      SEMICOLO        39
'F13        100     SLASH           53
'F14        101     SPACE           57
'F15        102     STOP            149
'G          34      T               20
'H          35      TAB             15
'HOME       199     U               22
'I          23      UNDERLINE       147
'INSERT     210     UP ARROW        200
'J          36      V               47
'K          37      W               17
'L          38      X               45
'L ALT      56      Y               21
'L BRACKET  26      Z               44

Option Explicit
Dim toggleKeyCoinDoor,inverseKeyCoinDoor
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
toggleKeyCoinDoor = True '     If true then a key press/switch change will flip the state from on/off, if false then one will have the real coindoor behaviour (for cabinet setups with real coindoors)
inverseKeyCoinDoor = False '   If false then the coindoor switch behaves like a original coindoor (key pressed = closed, key not pressed = open), otherwise the switch is inverted (key pressed = open, key not pressed = closed)

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
keyKPEnter       = 156 '(NumpadEnter)
keyVPMVolume     = 88  '(F12)  Added Control to set VPM Volume
'
' Optional keys
' These are not used nor handled by the vbs files
' They are only here to provide consistent keys for the games.
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
