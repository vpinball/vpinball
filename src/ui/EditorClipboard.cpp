// license:GPLv3+

#include "core/stdafx.h"
#include "ui/EditorClipboard.h"

#include "core/ieditable.h"
#include "utils/BiffWriter.h"
#include "utils/fileio.h"

#include <SDL3/SDL_clipboard.h>
#include <SDL3/SDL_init.h>

namespace VPX
{

namespace
{
   // Mime types under which the editor payloads are offered to the OS clipboard
   constexpr char partsMimeType[] = "application/x-vpx-parts";
   constexpr char pointMimeType[] = "application/x-vpx-point";

   const void *SDLCALL ClipboardDataCallback(void *userdata, const char *mimeType, size_t *size)
   {
      const vector<uint8_t> *const data = static_cast<const vector<uint8_t> *>(userdata);
      if (mimeType == nullptr) // The clipboard was cleared or replaced
      {
         *size = 0;
         return nullptr;
      }
      *size = data->size();
      return data->data();
   }

   void SDLCALL ClipboardCleanupCallback(void *userdata) { delete static_cast<vector<uint8_t> *>(userdata); }

   // SDL's clipboard is part of the video subsystem which may not be initialized yet (Win32 editor without a running player)
   bool EnsureClipboardAvailable() { return (SDL_WasInit(SDL_INIT_VIDEO) != 0) || SDL_InitSubSystem(SDL_INIT_VIDEO); }

   bool OfferClipboardData(const char *mimeType, vector<uint8_t> &&data)
   {
      if (!EnsureClipboardAvailable())
         return false;
      vector<uint8_t> *const clipboardData = new vector<uint8_t>(std::move(data));
      const char *const mimeTypes[] = { mimeType };
      if (!SDL_SetClipboardData(ClipboardDataCallback, ClipboardCleanupCallback, clipboardData, mimeTypes, 1))
      {
         delete clipboardData;
         return false;
      }
      return true;
   }

   vector<uint8_t> GetClipboardData(const char *mimeType)
   {
      vector<uint8_t> result;
      if (!EnsureClipboardAvailable())
         return result;
      size_t size = 0;
      if (void *const data = SDL_GetClipboardData(mimeType, &size); data != nullptr)
      {
         const uint8_t *const bytes = static_cast<const uint8_t *>(data);
         result.assign(bytes, bytes + size);
         SDL_free(data);
      }
      return result;
   }
}

void EditorClipboard::CopyParts(const vector<IEditable *> &parts)
{
   if (parts.empty())
      return;
   vector<uint8_t> data;
   const auto write = [&data](const void *pv, const size_t size)
   {
      const uint8_t *const bytes = static_cast<const uint8_t *>(pv);
      data.insert(data.end(), bytes, bytes + size);
   };
   const uint32_t count = static_cast<uint32_t>(parts.size());
   write(&count, sizeof(count));
   for (IEditable *const part : parts)
   {
      InMemStream stream;
      const int type = part->GetItemType();
      stream.Write(&type, sizeof(int));
      BiffWriter writer(&stream, nullptr);
      part->Save(writer, false);
      const uint32_t size = static_cast<uint32_t>(stream.Size());
      write(&size, sizeof(size));
      write(stream.Data(), size);
   }
   OfferClipboardData(partsMimeType, std::move(data));
}

bool EditorClipboard::HasParts() { return EnsureClipboardAvailable() && SDL_HasClipboardData(partsMimeType); }

vector<vector<uint8_t>> EditorClipboard::GetParts()
{
   vector<vector<uint8_t>> parts;
   const vector<uint8_t> data = GetClipboardData(partsMimeType);
   size_t pos = 0;
   const auto read = [&data, &pos](void *pv, const size_t size)
   {
      if (pos + size > data.size())
         return false;
      memcpy(pv, data.data() + pos, size);
      pos += size;
      return true;
   };
   uint32_t count = 0;
   if (!read(&count, sizeof(count)))
      return parts;
   for (uint32_t i = 0; i < count; i++)
   {
      uint32_t size = 0;
      if (!read(&size, sizeof(size)) || pos + size > data.size())
         break;
      parts.emplace_back(data.begin() + pos, data.begin() + pos + size);
      pos += size;
   }
   return parts;
}

void EditorClipboard::CopyPoint(const Vertex3Ds &pos)
{
   vector<uint8_t> data(sizeof(Vertex3Ds));
   memcpy(data.data(), &pos, sizeof(Vertex3Ds));
   OfferClipboardData(pointMimeType, std::move(data));
}

bool EditorClipboard::HasPoint() { return EnsureClipboardAvailable() && SDL_HasClipboardData(pointMimeType); }

bool EditorClipboard::GetPoint(Vertex3Ds &pos)
{
   const vector<uint8_t> data = GetClipboardData(pointMimeType);
   if (data.size() != sizeof(Vertex3Ds))
      return false;
   memcpy(&pos, data.data(), sizeof(Vertex3Ds));
   return true;
}

}
