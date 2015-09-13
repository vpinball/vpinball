#pragma once
class IDebugCommands
{
public:
   virtual void GetDebugCommands(std::vector<int> & pvids, std::vector<int> & pvcommandid) = 0;
   virtual void RunDebugCommand(int id) = 0;
};
