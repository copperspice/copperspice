Sub QAxWidget2_Click
	QAxWidget2.lineWidth = QAxWidget2.lineWidth + 1
	MainWindow.logMacro 0, "Hello from VBScript: QAxWidget2_Click", 0, ""
End Sub

Sub fatLines
	QAxWidget2.lineWidth = 25
End Sub

Sub thinLines
	QAxWidget2.lineWidth = 1
End Sub

Sub setLineWidth(width)
	QAxWidget2.lineWidth = width
End Sub

Public Function getLineWidth
	getLineWidth = QAxWidget2.lineWidth
End Function
