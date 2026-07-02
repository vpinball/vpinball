// license:GPLv3+

#include "WebServer.h"
#include <cstdio>
#include <string>

using namespace std::string_literals;
using namespace std::string_view_literals;


namespace Inspector
{

extern std::string GetInputStatesJson();
extern std::string GetDeviceStatesJson();

constexpr const char* HEADER_JSON = "Content-Type: application/json\r\n";
constexpr int STATUS_OK = 200;

WebServer::WebServer()
{
   m_run = false;
   m_pThread = nullptr;
   m_treeJson = "[]"sv;
}

WebServer::~WebServer() { Stop(); }

void WebServer::Start(int port)
{
   if (m_run)
   {
      printf("[Inspector] Web server already running\n");
      return;
   }

   std::string bindUrl = "http://0.0.0.0:" + std::to_string(port);

   printf("[Inspector] Starting web server at %s\n", bindUrl.c_str());

   mg_mgr_init(&m_mgr);

   if (mg_http_listen(&m_mgr, bindUrl.c_str(), &WebServer::EventHandler, this))
   {
      m_run = true;
      printf("[Inspector] Web server started\n");

      m_pThread = std::make_unique<std::thread>(
         [this]()
         {
            while (m_run)
               mg_mgr_poll(&m_mgr, 100);

            mg_mgr_free(&m_mgr);
            printf("[Inspector] Web server closed\n");
         });
   }
   else
   {
      printf("[Inspector] Unable to start web server\n");
   }
}

void WebServer::Stop()
{
   if (!m_run)
   {
      return;
   }

   m_run = false;

   if (m_pThread && m_pThread->joinable())
      m_pThread->join();

   m_pThread.reset();
}

void WebServer::UpdateTreeJson(const std::string& json)
{
   std::lock_guard<std::mutex> lock(m_treeMutex);
   m_treeJson = json;
}

void WebServer::EventHandler(struct mg_connection* c, int ev, void* ev_data)
{
   WebServer* webServer = (WebServer*)c->fn_data;

   if (ev == MG_EV_HTTP_MSG)
   {
      struct mg_http_message* hm = (struct mg_http_message*)ev_data;

      if (mg_match(hm->uri, mg_str("/info"), NULL))
         webServer->Info(c, hm);
      else if (mg_match(hm->uri, mg_str("/api/tree"), NULL))
         webServer->ApiTree(c, hm);
      else if (mg_match(hm->uri, mg_str("/api/input_states"), NULL))
         webServer->ApiInputStates(c, hm);
      else if (mg_match(hm->uri, mg_str("/api/device_states"), NULL))
         webServer->ApiDeviceStates(c, hm);
      else if (mg_match(hm->uri, mg_str("/"), NULL))
         webServer->Root(c, hm);
      else
      {
         mg_http_reply(c, 404, "", "Not found\n");
      }
   }
}

void WebServer::Info(struct mg_connection* c, struct mg_http_message* hm)
{
   const char* response = "{\"status\": \"ok\", \"plugin\": \"inspector\"}";
   mg_http_reply(c, STATUS_OK, HEADER_JSON, "%s", response);
}

void WebServer::ApiTree(struct mg_connection* c, struct mg_http_message* hm)
{
   std::string response;
   {
      std::lock_guard<std::mutex> lock(m_treeMutex);
      response = m_treeJson;
   }
   mg_http_reply(c, STATUS_OK, HEADER_JSON, "%s", response.c_str());
}

void WebServer::ApiInputStates(struct mg_connection* c, struct mg_http_message* hm)
{
   std::string response = GetInputStatesJson();
   mg_http_reply(c, STATUS_OK, HEADER_JSON, "%s", response.c_str());
}

void WebServer::ApiDeviceStates(struct mg_connection* c, struct mg_http_message* hm)
{
   std::string response = GetDeviceStatesJson();
   mg_http_reply(c, STATUS_OK, HEADER_JSON, "%s", response.c_str());
}

void WebServer::Root(struct mg_connection* c, struct mg_http_message* hm)
{
   const char* html = R"html(<!DOCTYPE html>
<html>
<head>
    <title>Inspector TreeView</title>
    <style>
        body { font-family: 'Inter', sans-serif; background: #121212; color: #ffffff; padding: 20px; }
        ul { list-style-type: none; }
        .tree-root { padding-left: 0; }
        details > summary { cursor: pointer; padding: 4px; border-radius: 4px; transition: background 0.2s; }
        details > summary:hover { background: #222222; }
        .node-game { color: #4CAF50; font-weight: bold; }
        .node-controller { color: #2196F3; }
        .node-category { color: #FF9800; }
        .node-item { color: #E0E0E0; font-size: 0.9em; margin-left: 20px; padding: 2px 0; font-family: 'Consolas', 'Menlo', 'Courier New', monospace; }
        h1 { font-weight: 300; border-bottom: 1px solid #333; padding-bottom: 10px; }

        .matrix-content { margin: 8px 0 16px 20px; display: flex; gap: 24px; align-items: flex-start; flex-wrap: wrap; margin-top: 8px; }
        .matrix-table { border-collapse: collapse; font-family: 'Consolas', 'Menlo', 'Courier New', monospace; font-size: 0.82em; }
        .matrix-table td { border: 1px solid #333; padding: 5px 7px; min-width: 30px; text-align: center; background: #181818; }
        .matrix-table td.active { background: #14381f; color: #9BE79B; }
        .matrix-table td.inactive { color: #BDBDBD; }
        .matrix-state { display: block; font-weight: bold; }
        .matrix-mapping { display: block; color: #B0BEC5; }
        .matrix-list {
             height: var(--matrix-height, auto);
             max-height: var(--matrix-height, none);
             column-gap: 24px;
             column-fill: auto;
             column-width: 140px;
             overflow: visible;
             width: max-content;
             font-family: 'Consolas', 'Menlo', 'Courier New', monospace;
             font-size: 0.86em;
        }
        .matrix-list-item { break-inside: avoid; color: #E0E0E0; }
        .matrix-list-mapping { color: #B0BEC5; }
        .matrix-list-name { color: #FFFFFF; }
    </style>
</head>
<body>
    <h1>VPX Inspector</h1>
    <div id="tree">Loading...</div>
    <script>
        const MATRIX_COLUMNS = 8;

        function escapeHtml(value) {
            return String(value ?? '')
                .replace(/&/g, '&amp;')
                .replace(/</g, '&lt;')
                .replace(/>/g, '&gt;')
                .replace(/"/g, '&quot;')
                .replace(/'/g, '&#39;');
        }

        function formatMapping(mapping, deviceOnly) {
            const deviceId = (mapping >>> 16) & 0xFFFF;
            const groupId = mapping & 0xFFFF;
            return deviceOnly ? `${deviceId.toString(16).padStart(4, '0')}` : `${groupId.toString(16).padStart(4, '0')}.${deviceId.toString(16).padStart(4, '0')}`;
        }

        function formatBinaryState(state) {
            return state ? '[X]' : '[ ]';
        }

        function formatFloatState(state) {
            return state > 0.5 ? '[X]' : '[ ]';
        }

        function formatState(node) {
            return node.type === 'device' ? formatFloatState(node.state) : formatBinaryState(node.state);
        }

        function isActiveState(itemType, state) {
            return itemType === 'device' ? state > 0.5 : !!state;
        }

        function updateMatrixListHeights() {
            document.querySelectorAll('.matrix-content').forEach(content => {
                const table = content.querySelector('.matrix-table');
                const list = content.querySelector('.matrix-list');
                if (!table || !list)
                    return;
                const height = table.getBoundingClientRect().height;
                list.style.setProperty('--matrix-height', `${height}px`);
            });
        }

        function updateVisualState(el, itemType, state) {
            el.textContent = itemType === 'device' ? formatFloatState(state) : formatBinaryState(state);

            const cell = el.closest('.matrix-cell');
            if (cell) {
                cell.classList.toggle('active', isActiveState(itemType, state));
                cell.classList.toggle('inactive', !isActiveState(itemType, state));
            }
        }

        function itemDataAttributes(node) {
            const kind = node.type === 'device' ? 'device' : 'input';
            return `data-${kind}-id="${node.mapping}" data-${kind}-name="${escapeHtml(node.name)}" data-state-kind="${kind}"`;
        }

        function isMatrixGroup(children) {
            if (!Array.isArray(children) || children.length < 64 || children.length % MATRIX_COLUMNS !== 0) {
                return false;
            }

            return children.every(child => child && (child.type === 'input' || child.type === 'device'));
        }

        function buildStateSpan(node) {
            return `<span class="state-value" ${itemDataAttributes(node)}>${formatState(node)}</span>`;
        }

        function buildLeafNode(node) {
            const mapping = formatMapping(node.mapping, false);
            return `<div class="node-item">${buildStateSpan(node)} ${escapeHtml(node.name)} (Mapping: ${mapping})</div>`;
        }

        function buildMatrixGroup(node) {
            const children = node.children;
            const rows = children.length / MATRIX_COLUMNS;
            let table = '<table class="matrix-table"><tbody>';

            for (let row = 0; row < rows; ++row) {
                table += '<tr>';
                for (let col = 0; col < MATRIX_COLUMNS; ++col) {
                    const child = children[row * MATRIX_COLUMNS + col];
                    const mapping = (child.mapping>>> 16) & 0xFFFF;
                    const activeClass = isActiveState(child.type, child.state) ? 'active' : 'inactive';
                    table += `<td class="matrix-cell ${activeClass}" title="${escapeHtml(child.name)}">
                        <span class="matrix-state">${buildStateSpan(child)}</span>
                        <span class="matrix-mapping">${mapping}</span>
                    </td>`;
                }
                table += '</tr>';
            }
            table += '</tbody></table>';

            let list = '<div class="matrix-list">';
            for (const child of children) {
                list += `<div class="matrix-list-item"><span class="matrix-list-mapping">${(child.mapping>>> 16) & 0xFFFF}</span> - <span class="matrix-list-name">${escapeHtml(child.name)}</span></div>`;
            }
            list += '</div>';

            return `<details open>
                <summary class="node-${escapeHtml(node.type)}">${escapeHtml(node.name)} (${children.length} items, ${rows}x${MATRIX_COLUMNS})</summary>
                <div class="matrix-content">${table}${list}</div>
            </details>`;
        }

        function buildNode(node) {
            if (!node || Object.keys(node).length === 0) return '<em>No active controllers</em>';
            if (Array.isArray(node)) {
                if (node.length === 0) return '<em>No active controllers</em>';
                let html = '';
                for (let child of node) {
                    html += `<li>${buildNode(child)}</li>`;
                }
                return html;
            }

            if (node.type === 'input' || node.type === 'device') {
                return buildLeafNode(node);
            } else if (node.type === 'display' || node.type === 'seg_display') {
                return `<div class="node-item">${escapeHtml(node.name)}</div>`;
            }

            if (isMatrixGroup(node.children)) {
                return buildMatrixGroup(node);
            }

            let html = `<details open><summary class="node-${escapeHtml(node.type)}">${escapeHtml(node.name)} (${node.children.length} items)</summary><ul>`;
            if (node.children) {
                for (let child of node.children) {
                    html += `<li>${buildNode(child)}</li>`;
                }
            }
            html += `</ul></details>`;
            return html;
        }

        function fetchTree() {
            fetch('/api/tree')
                .then(res => res.json())
                .then(data => {
                    document.getElementById('tree').innerHTML = `<ul class="tree-root">${buildNode(data)}</ul>`;
                    updateMatrixListHeights();
                })
                .catch(err => {
                    document.getElementById('tree').innerHTML = `Error loading tree: ${err}`;
                });
        }

        function updateInputState(item) {
            document.querySelectorAll(`[data-input-id="${item.id}"]`).forEach(el => updateVisualState(el, 'input', item.state));
        }

        function updateDeviceState(item) {
            document.querySelectorAll(`[data-device-id="${item.id}"]`).forEach(el => updateVisualState(el, 'device', item.state));
        }

        function fetchInputStates() {
            fetch('/api/input_states')
                .then(res => res.json())
                .then(data => {
                    data.forEach(updateInputState);
                })
                .catch(err => console.error('Input states error', err));
            setTimeout(fetchInputStates, 30);
        }

        function fetchDeviceStates() {
            fetch('/api/device_states')
                .then(res => res.json())
                .then(data => {
                    data.forEach(updateDeviceState);
                })
                .catch(err => console.error('Device states error', err));
            setTimeout(fetchDeviceStates, 30);
        }

        // Initial load
        fetchTree();

        // Keep matrix legends constrained to matrix height when the layout changes
        window.addEventListener('resize', updateMatrixListHeights);

        // Refresh states periodically
        setTimeout(fetchInputStates, 30);
        setTimeout(fetchDeviceStates, 30);
    </script>
</body>
</html>)html";
   mg_http_reply(c, STATUS_OK, "Content-Type: text/html\r\n", "%s", html);
}
} // namespace Inspector
