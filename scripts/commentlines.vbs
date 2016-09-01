Set objFS = CreateObject("Scripting.FileSystemObject")
Set objArgs = WScript.Arguments
strFilename = objArgs(0)
lineStart = CInt(objArgs(1))
lineEnd = CInt(objArgs(2))
outFilename = objArgs(3)

Const ForReading = 1, ForWriting = 2, ForAppending = 8
Set objFile = objFS.OpenTextFile(strFilename)
Set outFile = objFS.OpenTextFile(outFilename, ForWriting, True)
lineCount = 1
Do Until objFile.AtEndOfStream
    strLine = objFile.ReadLine
	If lineCount >= lineStart And lineCount <= lineEnd Then
		If Len(strLine) > 0 And Left(strLine, 1) <> "/" Then
			outFile.WriteLine "// " + strLine
		Else
			outFile.WriteLine strLine
		End If
	Else
		outFile.WriteLine strLine
	End If
	lineCount = lineCount + 1
Loop