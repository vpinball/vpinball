'            ***************************************************
'         *********************** B2S.VBS *************************
'      ***************************************************************
'   ***************************By Rosve 2011 ****************************
'***************************************************************************
'** Add this code to the table script to send data to an active backglass ** 
'***************************************************************************
' LaunchBackGlass name, true/false
' ResetB2SData startpos, endpos, value
' SetB2SData pos, value
' StepB2SData startpos, endpos, value, direction, steptime, stepsound
' b2sScore reel, stepvalue, player
' InitB2SScoring rp1_1, rp1_2, rp2_1, rp2_2, rp3_1, rp3_2, ror, rol
' SetB2SScoreSounds 100000, 10000, 1000, 100, 10, 1, RollOver
'
' b2s data value can be in the range of 0 to 254.
Dim B2SVersion
Dim B2SDataRange
Dim B2SData(75)
Dim B2STemp, b2si,b2sStepReady,b2sSP,b2sEP,b2sV,b2sDIR,b2sSS
Dim objRegistry, Key
Set objRegistry = CreateObject("Wscript.shell")
B2SDataRange = 49
B2SVersion = 3

For b2sregclean=0 to 49
   WriteLed b2sregclean,0
Next

'----- Launch a backglass.exe -----------------------------------
Sub LaunchBackGlass(ByVal exename, ByVal flag)
   If Flag=True Then
      exename=exename+".exe"
      objRegistry.Run(exename)
   End If
End Sub

'----- Set a range of B2S Datas to value ----------------------------
Sub ResetB2SData(ByVal StartPos,ByVal EndPos,ByVal Value)
    B2STemp=""
    If StartPos<0 Then StartPos=0
    If EndPos>B2SDataRange then EndPos=B2SDataRange
    For b2si=StartPos to EndPos
       B2SData(b2si)=Chr(Value+1)
    Next
    For b2si=0 to B2SDataRange
       B2STemp=B2STemp+B2SData(b2si)
    Next
    WriteReg(B2STemp)
End Sub

'----- Set a B2S Data to a value (0-254) ---------------------
Sub SetB2SData(ByVal Pos,ByVal Value)
   B2SData(Pos)=Chr(Value+1) 
   B2STemp=""
   For b2si=0 To B2SDataRange
      B2STemp=B2STemp+B2SData(b2si)
   Next
   WriteReg(B2STemp)
End Sub

'------ Step B2S values up or down to a specified value ----------------
Sub StepB2SData (ByVal startpos,ByVal endpos,ByVal value,ByVal direction,ByVal steptime,ByVal stepsound)
   b2sStepReady=0
   b2sSP=startpos
   b2sEP=endpos
   b2sV=value+1
   b2sDIR=direction
   b2sSS=stepsound
   B2Stimer.Interval=steptime
   B2Stimer.Enabled=True
End Sub

Sub B2Stimer_Timer()
   Dim b2sNoChange
   b2sNoChange=1
   For b2si=b2sSP to b2sEP
      Select Case b2sDIR
      Case 0 'Step down to value
         If Asc(B2SData(b2si)) > b2sV then
            B2SData(b2si)=Chr(Asc(B2SData(b2si))-1)
            b2sNoChange=0
         End If
      Case 1 'Step up to value
         If Asc(B2SData(b2si)) < b2sV then
            B2SData(b2si)=Chr(Asc(B2SData(b2si))+1)
            b2sNoChange=0
         End If
      Case 2 'Step up/down to value
         If Asc(B2SData(b2si)) > b2sV then
            B2SData(b2si)=Chr(Asc(B2SData(b2si))-1)
            b2sNoChange=0
         End If
         If Asc(B2SData(b2si)) < b2sV then
            B2SData(b2si)=Chr(Asc(B2SData(b2si))+1)
            b2sNoChange=0
         End If
      End Select
   Next
   If b2sNoChange=1 Then
      b2sStepReady=1
      B2Stimer.Enabled=False
   Else
      PlaySound b2sSS
      B2STemp=""
      For b2si=0 To B2SDataRange
         B2STemp=B2STemp+B2SData(b2si)
      Next
      WriteReg(B2STemp)
   End If
End Sub

'----------- Send LED states to the Backglass --------------
Sub B2SUpdateLed ()
On Error Resume Next
Dim ChgLED, ii, jj, chg, stat
ChgLED = Controller.ChangedLEDs(&H0000003f, &Hffffffff)
   If Not IsEmpty(ChgLED) Then
      For ii = 0 To UBound(ChgLED)
         chg = chgLED(ii, 1):stat = chgLED(ii, 2)

         If chg>0 then WriteLed chgLED(ii, 0),stat
      Next
   End IF
End Sub

'------ Read and Write to the B2S registry data ---------
Function WriteReg(ByVal Value)
   Key = objRegistry.RegWrite("HKCU\B2S\B2SDATA",Value,"REG_SZ")
   WriteReg = Key
End Function

Function ReadReg()
   Key = objRegistry.RegRead("HKCU\B2S\B2SDATA")
   ReadReg = Key
End Function

Function WriteLed(ByVal LedNo,ByVal Value)
   Key = objRegistry.RegWrite("HKCU\B2S\B2SLED"+cstr(LedNo),Value,"REG_DWORD")
   WriteLED = Key
End Function



'*************************
'******** Scoring ********
'*************************

Dim b2sPlayer,b2sTilt,b2sRollOverReel,b2sRollOverLights
Dim b2sReplayLevels(4,2) 'Player,reel1/reel2 values
Dim b2sAddBall(4)        'Replay flags to handle the order of replay levels
Dim b2sScoreSounds(7)
Dim ReelPulses(24,2)     '(PulseQueue, ReelValue)



Sub b2sScore (ByVal reel,ByVal pulses,ByVal player)
    Dim ri
    'Reel Array  -->   ReelPulses(24,2)  '(PulseQueue, ReelValue)
    ri=(player-1)*6+reel
    ReelPulses (ri,1)=ReelPulses (ri,1)+pulses
    b2sPlayer=player
 End Sub
 
 Sub b2sScoreTimer_Timer()
    Dim i
    Dim ri
    If b2sPlayer>0 And b2sTilt<3 Then
       ri=(b2sPlayer-1)*6
       For i =6 to 1 Step -1
          If ReelPulses (ri+i,1)>0 Then
             PlaySound b2sScoreSounds(i)
             ReelPulses (ri+i,1)=ReelPulses (ri+i,1)-1
             ReelPulses (ri+i,2)=ReelPulses (ri+i,2)+1
             If ReelPulses (ri+i,2)=10 Then
                If i=b2sRollOverReel Then
                   ' Overflow
                   ' This is where you turn on the rollover lights
                    If b2sRollOverLights= TRUE Then SetB2SData b2sPlayer+23,1
                    ReelPulses (ri+i,1)=0
                    ReelPulses (ri+i,2)=0
                    PlaySound b2sScoreSounds(7)
                Else
                   ReelPulses (ri+i-1,1)=ReelPulses (ri+i-1,1)+1
                   ReelPulses (ri+i,2)=0
                End If
             End If
             SetB2SData (ri+i-1), ReelPulses (ri+i,2)
             'Check replay scores
             If i=b2sRollOverReel+1 Then                
                If ReelPulses (ri+b2sRollOverReel,2)=b2sReplayLevels(b2sAddBall(b2sPlayerPlayer)+1,1) Then
                   If ReelPulses (ri+b2sRollOverReel+1,2)=b2sReplayLevels(b2sAddBall(b2sPlayerPlayer)+1,2) Then
                      b2sAddBall(b2sPlayerPlayer)=b2sAddBall(b2sPlayerPlayer)+1
                      'Action replay level reached
                       ReplayAction 'This Sub must be present in the table script 
                      '=====================
                   End If
                End If
             End If
          End If
       Next
    End If
 End Sub

Sub InitB2SScoring (ByVal rp1_1,ByVal rp1_2,ByVal rp2_1,ByVal rp2_2,ByVal rp3_1,ByVal rp3_2,ByVal ror, ByVal rol)
   ' This routine must be called before starting a new game
   Dim i
   b2sReplayLevels (1,1)=rp1_1
   b2sReplayLevels (1,2)=rp1_2
   b2sReplayLevels (2,1)=rp2_1
   b2sReplayLevels (2,2)=rp2_2
   b2sReplayLevels (3,1)=rp3_1
   b2sReplayLevels (3,2)=rp3_2
   b2sRollOverReel=ror
   b2sRollOverlights=rol
   b2sAddBall(1)=0
   b2sAddBall(2)=0
   b2sAddBall(3)=0
   b2sAddBall(4)=0
   For i = 0 To 24
      ReelPulses (i,1)=0
      ReelPulses (i,2)=0
   Next
End Sub

Sub SetB2SScoreSounds(ByVal s1,ByVal s2,ByVal s3,ByVal s4,ByVal s5,ByVal s6,ByVal s7)
   b2sScoreSounds(1)=s1
   b2sScoreSounds(2)=s2
   b2sScoreSounds(3)=s3
   b2sScoreSounds(4)=s4
   b2sScoreSounds(5)=s5
   b2sScoreSounds(6)=s6
   b2sScoreSounds(7)=s7
End Sub