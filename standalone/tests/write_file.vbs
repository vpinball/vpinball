Option Explicit

Const cGameName = "GrandTour_1964"
Const cOptions = "GrandTour_1964_v2.06.txt"
Const hsFileName = "Grand Tour (Bally 1964)"
Const cPinballY = 1

Dim initial(6,5)
Dim highScore(5)
Dim credit, freeplay, balls, chime, slide, lutValue, matchNumber, pfOption

If initial(0,1) = "" Then
	initial(0,1) = 19: initial(0,2) = 5: initial(0,3) = 13
	initial(1,1) = 1: initial(1,2) = 1: initial(1,3) = 1
	initial(2,1) = 2: initial(2,2) = 2: initial(2,3) = 2
	initial(3,1) = 3: initial(3,2) = 3: initial(3,3) = 3
	initial(4,1) = 4: initial(4,2) = 4: initial(4,3) = 4
End If

If highScore(0) = "" Then highScore(0) = 1500
If highScore(1) = "" Then highScore(1) = 1200
If highScore(2) = "" Then highScore(2) = 1000
If highScore(3) = "" Then highScore(3) = 200
If HighScore(4) = "" Then highScore(4) = 100
If matchNumber = "" Then matchNumber = 4
If pfOption = "" Then pfOption = 1
If credit = "" Then credit = 0
If freePlay = "" Then freePlay = 1
If balls = "" Then balls = 5
If chime = "" Then chime = 0
If slide = "" Then slide = 71
If lutValue = "" Then lutValue = 0

saveHighScore

Sub saveHighScore
	Dim x, y
	Dim hiInit(5)
	Dim hiInitTemp(5)
	Dim FolderPath
	For x = 0 to 4
		For y = 1 to 3
			hiInitTemp(y) = chr(initial(x,y) + 64)
		Next
		hiInit(x) = hiInitTemp(1) + hiInitTemp(2) + hiInitTemp(3)
	Next
	Dim fileObj
	Dim scoreFile
	Debug.Print userDirectory
	Set fileObj = createObject("Scripting.FileSystemObject")
	If Not fileObj.folderExists(userDirectory) Then 
		Exit Sub
	End If
	Set scoreFile = fileObj.createTextFile(userDirectory & cOptions,True)

		For x = 0 to 4
			scoreFile.writeLine highScore(x)
		Next
		For x = 0 to 4
			scoreFile.writeLine hiInit(x)
		Next
		For x = 0 to 4
			For y = 1 to 3
				scoreFile.writeLine initial(x,y)
			Next
		Next
		scoreFile.WriteLine "Credits: " & credit
		scorefile.writeline "FreePlay (0 = coin, 1 = free): " & freePlay
		scoreFile.WriteLine "Balls: " & balls
		scoreFile.WriteLine "Match Number: " & matchNumber
		scoreFile.WriteLine "Chime (0 = sound file, 1 = DOF chime): " & chime
		scoreFile.WriteLine "pfOption (1 = L/R Stereo, 2 = U/D Stereo, 3 = quad): " & pfOption
		scoreFile.WriteLine "LUT: " & lutValue
		scoreFile.WriteLine "Slide Number: " & slide
		scoreFile.Close
	Set scoreFile = Nothing
	Set fileObj = Nothing

	Set FileObj = CreateObject("Scripting.FileSystemObject")

	If cPinballY <> 1 Then Exit Sub

	If Not FileObj.FolderExists(UserDirectory) Then 
		Exit Sub
	End If

	FolderPath = FileObj.GetParentFolderName(UserDirectory)

	If cPinballY = 1 Then
		Set ScoreFile = FileObj.CreateTextFile(UserDirectory & hsFileName & ".PinballYHighScores",True)	
	End If

	For x = 0 to 4 
		ScoreFile.WriteLine HighScore(x) 
		ScoreFile.WriteLine HiInit(x)
	Next
	ScoreFile.Close
	Set ScoreFile = Nothing
	Set FileObj = Nothing

End Sub