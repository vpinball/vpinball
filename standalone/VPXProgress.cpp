#include "VPXProgress.h"

#include "VPinballLib.h"


void VPXProgress::ItemHasBeenProcessed(int itemsCount, int totalItems)
{
   VPinballLib::ProgressData progressData = { 100 * (itemsCount - 1) / totalItems };
   VPinballLib::VPinball::SendEvent(VPinballLib::Event::LoadingItems, &progressData);
}


void VPXProgress::SoundHasBeenProcessed(int soundCount, int totalSounds) 
{
   VPinballLib::ProgressData progressData = { 100 * (soundCount - 1) / totalSounds };
   VPinballLib::VPinball::SendEvent(VPinballLib::Event::LoadingSounds, &progressData);
}


void VPXProgress::ImageHasBeenProcessed(int imageCount, int totalImages)
{
   VPinballLib::ProgressData progressData = { 100 * imageCount / totalImages };
   VPinballLib::VPinball::SendEvent(VPinballLib::Event::LoadingImages, &progressData);
}


void VPXProgress::FontHasBeenProcessed(int fontCount, int totalFonts)
{
   VPinballLib::ProgressData progressData = { 100 * (fontCount - 1) / totalFonts };
   VPinballLib::VPinball::SendEvent(VPinballLib::Event::LoadingFonts, &progressData);
}


void VPXProgress::CollectionHasBeenProcessed(int collectionCount, int totalCollections)
{
   VPinballLib::ProgressData progressData = { 100 * (collectionCount - 1) / totalCollections };
   VPinballLib::VPinball::SendEvent(VPinballLib::Event::LoadingCollections, &progressData);
}
