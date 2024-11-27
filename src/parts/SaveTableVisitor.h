#pragma once


/**
 * A abstraction that adds some functionalities to the table saving process.
 *
 * It must be overriden to provide any feature. This implementation does not
 * do anything and should be used when no additional feature is needed on table
 * save.
 */
class SaveTableVisitor
{
   public:
      virtual ~SaveTableVisitor() {}

      // Called at the very start of the table save process.
      virtual void SavingStarted() {}
      // Called when we start saving individual table items. 'itemCount' is
      // the total number of items that will be saved.
      virtual void AboutToSaveItems(int itemCount) {}
      // Called when an item has been persisted. 'totalItemSaved' is the total
      // number of item saved until now. It should not exceed 'itemCount'
      virtual void ItemHasBeenSaved(int totalItemSaved) {}
      // Called when all items have been saved and the file is ready to be
      // persisted on disc.
      virtual void Finalizing() {}
      // Called if an error occured while saving the table. 'error' contains
      // some details about the error.
      virtual void ErrorOccured(const char* error) {}
      // Called when existing the table saving procedure. This method is called
      // either on success or failure.
      virtual void DoneSaving() {}
};
