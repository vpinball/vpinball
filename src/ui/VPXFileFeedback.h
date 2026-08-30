// license:GPLv3+

#pragma once

class VPXFileFeedback
{
   public:
      virtual ~VPXFileFeedback() {}

      virtual void SetProgress(unsigned int progress) { };
      virtual void SetLength(unsigned int length) { };
};
