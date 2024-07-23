Option Explicit

Const cGameName = "GrandTour_1964"
Const cOptions = "GrandTour_1964_v2.06.txt"

Dim x, y, i
Dim initial(6,5)
Dim highScore(5)
Dim credit, freeplay, balls, chime, slide, lutValue, matchNumber, pfOption

loadHighScore

Debug.Print	"credit " & credit
Debug.Print	"freePlay " & freePlay
Debug.Print	"balls " & balls
Debug.Print	"matchNumber " & matchNumber
Debug.Print	"chime " & chime
Debug.Print	"pfOption " & pfOption
Debug.Print	"lutValue " & lutValue
Debug.Print	"slide " & slide

Sub loadHighScore
	Dim fileObj
	Dim scoreFile
	Dim temp(40)
	Dim textStr

	dim hiInitTemp(3)
	dim hiInit(5)

    Set fileObj = CreateObject("Scripting.FileSystemObject")
	If Not fileObj.FolderExists(UserDirectory) Then 
		Exit Sub
	End If
	If Not fileObj.FileExists(UserDirectory & cOptions) Then
		Exit Sub
	End If
	Set scoreFile = fileObj.GetFile(UserDirectory & cOptions)
	Set textStr = scoreFile.OpenAsTextStream(1,0)
		If (textStr.AtEndOfStream = True) Then
			Exit Sub
		End If

		For x = 1 to 33
			temp(x) = textStr.readLine
		Next
		TextStr.Close
	
		For x = 0 to 4
			highScore(x) = cdbl (temp(x+1))
		Next

		For x = 0 to 4
			hiInit(x) = (temp(x + 6))
		Next

		i = 10
		For x = 0 to 4
			For y = 1 to 3
				i = i + 1
				initial(x,y) = cdbl (temp(i))
			Next
		Next
		credit = CInt(Right(temp(26),1))
		freePlay = CInt(Right(temp(27),1))
		balls = CInt(Right(temp(28),1))
		matchNumber = CInt(Right(temp(29),1))
		chime = CInt(Right(temp(30),1))
		pfOption = CInt(Right(temp(31),1))
		lutValue = CInt(Right(temp(32),1))
		slide = CInt(Right(temp(33),2))

		For x = 0 to 4 
			Debug.Print highScore(x) 
			Debug.Print hiInit(x)
		Next

		Set scoreFile = Nothing
	    Set fileObj = Nothing
End Sub