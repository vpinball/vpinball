#pragma once
class IDebugCommands
	{
public:
	virtual void GetDebugCommands(VectorInt<int> *pvids, VectorInt<int> *pvcommandid) = 0;
	virtual void RunDebugCommand(int id) = 0;
	};