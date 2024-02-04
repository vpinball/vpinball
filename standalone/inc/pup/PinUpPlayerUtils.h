#pragma once

class PinUpPlayerUtils {
public:
   PinUpPlayerUtils();
   ~PinUpPlayerUtils();

   static vector<string> ParseCSVLine(string line);
};