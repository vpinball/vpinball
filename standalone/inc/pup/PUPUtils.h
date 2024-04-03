#pragma once

class PUPUtils {
public:
   PUPUtils();
   ~PUPUtils();

   static vector<string> ParseCSVLine(string line);
};