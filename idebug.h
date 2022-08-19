#pragma once

class IDebugCommands
{
public:
   virtual void GetDebugCommands(vector<int> & pvids, vector<int> & pvcommandid) = 0;
   virtual void RunDebugCommand(int id) = 0;
};
