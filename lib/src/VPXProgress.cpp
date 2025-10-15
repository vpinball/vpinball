#include "VPXProgress.h"

#include "VPinballLib.h"

void VPXProgress::ItemHasBeenProcessed(int itemsCount, int totalItems)
{
   VPinballLib::ProgressData progressData = { 100 * (itemsCount - 1) / totalItems };
   VPinballLib::VPinballLib::SendEvent(VPINBALL_EVENT_LOADING_ITEMS, &progressData);
}


void VPXProgress::SoundHasBeenProcessed(int soundCount, int totalSounds)
{
   VPinballLib::ProgressData progressData = { 100 * (soundCount - 1) / totalSounds };
   VPinballLib::VPinballLib::SendEvent(VPINBALL_EVENT_LOADING_SOUNDS, &progressData);
}


void VPXProgress::ImageHasBeenProcessed(int imageCount, int totalImages)
{
   VPinballLib::ProgressData progressData = { 100 * imageCount / totalImages };
   VPinballLib::VPinballLib::SendEvent(VPINBALL_EVENT_LOADING_IMAGES, &progressData);
}


void VPXProgress::FontHasBeenProcessed(int fontCount, int totalFonts)
{
   VPinballLib::ProgressData progressData = { 100 * (fontCount - 1) / totalFonts };
   VPinballLib::VPinballLib::SendEvent(VPINBALL_EVENT_LOADING_FONTS, &progressData);
}


void VPXProgress::CollectionHasBeenProcessed(int collectionCount, int totalCollections)
{
   VPinballLib::ProgressData progressData = { 100 * (collectionCount - 1) / totalCollections };
   VPinballLib::VPinballLib::SendEvent(VPINBALL_EVENT_LOADING_COLLECTIONS, &progressData);
}
