#ifndef DEBUG_BLACKBOX_H
#define DEBUG_BLACKBOX_H

namespace rde
{
   namespace BlackBox
   {
      void AddMessage(const char* message);
      void AddMessagef(const char* fmt, ...);
      long GetNumMessages();
      const char* GetBoxMessage(long index, int& threadId);
   }
}

#endif
