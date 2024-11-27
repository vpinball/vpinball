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

      // Called when we start processing individual table elements. 'itemCount' is
      // the total number of elements the table contains.
      // Elements represent any entity needed to be loaded.
      virtual void AboutToProcessTable(int elementCount) {}

      // Called when a table item has been processed. 'itemsCount' is the total
      // number of items processed so far. 'totalItems' is the total number of items
      // the table contains.
      virtual void ItemHasBeenProcessed(int itemsCount, int totalItems) {}

      // Called when a table sound has been processed. 'soudCount' is the total
      // number of sound files processed so far. 'totalSounds' is the total number
      // of sounds the table contains.
      virtual void SoundHasBeenProcessed(int soundCount, int totalSounds) {}

      // Called when a table sound has been processed. 'imageCount' is the total
      // number of sound files processed so far. 'totalImages' is the total number
      // of sounds the table contains.
      virtual void ImageHasBeenProcessed(int imageCount, int totalImages) {}

      // Called when a table font has been processed. 'fontCount' is the total
      // number of font files processed so far. 'totalFonts' is the total number
      // of fonts the table contains.
      virtual void FontHasBeenProcessed(int fontCount, int totalFonts) {}

      // Called when a table font has been processed. 'collectionCount' is the total
      // number of collections processed so far. 'totalCollections' is the total number
      // of collections the table contains.
      virtual void CollectionHasBeenProcessed(int collectionCount, int totalCollections) {}

      // Called when all items have been processed and the file is ready to be
      // used.
      virtual void Finalizing() {}

      // Called if an error occured while operating on the table. 'error' contains
      // some details about the error.
      virtual void ErrorOccured(const char* error) {}

      // Called when existing the table operation. This method is called
      // either on success or failure.
      virtual void Done() {}
};
