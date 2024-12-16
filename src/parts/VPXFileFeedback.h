#pragma once


/**
 * A abstraction that adds some functionalities to the VPX file read/write process.
 *
 * It must be overriden to provide any feature. This implementation does not
 * do anything and should be used when no additional feature is needed on table
 * save.
 */
class VPXFileFeedback
{
   public:
      virtual ~VPXFileFeedback() {}

      // Called at the very start of an operation on a file
      virtual void OperationStarted() {}

      // Called when we start processing individual table items. 'itemCount' is
      // the total number of items the table contains.
      virtual void AboutToProcessItems(int itemCount) {}

      // Called when an item has been processed. 'totalItemProcessed' is the total
      // number of items processed until now. It should not exceed 'itemCount'
      virtual void ItemHasBeenProcessed(int totalItemProcessed) {}

      // Called when all items have been preocessed and the file is ready to be
      // used.
      virtual void Finalizing() {}

      // Called if an error occured while operating on the table. 'error' contains
      // some details about the error.
      virtual void ErrorOccured(const char* error) {}

      // Called when existing the table operation. This method is called
      // either on success or failure.
      virtual void Done() {}
};
