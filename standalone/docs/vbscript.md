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
See: [[DTArray Drop Targets]](https://github.com/vpinball/vpinball/tree/standalone/standalone#dtarray-drop-targets), [[STArray Standup Targets]](https://github.com/vpinball/vpinball/tree/standalone/standalone#starray-standup-targets), and [[Wine Bug #53877]](https://bugs.winehq.org/show_bug.cgi?id=53877)

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
