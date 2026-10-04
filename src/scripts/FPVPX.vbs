'*************************************************************************************
'Utilidades para ahorrar trabajo en las converciones de Future Pinball a VPX VER.1.1
'*************************************************************************************
Option Explicit

Const FPVPXv = 1.02

'*****************
' PARA LAS LUCES
'*****************

Const BulbOn = "1"  'Luz encendida
Const BulbOff = "0" 'Luz Apagada
Const BulbBlink = "2" 'Luz intermitente

'---------------------------------------------
' Algunas Variables

Dim fpTilted:If IsEmpty(Eval("Tilted"))=true Then fpTilted=false Else fpTilted=Tilted 'Falta

Dim nvCredits:If IsEmpty(Eval("Credits"))=true Then nvCredits=0 Else nvCredits=Credits 'Creditos

Dim nvTotalGamesPlayed:If IsEmpty(Eval("TotalGamesPlayed"))=true Then nvTotalGamesPlayed=0 Else nvTotalGamesPlayed=TotalGamesPlayed 'Total de partidas Jugadas

Dim nvScore:If IsEmpty(Eval("Score"))=true Then nvScore=Array(0,0,0,0) Else nvScore=Score 'Marcador

Dim nvJackpot:If IsEmpty(Eval("Jackpot"))=true Then nvJackpot=0 Else nvJackpot=Jackpot 'Jackpot
