const CACHE = 'vpinball';
const CACHED_FILES = /\/vpinball\.(js|wasm|data)$/;

self.addEventListener('install', () => self.skipWaiting());
self.addEventListener('activate', (e) => e.waitUntil(self.clients.claim()));

function isolate(response) {
   if (response.status === 0)
      return response;
   const headers = new Headers(response.headers);
   headers.set('Cross-Origin-Opener-Policy', 'same-origin');
   headers.set('Cross-Origin-Embedder-Policy', 'require-corp');
   return new Response(response.body, { status: response.status, statusText: response.statusText, headers });
}

async function fromCache(e) {
   const request = e.request;
   const cache = await caches.open(CACHE);
   const hit = await cache.match(request);
   if (hit) {
      const headers = {};
      if (hit.headers.has('ETag'))
         headers['If-None-Match'] = hit.headers.get('ETag');
      if (hit.headers.has('Last-Modified'))
         headers['If-Modified-Since'] = hit.headers.get('Last-Modified');
      try {
         const response = await fetch(request.url, { headers, cache: 'no-store' });
         if (response.status === 304)
            return hit;
         if (response.ok) {
            e.waitUntil(cache.put(request, response.clone()));
            return response;
         }
      } catch {
      }
      return hit;
   }
   const response = await fetch(request);
   if (response.ok)
      e.waitUntil(cache.put(request, response.clone()));
   return response;
}

self.addEventListener('fetch', (e) => {
   const request = e.request;
   if (request.cache === 'only-if-cached' && request.mode !== 'same-origin')
      return;
   if (request.method === 'GET' && CACHED_FILES.test(new URL(request.url).pathname))
      e.respondWith(fromCache(e).then(isolate));
   else
      e.respondWith(fetch(request).then(isolate));
});
