# VBScript Issues and Workarounds

This page documents known issues with the VBScript interpreter in Visual Pinball Standalone, along with workarounds. Many issues have been fixed by the Wine team, and the Visual Pinball team has added several missing features (e.g., support for `Eval`, `Execute`, `ExecuteGlobal`, and `GetRef`). For more details, refer to the linked external resources.

>[!NOTE]
Many newer tables already know about these issues and work with no updates.

---

## Table of Contents

1. [Syntax and Parsing Issues](#syntax-and-parsing-issues)
2. [Declaration and Definition Issues](#declaration-and-definition-issues)
3. [Array and Evaluation Issues](#array-and-evaluation-issues)
4. [Loop and Type Conversion Issues](#loop-and-type-conversion-issues)
5. [Execution Issues](#execution-issues)
6. [Object and Shell Issues](#object-and-shell-issues)
7. [RegExp is not implemented](#regexp-is-not-implemented)

---

## Syntax and Parsing Issues

### 1. Parenthesis Order When Calling a Function

**Issue:**
```vbscript
' does not work
addscore (starstate+1)*1000    
```

**Workarounds:**
- **Option 1:**  
  ```vbscript
  addscore 1000*(starstate+1)
  ```
- **Option 2:**  
  ```vbscript
  addscore ((starstate+1)*1000)
  ```

>[!Tip]
This issue is hard to spot as the parser does not provide an exact error line. Bisecting the script may help locate the problem.  
See: [[Wine Bug #54177]](https://bugs.winehq.org/show_bug.cgi?id=54177)

---

### 2. Using `Not` in an `If` Statement Needs Parentheses

**Issue:**
```vbscript
' does not work
If isGIOn <> Not IsOff Then
```

**Workaround:**
```vbscript
If isGIOn <> (Not IsOff) Then
```

See: [[Wine Bug #55093]](https://bugs.winehq.org/show_bug.cgi?id=55093)

---

### 3. Else..End If on the Same Line Without a Colon

**Issue:**
```vbscript
' does not work
else keygrad1 = 0 end if
```

**Workaround:**
```vbscript
else 
   keygrad1 = 0 
end if
```

---

### 4. Colon on a New Line After `Then` Fails to Compile

**Issue:**
```vbscript
' does not work
If Keycode = StartGameKey Then
  :pupevent 800
End If
```

**Workaround:**
```vbscript
If Keycode = StartGameKey Then
  pupevent 800
End If
```

See: [[Wine Bug #55037]](https://bugs.winehq.org/show_bug.cgi?id=55037)

---

## Declaration and Definition Issues

### 5. Constants Must Be Defined Before They Are Used

**Issue:**
```vbscript
' does not work
Dim x
x = data
Const data = 1
```

**Workaround:**
```vbscript
Const data = 1
Dim x
x = data
```

>[!IMPORTANT]
Always define constants before using them to prevent runtime errors.

---

## Array and Evaluation Issues

### 6. Setting Values in a 2D Array

**Issue:**
```vbscript
' does not work
DTArray(i)(4) = DTCheckBrick(Activeball,DTArray(i)(2))        
```

**Workaround:**
```vbscript
DTArray(i).animate = DTCheckBrick(Activeball,DTArray(i).prim) ' move to class approach
```

>[!Note]
This is one of the most common issues. It can be automatically patched by [vpxtool](https://github.com/francisdb/vpxtool).
See: [[Wine Bug #53877]](https://bugs.winehq.org/show_bug.cgi?id=53877)

<details>
<summary>DTArray (Drop Targets) Workaround</summary>
<br/>

**1) Add the `DropTarget` class:**

```vbscript
Class DropTarget
Private m_primary, m_secondary, m_prim, m_sw, m_animate, m_isDropped

Public Property Get Primary(): Set Primary = m_primary: End Property
Public Property Let Primary(input): Set m_primary = input: End Property

Public Property Get Secondary(): Set Secondary = m_secondary: End Property
Public Property Let Secondary(input): Set m_secondary = input: End Property

Public Property Get Prim(): Set Prim = m_prim: End Property
Public Property Let Prim(input): Set m_prim = input: End Property

Public Property Get Sw(): Sw = m_sw: End Property
Public Property Let Sw(input): m_sw = input: End Property

Public Property Get Animate(): Animate = m_animate: End Property
Public Property Let Animate(input): m_animate = input: End Property

Public Property Get IsDropped(): IsDropped = m_isDropped: End Property
Public Property Let IsDropped(input): m_isDropped = input: End Property

Public default Function init(primary, secondary, prim, sw, animate, isDropped)
Set m_primary = primary
Set m_secondary = secondary
Set m_prim = prim
m_sw = sw
m_animate = animate
m_isDropped = isDropped

Set Init = Me
End Function
End Class
```

**2) Update DT definitions to use `DropTarget` instead of `Array`:**

```vbscript
' Before:
DT7 = Array(dt1, dt1a, pdt1, 7, 0, false)
DT27 = Array(dt2, dt2a, pdt2, 27, 0, false)
DT37 = Array(dt3, dt3a, pdt3, 37, 0, false)

' After:
Set DT7 = (new DropTarget)(dt1, dt1a, pdt1, 7, 0, false)
Set DT27 = (new DropTarget)(dt2, dt2a, pdt2, 27, 0, false)
Set DT37 = (new DropTarget)(dt3, dt3a, pdt3, 37, 0, false)
```

**3) Search and replace array indices with properties:**

| From | To | Vi Command |
| --- | --- | --- |
| `DTArray(i)(0)` | `DTArray(i).primary` | `:%s/DTArray(i)(0)/DTArray(i).primary/g` |
| `DTArray(i)(1)` | `DTArray(i).secondary` | `:%s/DTArray(i)(1)/DTArray(i).secondary/g` |
| `DTArray(i)(2)` | `DTArray(i).prim` | `:%s/DTArray(i)(2)/DTArray(i).prim/g` |
| `DTArray(i)(3)` | `DTArray(i).sw` | `:%s/DTArray(i)(3)/DTArray(i).sw/g` |
| `DTArray(i)(4)` | `DTArray(i).animate` | `:%s/DTArray(i)(4)/DTArray(i).animate/g` |
| `DTArray(i)(5)` | `DTArray(i).isDropped` | `:%s/DTArray(i)(5)/DTArray(i).isDropped/g` |
| `DTArray(ind)(5)` | `DTArray(ind).isDropped` | `:%s/DTArray(ind)(5)/DTArray(ind).isDropped/g` |

</details>

<details>
<summary>STArray (Standup Targets) Workaround</summary>
<br/>

**1) Add the `StandupTarget` class:**

```vbscript
Class StandupTarget
Private m_primary, m_prim, m_sw, m_animate

Public Property Get Primary(): Set Primary = m_primary: End Property
Public Property Let Primary(input): Set m_primary = input: End Property

Public Property Get Prim(): Set Prim = m_prim: End Property
Public Property Let Prim(input): Set m_prim = input: End Property

Public Property Get Sw(): Sw = m_sw: End Property
Public Property Let Sw(input): m_sw = input: End Property

Public Property Get Animate(): Animate = m_animate: End Property
Public Property Let Animate(input): m_animate = input: End Property

Public default Function init(primary, prim, sw, animate)
Set m_primary = primary
Set m_prim = prim
m_sw = sw
m_animate = animate

Set Init = Me
End Function
End Class
```

**2) Update ST definitions to use `StandupTarget` instead of `Array`:**

```vbscript
' Before:
ST41 = Array(sw41, Target_Rect_Fat_011_BM_Lit_Room, 41, 0)
ST42 = Array(sw42, Target_Rect_Fat_010_BM_Lit_Room, 42, 0)
ST43 = Array(sw43, Target_Rect_Fat_005_BM_Lit_Room, 43, 0)

' After:
Set ST41 = (new StandupTarget)(sw41, Target_Rect_Fat_011_BM_Lit_Room, 41, 0)
Set ST42 = (new StandupTarget)(sw42, Target_Rect_Fat_010_BM_Lit_Room, 42, 0)
Set ST43 = (new StandupTarget)(sw43, Target_Rect_Fat_005_BM_Lit_Room, 43, 0)
```

**3) Search and replace array indices with properties:**

| From | To | Vi Command |
| --- | --- | --- |
| `STArray(i)(0)` | `STArray(i).primary` | `:%s/STArray(i)(0)/STArray(i).primary/g` |
| `STArray(i)(1)` | `STArray(i).prim` | `:%s/STArray(i)(1)/STArray(i).prim/g` |
| `STArray(i)(2)` | `STArray(i).sw` | `:%s/STArray(i)(2)/STArray(i).sw/g` |
| `STArray(i)(3)` | `STArray(i).animate` | `:%s/STArray(i)(3)/STArray(i).animate/g` |

</details>

---

### 7. Evals Fail When Setting a 2D Array

**Issue:**
```vbscript
' does not work
dy = -1*(EVAL("roachxy" & xx)(1)(roachstep) - EVAL("roachxy" & xx)(1)(roachstep-1)) 'delta Y
```

**Workaround:**
```vbscript
dim roachxy : roachxy = EVAL("roachxy" & xx)
dy = -1*(roachxy(1)(roachstep) - roachxy(1)(roachstep-1)) 'delta Y
```

---

## Loop and Type Conversion Issues

### 8. Coercion Issue in a For Loop When Right Bound Is a String

**Issue:**
```vbscript
' This code loops 16 times on Windows but on Linux it leads to a very large numerical value.
' This results in the loop iterating an extremely high number of times.
Dim i, num
num = "16"
For i = 0 To num
    WScript.Echo i
Next
```

**Workaround:**
```vbscript
For i = 0 To CInt(num)
```

See: [[Wine Bug #55052]](https://bugs.winehq.org/show_bug.cgi?id=55052)

---

## Execution Issues

### 9. Execute Fails When Object Does Not Exist

**Issue:**
```vbscript
' does not work
For i = 0 To 127: Execute "Set Lights(" & i & ") = L" & i: Next   
```

**Workaround:**
```vbscript
For i = 0 To 127
    If IsObject(Eval("L" & i)) Then
        Execute "Set Lights(" & i & ") = L" & i
    End If
Next
```

---

### 10. Trailing Else Without `End If`

**Issue:**
```vbscript
' does not work (notice no End If)
If FlasherOnG = False then FlasherTimer3.Enabled = 1: Else 
FlasherTimer4.Enabled = 0
```

**Workaround:**
```vbscript
If FlasherOnG = False then FlasherTimer3.Enabled = 1
FlasherTimer4.Enabled = 0
```

See: [[Wine Bug #55006]](https://bugs.winehq.org/show_bug.cgi?id=55006)

---

## Object and Shell Issues

### 11. WScript.Shell Does Not Work

**Issue:**
```vbscript
' does not work
Dim WshShell : Set WshShell = CreateObject("WScript.Shell")
GetNVramPath = WshShell.RegRead("HKEY_CURRENT_USER\Software\Freeware\Visual PinMame\globals\nvram_directory")
```

>[!WARNING]
There is no workaround because the Standalone version will not support `WScript.Shell`.

>[!TIP]
Sometimes you can just comment out the line.

---

### 12. Table Crashes Immediately After Ball Hits Flipper

**Issue:**
```vbscript
' CorTracker
Public Sub Update()    ' tracks in-ball-velocity
   Dim str, b, AllBalls, highestID : AllBalls = gBOT
```

**Workaround:**
```vbscript
Public Sub Update()    ' tracks in-ball-velocity
   Dim str, b, AllBalls, highestID : AllBalls = getballs
```

>[!NOTE] 
VBScript uses reference counters for objects. In this case, using `gBOT` may reduce a ball's reference count to zero, leading to its destruction and causing subsequent code to crash when accessing a destroyed object.


## RegExp is not implemented

### 13. Creating a RegExp will crash Visual Pinball

**Issue:**
```vbscript
pattern = "current_player\.([a-zA-Z0-9_]+)"
Set regex = New RegExp
regex.Pattern = pattern
regex.IgnoreCase = True
regex.Global = True
replacement = "GetPlayerState(""$1"")"
outputString = regex.Replace(inputString, replacement)
```

**Workaround:**

Use `InStr`, `Replace` and other string functions to search/replace parts of a string.

---

## Conclusion

Many of these issues stem from differences in how VBScript is interpreted in the Wine environment compared to native Windows environments. Where possible, workarounds are provided. For further automation, consider using tools like [vpxtool](https://github.com/francisdb/vpxtool).

>[!TIP]
If you encounter any new issues, check the Wine project's bug tracker for similar reports.

---

*This document is intended to help developers quickly identify and resolve common VBScript issues in Visual Pinball Standalone.*
