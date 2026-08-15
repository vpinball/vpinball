// Streams display frames from the inspector web server over WebSocket and paints them onto the
// given canvas. The server pushes one binary message per new frame: a 12 byte header (uint32 LE
// width, height, frameId) followed by top-down RGB24 data. Reconnects automatically; returns a
// handle with close().
function streamDisplay(mapping, canvas, handlers = {}) {
    const ctx = canvas.getContext('2d');
    let image = null;
    let ws = null;
    let retryTimer = null;
    let closed = false;

    function connect() {
        ws = new WebSocket(`${location.protocol === 'https:' ? 'wss' : 'ws'}://${location.host}/ws/display?id=${mapping}`);
        ws.binaryType = 'arraybuffer';
        ws.onmessage = ev => {
            if (!(ev.data instanceof ArrayBuffer) || ev.data.byteLength < 12) return;
            const dv = new DataView(ev.data);
            const w = dv.getUint32(0, true), h = dv.getUint32(4, true);
            if (!w || !h || ev.data.byteLength < 12 + w * h * 3) return;
            if (!image || canvas.width !== w || canvas.height !== h) {
                canvas.width = w;
                canvas.height = h;
                image = ctx.createImageData(w, h);
            }
            const rgb = new Uint8Array(ev.data, 12), px = image.data;
            for (let i = 0, j = 0, k = 0, n = w * h; i < n; i++, j += 3, k += 4) {
                px[k] = rgb[j];
                px[k + 1] = rgb[j + 1];
                px[k + 2] = rgb[j + 2];
                px[k + 3] = 255;
            }
            ctx.putImageData(image, 0, 0);
            handlers.onFrame?.(w, h);
        };
        ws.onclose = () => {
            if (closed) return;
            handlers.onDown?.();
            retryTimer = setTimeout(connect, 3000);
        };
        ws.onerror = () => {};
    }
    connect();

    return {
        close() {
            closed = true;
            clearTimeout(retryTimer);
            ws.onmessage = ws.onclose = ws.onerror = null;
            ws.close();
        }
    };
}
