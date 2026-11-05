// license:GPLv3+

#include "core/stdafx.h"
#include "FileDialog.h"

#ifdef __EMSCRIPTEN__

#include <emscripten.h>

namespace
{

struct DialogRequest
{
   SDL_DialogFileCallback callback;
   void *userdata;
};

std::unordered_map<int, DialogRequest> s_requests;
int s_nextRequestId = 0;

void ShowDialog(const int type, SDL_DialogFileCallback callback, void *userdata, const SDL_DialogFileFilter *filters, const int nfilters, const char *defaultLocation, const bool allowMany)
{
   string patterns;
   for (int i = 0; i < nfilters; i++)
   {
      if (i > 0)
         patterns += ';';
      patterns += filters[i].pattern;
   }
   const int id = ++s_nextRequestId;
   s_requests[id] = { callback, userdata };
   MAIN_THREAD_EM_ASM({
      const id = $0;
      const type = $1;
      const patterns = UTF8ToString($2);
      const location = UTF8ToString($3);
      const many = $4;
      Module.vpxFileDialogs = Module.vpxFileDialogs || {};
      const complete = (paths) => {
         if (paths)
            Module.vpxFileDialogs[id] = paths;
         _VPXFileDialogComplete(id, paths ? paths.length : -1);
      };
      if (type == 1) {
         const slash = location.lastIndexOf('/');
         const promptName = () => {
            const name = window.prompt('Save as', location.substring(slash + 1));
            complete(name ? [location.substring(0, slash + 1) + name] : []);
         };
         if (!window.showSaveFilePicker) {
            promptName();
            return;
         }
         const exts = patterns.split(';').filter(p => p.length > 0 && p != '*').map(p => '.' + p);
         const options = { suggestedName: location.substring(slash + 1) };
         if (exts.length > 0)
            options.types = [({ description: 'Visual Pinball Tables', accept: { 'application/octet-stream': exts } })];
         window.showSaveFilePicker(options).then((handle) => {
            const dir = '/tmp/save/' + id;
            FS.mkdirTree(dir);
            const path = dir + '/' + handle.name;
            Module.vpxSaveHandles = Module.vpxSaveHandles || {};
            Module.vpxSaveHandles[path] = handle;
            complete([path]);
         }, (e) => {
            if (e && e.name == 'AbortError')
               complete([]);
            else
               promptName();
         });
         return;
      }
      const input = document.createElement('input');
      input.type = 'file';
      if (type == 2) {
         input.webkitdirectory = true;
      } else {
         input.multiple = !!many;
         const exts = patterns.split(';').filter(p => p.length > 0);
         if (exts.length > 0 && !exts.includes('*'))
            input.accept = exts.map(p => '.' + p).join(',');
      }
      input.style.display = 'none';
      document.body.appendChild(input);
      let done = false;
      const finish = (paths) => {
         if (done)
            return;
         done = true;
         input.remove();
         complete(paths);
      };
      input.addEventListener('cancel', () => finish([]));
      input.addEventListener('change', () => {
         const dir = '/tmp/dialog/' + id;
         const paths = [];
         const pending = Array.from(input.files).map((file) => {
            const rel = file.webkitRelativePath || file.name;
            const path = dir + '/' + rel;
            return file.arrayBuffer().then((buf) => {
               FS.mkdirTree(path.substring(0, path.lastIndexOf('/')));
               FS.writeFile(path, new Uint8Array(buf));
               const entry = type == 2 ? dir + '/' + rel.split('/')[0] : path;
               if (!paths.includes(entry))
                  paths.push(entry);
            });
         });
         Promise.all(pending).then(() => finish(paths), () => finish(null));
      });
      input.click();
   }, id, type, patterns.c_str(), defaultLocation ? defaultLocation : "", allowMany ? 1 : 0);
}

}

extern "C" EMSCRIPTEN_KEEPALIVE void VPXFileDialogComplete(const int id, const int count)
{
   const auto it = s_requests.find(id);
   if (it == s_requests.end())
      return;
   const DialogRequest request = it->second;
   s_requests.erase(it);
   if (count < 0)
   {
      request.callback(request.userdata, nullptr, -1);
      return;
   }
   vector<string> paths(count);
   for (int i = 0; i < count; i++)
   {
      const int len = EM_ASM_INT({ return lengthBytesUTF8(Module.vpxFileDialogs[$0][$1]); }, id, i);
      paths[i].resize(len + 1);
      EM_ASM({ stringToUTF8(Module.vpxFileDialogs[$0][$1], $2, $3); }, id, i, paths[i].data(), len + 1);
      paths[i].resize(len);
   }
   EM_ASM({ delete Module.vpxFileDialogs[$0]; }, id);
   vector<const char *> list;
   for (const string &path : paths)
      list.push_back(path.c_str());
   list.push_back(nullptr);
   request.callback(request.userdata, list.data(), -1);
}

namespace VPX
{

void ShowOpenFileDialog(SDL_DialogFileCallback callback, void *userdata, SDL_Window *window, const SDL_DialogFileFilter *filters, int nfilters, const char *defaultLocation, bool allowMany)
{
   ShowDialog(0, callback, userdata, filters, nfilters, defaultLocation, allowMany);
}

void ShowSaveFileDialog(SDL_DialogFileCallback callback, void *userdata, SDL_Window *window, const SDL_DialogFileFilter *filters, int nfilters, const char *defaultLocation)
{
   ShowDialog(1, callback, userdata, filters, nfilters, defaultLocation, false);
}

void ShowOpenFolderDialog(SDL_DialogFileCallback callback, void *userdata, SDL_Window *window, const char *defaultLocation, bool allowMany)
{
   ShowDialog(2, callback, userdata, nullptr, 0, defaultLocation, allowMany);
}

}

#else

namespace VPX
{

void ShowOpenFileDialog(SDL_DialogFileCallback callback, void *userdata, SDL_Window *window, const SDL_DialogFileFilter *filters, int nfilters, const char *defaultLocation, bool allowMany)
{
   SDL_ShowOpenFileDialog(callback, userdata, window, filters, nfilters, defaultLocation, allowMany);
}

void ShowSaveFileDialog(SDL_DialogFileCallback callback, void *userdata, SDL_Window *window, const SDL_DialogFileFilter *filters, int nfilters, const char *defaultLocation)
{
   SDL_ShowSaveFileDialog(callback, userdata, window, filters, nfilters, defaultLocation);
}

void ShowOpenFolderDialog(SDL_DialogFileCallback callback, void *userdata, SDL_Window *window, const char *defaultLocation, bool allowMany)
{
   SDL_ShowOpenFolderDialog(callback, userdata, window, defaultLocation, allowMany);
}

}

#endif
