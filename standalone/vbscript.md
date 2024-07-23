# VBScript Issues

[Visual Pinball](https://github.com/vpinball/vpinball) Standalone uses source code from the [Wine](https://www.winehq.org/) project to support interpretting [VBScript](https://gitlab.winehq.org/wine/wine/-/tree/master/dlls/vbscript?ref_type=heads).

When we first started on the Standalone project, the VBScript interpretter had several issues. 

Many of these issues have been resolved by the Wine team. 

The Visual Pinball team has also added several missing features such as support for:
- `Eval`
- `Execute`
- `ExecuteGlobal`
- `GetRef`

There are still some issues that we have not yet been able to fix, but we have come up with workarounds. 

*Many newer tables already know about these issues and work with no updates.*

## Issues and workarounds:

#### Issue: Settings values in a 2D array does not work

```
' does not work
DTArray(i)(4) = DTCheckBrick(Activeball,DTArray(i)(2))        

' workaround 
DTArray(i).animate = DTCheckBrick(Activeball,DTArray(i).prim) ' move to class approach
```

Note: This is the most common issue found with tables and can be automatically patched by [vpxtool](https://github.com/francisdb/vpxtool).

See: [here](https://github.com/vpinball/vpinball/tree/standalone/standalone#dtarray-drop-targets), [here](https://github.com/vpinball/vpinball/tree/standalone/standalone#starray-standup-targets), and [here](https://bugs.winehq.org/show_bug.cgi?id=53877) for more information.

#### Issue: Parenthesis order when calling function

```
' does not work
addscore (starstate+1)*1000    

' workaround 1
addscore 1000*(starstate+1)

' workaround 2
addscore ((starstate+1)*1000)
```

Note: This issue is difficult to find as the parser doesn't give an exact line number. Bisecting the script multiple times can be effective at narrowing down the source.

See [here](https://bugs.winehq.org/show_bug.cgi?id=54177) for more information.

#### Issue: Using Not in If needs parenthesis

```
' does not work
If isGIOn <> Not IsOff Then

' workaround
If isGIOn <> (Not IsOff) Then
```

See [here](https://bugs.winehq.org/show_bug.cgi?id=55093) for more information.

#### Issue: Constants can't be used before they are defined

```
' does not work
Dim x
x = data
Const data = 1

' workaround
Const data = 1
Dim x
x = data
```

#### Issue: Else..End If on same line with no colon

```
' does not work
else keygrad1 = 0 end if

' workaround
else 
   keygrad1 = 0 
end if
```

#### Issue: Execute fails when object does not exist

```
' does not work
For i=0 To 127: Execute "Set Lights(" & i & ")  = L" & i: Next   

' workaround
For i=0 To 127
    If IsObject(eval("L" & i)) Then
        Execute "Set Lights(" & i & ")  = L" & i
    End If
Next
```

#### Issue: Evals fail when setting 2D array

```
' does not work
dy=-1*(EVAL("roachxy"&xx)(1)(roachstep)-EVAL("roachxy"&xx)(1)(roachstep-1))	'delta Y

' workaround
dim roachxy : roachxy = EVAL("roachxy"&xx)
dy=-1*(roachxy(1)(roachstep)-roachxy(1)(roachstep-1)) 'delta Y
```

#### Issue: Trailing Else

```
' does not work (notice no End If)
If FlasherOnG = False then FlasherTimer3.Enabled = 1: Else 
FlasherTimer4.Enabled = 0

' workaround
If FlasherOnG = False then FlasherTimer3.Enabled = 1
FlasherTimer4.Enabled = 0
```

See [here](https://bugs.winehq.org/show_bug.cgi?id=55006) for more information.

#### Issue: WScript.Shell does not work

```
' does not work
Dim WshShell : Set WshShell = CreateObject("WScript.Shell")
GetNVramPath = WshShell.RegRead("HKEY_CURRENT_USER\Software\Freeware\Visual PinMame\globals\nvram_directory")

' no workaround as we standalone will not support wscript.shell
```

#### Issue: Table crashes immediately after ball hits flipper

```
' CorTracker
Public Sub Update()	'tracks in-ball-velocity
   dim str, b, AllBalls, highestID : allBalls = gBOT

' workaround
Public Sub Update()	'tracks in-ball-velocity
   dim str, b, AllBalls, highestID : allBalls = getballs
```

Note: VBScript uses reference counters for objects. Sometimes when using `gBOT` in this specific instance, the reference counter of a ball will go to zero which causes it to be destroyed. Later code accesses the destroyed ball in `gBOT` causing a crash.
