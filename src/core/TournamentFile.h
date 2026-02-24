// license:GPLv3+

#pragma once

namespace VPX::TournamentFile
{
   // Save a tournament file from the global Player instance
   // The file is generated along the table file, with '.txt' extension
   extern void GenerateTournamentFile();

   // Generate an image from a tournament file
   // The image is generated along the tournament txt file, adding a '.png' extension
   extern void GenerateImageFromTournamentFile(PinTable* table, const std::filesystem::path& txtfile);
};
