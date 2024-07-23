Dim obj_datadict
Set obj_datadict = CreateObject("Scripting.Dictionary")

obj_datadict.Add "a", "Apple"
obj_datadict.Add "b", "BlueTooth"
obj_datadict.Add "c", "C++"

Debug.Print "c exists: " & obj_datadict.Exists("c")
Debug.Print "d exists: " & obj_datadict.Exists("d")

a = obj_datadict.items
         
Debug.print "a(0) = " & a(0)
Debug.print "a(2) = " & a(2)

b = obj_datadict.Keys
         
Debug.print b(0)
Debug.print b(2)

Debug.print "b(0) = " & b(0)
Debug.print "b(2) = " & b(2)

Debug.print "Hash c: " & obj_datadict.HashVal("c")

obj_datadict.remove("b")  

Debug.Print "remove b, b exists: " & obj_datadict.Exists("b")

Debug.print "count: " & obj_datadict.Count

obj_datadict.RemoveAll 

Debug.print "remove all count: " & obj_datadict.Count
