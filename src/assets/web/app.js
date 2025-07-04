let _editorView = null;
async function createEditor(content, readOnly = true) {
  const container = DOMCache.get("editor-content");

  if (_editorView) {
    _editorView.destroy();
    _editorView = null;
  }

  container.innerHTML = '<div class="monaco-loading">Loading editor...</div>';

  try {
    if (typeof window.createMonacoEditor !== 'function') {
      throw new Error('Monaco editor library not loaded. Check that monaco-editor.js loaded correctly.');
    }

    const fileName = _currentFilePath ? _currentFilePath.split('/').pop() : '';
    _editorView = await createMonacoEditor(container, content, {
      readOnly: readOnly,
      fileName: fileName,
      theme: 'light'
    });

    return _editorView;
  } catch (error) {
    console.error('Failed to create Monaco editor:', error);
    container.innerHTML = `<div class="monaco-error">Failed to load editor. Error: ${error.message}</div>`;
    return null;
  }
}

const SVG_ICONS = {
  folder: '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M22 19a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h5l2 3h9a2 2 0 0 1 2 2z"></path></svg>',
  file: '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z"></path><polyline points="14 2 14 8 20 8"></polyline></svg>',
  code: '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polyline points="16 18 22 12 16 6"></polyline><polyline points="8 6 2 12 8 18"></polyline></svg>',
  settings: '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="3"></circle><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 0 1 0 2.83 2 2 0 0 1-2.83 0l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 0 1-2 2 2 2 0 0 1-2-2v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 0 1-2.83 0 2 2 0 0 1 0-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 0 1-2-2 2 2 0 0 1 2-2h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 0 1 0-2.83 2 2 0 0 1 2.83 0l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 0 1 2-2 2 2 0 0 1 2 2v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 0 1 2.83 0 2 2 0 0 1 0 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 0 1 2 2 2 2 0 0 1-2 2h-.09a1.65 1.65 0 0 0-1.51 1z"></path></svg>',
  web: '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="10"></circle><line x1="2" y1="12" x2="22" y2="12"></line><path d="M12 2a15.3 15.3 0 0 1 4 10 15.3 15.3 0 0 1-4 10 15.3 15.3 0 0 1-4-10 15.3 15.3 0 0 1 4-10z"></path></svg>',
  zip: '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M21 8v8a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2V8a2 2 0 0 1 2-2h14a2 2 0 0 1 2 2z"></path><line x1="16" y1="13" x2="8" y2="13"></line><line x1="16" y1="17" x2="8" y2="17"></line><polyline points="10 9 9 9 8 9"></polyline></svg>',
  threeDots: '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="1"></circle><circle cx="12" cy="5" r="1"></circle><circle cx="12" cy="19" r="1"></circle></svg>',
  font: '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polyline points="4 7 4 4 20 4 20 7"></polyline><line x1="9" y1="20" x2="15" y2="20"></line><line x1="12" y1="4" x2="12" y2="20"></line></svg>',
  game: '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"  viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><line x1="6" y1="11" x2="10" y2="11"></line><line x1="8" y1="9" x2="8" y2="13"></line><rect x="14" y="9" width="4" height="4"></rect><circle cx="19" cy="12" r="1"></circle><rect x="3" y="6" width="18" height="12" rx="2" ry="2"></rect></svg>',
  edit: '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M11 4H4a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2v-7"></path><path d="M18.5 2.5a2.121 2.121 0 0 1 3 3L12 15l-4 1 1-4 9.5-9.5z"></path></svg>',
  delete: '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polyline points="3 6 5 6 21 6"></polyline><path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"></path><line x1="10" y1="11" x2="10" y2="17"></line><line x1="14" y1="11" x2="14" y2="17"></line></svg>',
  extract: '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M14 3v4a1 1 0 0 0 1 1h4"></path><path d="M17 21h-10a2 2 0 0 1 -2 -2v-14a2 2 0 0 1 2 -2h7l5 5v11a2 2 0 0 1 -2 2z"></path><path d="M12 10v6"></path><path d="M9.5 12.5l2.5 -2.5l2.5 2.5"></path><path d="M9.5 16.5l2.5 2.5l2.5 -2.5"></path></svg>',
  sortAsc: '<svg xmlns="http://www.w3.org/2000/svg" width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polyline points="18 15 12 9 6 15"></polyline></svg>',
  sortDesc: '<svg xmlns="http://www.w3.org/2000/svg" width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><polyline points="6 9 12 15 18 9"></polyline></svg>',
  threeDots: '<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="1"></circle><circle cx="12" cy="5" r="1"></circle><circle cx="12" cy="19" r="1"></circle></svg>',
  rename: '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z"></path><path d="M14 2v6h6"></path><path d="M10 13h4"></path><path d="M10 17h4"></path></svg>',
  download: '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"></path><polyline points="7 10 12 15 17 10"></polyline><line x1="12" y1="15" x2="12" y2="3"></line></svg>',
  logStream: '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><line x1="3" y1="6" x2="21" y2="6"></line><line x1="3" y1="12" x2="21" y2="12"></line><line x1="3" y1="18" x2="21" y2="18"></line></svg>'
};

const AppState = {
  directory: null,
  data: null,
  tables: null,
  statusData: null,
  infoData: null,
  connectionFailed: false,
  isNavigating: false,
  lastUpdateTimestamp: null
};

const SortState = {
  lastSort: "name",
  orders: {
    name: "asc",
    size: "asc", 
    date: "desc"
  }
};

const EditorState = {
  currentFilePath: null,
  isViewingFile: false,
  isReadOnly: true,
  originalContent: null,
  hasChanges: false,
  view: null
};

const LogState = {
  eventSource: null,
  entries: [],
  entryCount: 0,
  maxEntries: 1000
};

const UIState = {
  statusWs: null,
  currentContextMenu: null
};

const DOMCache = {
  _cache: new Map(),

  get(id) {
    if (!this._cache.has(id)) {
      this._cache.set(id, document.getElementById(id));
    }
    return this._cache.get(id);
  },

  querySelector(selector) {
    if (!this._cache.has(selector)) {
      this._cache.set(selector, document.querySelector(selector));
    }
    return this._cache.get(selector);
  },

  querySelectorAll(selector) {
    return document.querySelectorAll(selector);
  },

  clear() {
    this._cache.clear();
  },

  remove(id) {
    this._cache.delete(id);
  }
};

const EventManager = {
  _listeners: new Map(),

  addEventListener(element, event, handler, options) {
    if (!this._listeners.has(element)) {
      this._listeners.set(element, new Map());
    }

    const elementListeners = this._listeners.get(element);
    if (!elementListeners.has(event)) {
      elementListeners.set(event, new Set());
    }

    elementListeners.get(event).add({ handler, options });
    element.addEventListener(event, handler, options);
  },

  removeEventListener(element, event, handler) {
    if (this._listeners.has(element)) {
      const elementListeners = this._listeners.get(element);
      if (elementListeners.has(event)) {
        const eventHandlers = elementListeners.get(event);
        for (const item of eventHandlers) {
          if (item.handler === handler) {
            eventHandlers.delete(item);
            element.removeEventListener(event, handler);
            break;
          }
        }

        if (eventHandlers.size === 0) {
          elementListeners.delete(event);
        }
      }

      if (elementListeners.size === 0) {
        this._listeners.delete(element);
      }
    }
  },

  removeAllListeners(element) {
    if (this._listeners.has(element)) {
      const elementListeners = this._listeners.get(element);
      for (const [event, handlers] of elementListeners) {
        for (const item of handlers) {
          element.removeEventListener(event, item.handler);
        }
      }
      this._listeners.delete(element);
    }
  },

  cleanupRemovedElements() {
    for (const [element] of this._listeners) {
      if (element instanceof Node && !document.contains(element)) {
        this.removeAllListeners(element);
      }
    }
  }
};

var _directory = AppState.directory;
var _data = AppState.data;
var _lastSort = SortState.lastSort;
var _sortOrders = SortState.orders;
var _statusData = AppState.statusData;
var _infoData = AppState.infoData;
var _statusWs = UIState.statusWs;
var _connectionFailed = AppState.connectionFailed;
var _currentFilePath = EditorState.currentFilePath;
var _isReadOnly = EditorState.isReadOnly;
var _originalContent = EditorState.originalContent;
var _hasChanges = EditorState.hasChanges;
var _logEventSource = LogState.eventSource;
var _logEntries = LogState.entries;
var _logEntryCount = LogState.entryCount;
var _maxLogEntries = LogState.maxEntries;
var _tables = AppState.tables;
var _currentContextMenu = UIState.currentContextMenu;
var _isNavigating = AppState.isNavigating;
var _isViewingFile = EditorState.isViewingFile;
var _lastUpdateTimestamp = AppState.lastUpdateTimestamp;

function downloadFile(fileName, filePath) {
  const link = document.createElement('a');
  link.href = `download?q=${encodeURIComponent(filePath || fileName)}`;
  link.download = fileName || '';
  link.style.display = 'none';
  link.target = '_self';
  document.body.appendChild(link);
  link.click();
  document.body.removeChild(link);
}


function fetchTableList() {
  return fetch('/table-list')
    .then(response => response.json())
    .then(data => {
      _tables = {};
      data.forEach(table => {
        _tables[table.table] = table.name;
      });
      return _tables;
    })
    .catch(error => {
      console.error('Error fetching tables:', error);
      _tables = {};
      return _tables;
    });
}

function fetchInfo() {
  return fetch('/info')
    .then(response => response.json())
    .then(data => {
      _infoData = data;
      updateStatusDisplay();
      return data;
    })
    .catch(error => {
      console.error('Error fetching info:', error);
      throw error;
    });
}

function connectStatusWebSocket() {
  if (_statusWs) {
    _statusWs.close();
  }

  const wsProtocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
  const wsUrl = `${wsProtocol}//${window.location.host}/status`;

  _statusWs = new WebSocket(wsUrl);

  _statusWs.onopen = () => {
    _connectionFailed = false;
  };

  _statusWs.onmessage = (event) => {
    try {
      const newStatusData = JSON.parse(event.data);

      if (_statusData && newStatusData.lastUpdate && 
          _statusData.lastUpdate !== newStatusData.lastUpdate) {

        fetchTableList();

        navigateToPath(_directory || '');
      }

      _statusData = newStatusData;
      updateStatusDisplay();
    } catch (error) {
      console.error('Error parsing status WebSocket message:', error);
    }
  };

  _statusWs.onerror = (error) => {
    console.error('Status WebSocket error:', error);
    _connectionFailed = true;
    clearStatusDisplay();
  };

  _statusWs.onclose = () => {
    _connectionFailed = true;
    clearStatusDisplay();

    setTimeout(() => {
      if (!_statusWs || _statusWs.readyState === WebSocket.CLOSED) {
        connectStatusWebSocket();
      }
    }, 5000);
  };
}

function clearStatusDisplay() {
  if (_infoData) {
    DOMCache.get('version-info').textContent = _infoData.version;
  } else {
    DOMCache.get('version-info').textContent = '';
  }
  DOMCache.get('current-table').textContent = 'Connection failed';
  DOMCache.get('header-actions').innerHTML = '';
}

function updateStatusDisplay() {
  if (_infoData) {
    DOMCache.get('version-info').textContent = _infoData.version;
  }

  if (!_statusData) return;

  if (_statusData.running && _statusData.currentTable) {
    const tablePath = _statusData.currentTable;
    const pathParts = tablePath.split('/');
    if (pathParts.length >= 2) {
      const uuid = pathParts[pathParts.length - 2];
      const filename = pathParts[pathParts.length - 1];
      DOMCache.get('current-table').innerHTML = `<span style="word-break: break-all;">${uuid}</span>/<span style="white-space: nowrap;">${filename}</span>`;
    } else {
      DOMCache.get('current-table').textContent = tablePath;
    }
  } else {
    DOMCache.get('current-table').textContent = 'No table running';
  }

  const headerActions = DOMCache.get('header-actions');
  headerActions.innerHTML = '';

  if (_statusData.running) {
    const fpsButton = document.createElement('button');
    fpsButton.className = 'btn btn-primary';
    fpsButton.onclick = () => sendCommand('fps');
    fpsButton.innerHTML = `
      <span class="btn-icon">
        <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
          <path d="M13 2L3 14h9l-1 8 10-12h-9l1-8z"></path>
        </svg>
      </span>
      Toggle FPS
    `;
    headerActions.appendChild(fpsButton);

    const exitButton = document.createElement('button');
    exitButton.className = 'btn btn-danger';
    exitButton.onclick = () => sendCommand('shutdown');
    exitButton.innerHTML = `
      <span class="btn-icon">
        <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
          <path d="M18.36 6.64a9 9 0 1 1-12.73 0"></path>
          <line x1="12" y1="2" x2="12" y2="12"></line>
        </svg>
      </span>
      Exit Table
    `;
    headerActions.appendChild(exitButton);
  }
}

function fetchFiles(directory) {
  const filesPromise = fetch(`files?q=${encodeURIComponent(directory)}`)
    .then((response) => response.json());

  const shouldFetchTables = !directory;
  const tablesPromise = shouldFetchTables ? fetchTableList() : Promise.resolve(_tables || {});

  return Promise.all([filesPromise, tablesPromise])
    .then(([data, tables]) => {
      _directory = directory;
      _data = data;
      _tables = tables;

      sortFiles(_lastSort);
      updateSortIndicators();
      updateFilesList();

      return data;
    })
    .catch((error) => {
      console.error("Error:", error);
      throw error;
    });
}

function createFileListItem(file, subdirectory, hasTable) {
  const listItem = document.createElement("li");
  listItem.className = "file-item";
  listItem.style.cursor = "pointer";

  if (file.isDir) {
    listItem.onclick = (e) => {
      if (e.target.tagName !== 'BUTTON' && !e.target.closest('.action-icon')) {
        navigateToPath(subdirectory);
      }
    };
  } else {
    const isViewable = FileTypeHelper.isViewable(file.ext);
    const isImage = FileTypeHelper.isImage(file.ext);

    listItem.onclick = (e) => {
      if (e.target.tagName !== 'BUTTON' && !e.target.closest('.action-icon')) {
        if (isViewable) {
          viewFile(file.name);
        } else if (isImage) {
          viewImage(file.name);
        }
      }
    };
  }

  listItem.oncontextmenu = (e) => {
    e.preventDefault();
    showContextMenu(e, file.name, file.ext, file.isDir, hasTable);
  };

  const nameDiv = document.createElement("div");
  nameDiv.className = "file-name";

  const mainRowDiv = document.createElement("div");

  const icon = document.createElement("span");
  icon.className = "file-icon";
  icon.innerHTML = getFileIcon(file.ext, file.isDir);
  mainRowDiv.appendChild(icon);

  const link = document.createElement("a");
  link.href = "#";
  link.textContent = file.name;

  if (file.isDir) {
    link.onclick = (e) => {
      e.stopPropagation();
      navigateToPath(subdirectory);
      return false;
    };
  } else {
    const isViewable = FileTypeHelper.isViewable(file.ext);
    const isImage = FileTypeHelper.isImage(file.ext);

    if (isViewable) {
      link.onclick = (e) => {
        e.stopPropagation();
        viewFile(file.name);
        return false;
      };
    } else if (isImage) {
      link.onclick = (e) => {
        e.stopPropagation();
        viewImage(file.name);
        return false;
      };
    } else {
      link.onclick = (e) => {
        e.preventDefault();
        return false;
      };
    }
  }

  mainRowDiv.appendChild(link);
  nameDiv.appendChild(mainRowDiv);

  if (hasTable) {
    const tableName = _tables[file.name];
    const tableNameSpan = document.createElement("div");
    tableNameSpan.className = "game-name";
    tableNameSpan.textContent = tableName;
    nameDiv.appendChild(tableNameSpan);
  }

  listItem.appendChild(nameDiv);

  const dateDiv = document.createElement("div");
  dateDiv.className = "file-date";
  dateDiv.textContent = formatDate(file.date);
  listItem.appendChild(dateDiv);

  const sizeDiv = document.createElement("div");
  sizeDiv.className = "file-size";
  sizeDiv.textContent = formatFileSize(file.size);
  listItem.appendChild(sizeDiv);

  const actionsDiv = document.createElement("div");
  actionsDiv.className = "file-actions";

  const menuIcon = document.createElement("span");
  menuIcon.className = "menu-icon";
  menuIcon.innerHTML = SVG_ICONS.threeDots;
  menuIcon.title = "More actions";
  menuIcon.onclick = (e) => {
    e.stopPropagation();
    showContextMenu(e, file.name, file.ext, file.isDir, hasTable);
  };
  actionsDiv.appendChild(menuIcon);
  listItem.appendChild(actionsDiv);

  return listItem;
}

function createParentDirItem(parentDirectory) {
  const listItem = document.createElement("li");
  listItem.className = "file-item";
  listItem.style.cursor = "pointer";
  listItem.onclick = (e) => {
    if (e.target.tagName !== 'BUTTON' && !e.target.closest('.action-icon')) {
      navigateToPath(parentDirectory);
    }
  };

  const nameDiv = document.createElement("div");
  nameDiv.className = "file-name";
  const mainRowDiv = document.createElement("div");

  const folderIcon = document.createElement("span");
  folderIcon.className = "file-icon";
  folderIcon.innerHTML = SVG_ICONS.folder;
  mainRowDiv.appendChild(folderIcon);

  const link = document.createElement("a");
  link.href = "#";
  link.textContent = "..";
  link.onclick = (e) => {
    e.stopPropagation();
    navigateToPath(parentDirectory);
    return false;
  };
  mainRowDiv.appendChild(link);
  nameDiv.appendChild(mainRowDiv);
  listItem.appendChild(nameDiv);

  listItem.appendChild(document.createElement("div"));
  listItem.appendChild(document.createElement("div"));
  listItem.appendChild(document.createElement("div"));

  return listItem;
}

function updateFilesList() {
  const fileList = DOMCache.get("file-list");

  const existingItems = fileList.querySelectorAll('.file-item');
  existingItems.forEach(item => EventManager.removeAllListeners(item));

  const fragment = document.createDocumentFragment();

  if (_directory !== "") {
    const parentDirectory = _directory.substring(0, _directory.lastIndexOf("/")).replace("'", "\\'");
    fragment.appendChild(createParentDirItem(parentDirectory));
  }

  _data.forEach((file) => {
    const subdirectory = file.isDir ? (_directory ? `${_directory}/${file.name}` : file.name).replace("'", "\\'") : '';
    const hasTable = !_directory && _tables && _tables[file.name];
    fragment.appendChild(createFileListItem(file, subdirectory, hasTable));
  });

  fileList.innerHTML = "";
  fileList.appendChild(fragment);

  EventManager.cleanupRemovedElements();
}

function formatDate(dateString) {
  if (!dateString) return "";

  const date = new Date(dateString);
  if (isNaN(date.getTime())) return "";

  return date.toLocaleDateString() + ' ' + date.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' });
}

const FileTypeHelper = {
  TEXT_EXTENSIONS: new Set(['vbs', 'ini', 'txt', 'xml']),
  LOG_EXTENSIONS: new Set(['log']),
  IMAGE_EXTENSIONS: new Set(['jpg', 'jpeg', 'png', 'gif', 'bmp', 'webp', 'svg']),
  VIEWABLE_EXTENSIONS: new Set(['vbs', 'ini', 'txt', 'log', 'html', 'json', 'xml', 'fnt']),
  EXTRACTABLE_EXTENSIONS: new Set(['zip']),

  isText: (ext) => FileTypeHelper.TEXT_EXTENSIONS.has(ext),
  isLog: (ext) => FileTypeHelper.LOG_EXTENSIONS.has(ext),
  isImage: (ext) => FileTypeHelper.IMAGE_EXTENSIONS.has(ext),
  isViewable: (ext) => FileTypeHelper.VIEWABLE_EXTENSIONS.has(ext),
  isEditable: (ext) => FileTypeHelper.TEXT_EXTENSIONS.has(ext),
  isExtractable: (ext) => FileTypeHelper.EXTRACTABLE_EXTENSIONS.has(ext),
  isReadOnly: (ext) => FileTypeHelper.LOG_EXTENSIONS.has(ext),

  getIcon: (ext, isDir) => {
    if (isDir) return SVG_ICONS.folder;

    const iconMap = {
      txt: SVG_ICONS.file, log: SVG_ICONS.file, ini: SVG_ICONS.settings,
      json: SVG_ICONS.code, xml: SVG_ICONS.code, vbs: SVG_ICONS.code,
      html: SVG_ICONS.web, zip: SVG_ICONS.zip, fnt: SVG_ICONS.font,
      glfx: SVG_ICONS.game, fxh: SVG_ICONS.game,
    };
    return iconMap[ext] || SVG_ICONS.file;
  }
};

function getFileIcon(ext, isDir) {
  return FileTypeHelper.getIcon(ext, isDir);
}

function updateSortIndicators() {
  DOMCache.get("name-sort-indicator").innerHTML = "";
  DOMCache.get("size-sort-indicator").innerHTML = "";
  DOMCache.get("date-sort-indicator").innerHTML = "";

  if (_lastSort === "name") {
    DOMCache.get("name-sort-indicator").innerHTML = _sortOrders[_lastSort] === "asc" ? SVG_ICONS.sortAsc : SVG_ICONS.sortDesc;
  } else if (_lastSort === "size") {
    DOMCache.get("size-sort-indicator").innerHTML = _sortOrders[_lastSort] === "asc" ? SVG_ICONS.sortAsc : SVG_ICONS.sortDesc;
  } else if (_lastSort === "date") {
    DOMCache.get("date-sort-indicator").innerHTML = _sortOrders[_lastSort] === "asc" ? SVG_ICONS.sortAsc : SVG_ICONS.sortDesc;
  }
}

function toggleSort(column) {
  if (_lastSort === column) {
    _sortOrders[column] = _sortOrders[column] === "asc" ? "desc" : "asc";
  } else {
    _lastSort = column;
  }

  sortFiles(column);
  updateSortIndicators();
}

function sortFiles(column) {
  _data.sort((a, b) => {
    if (a.isDir && !b.isDir) return -1;
    if (!a.isDir && b.isDir) return 1;

    if (column === "name") {
      if (_sortOrders[column] === "asc") {
        return a.name.localeCompare(b.name);
      } else {
        return b.name.localeCompare(a.name);
      }
    } else if (column === "size") {
      if (_sortOrders[column] === "asc") {
        return a.size - b.size;
      } else {
        return b.size - a.size;
      }
    } else if (column === "date") {
      const dateA = a.date ? new Date(a.date).getTime() : 0;
      const dateB = b.date ? new Date(b.date).getTime() : 0;

      if (_sortOrders[column] === "asc") {
        return dateA - dateB;
      } else {
        return dateB - dateA;
      }
    }
  });

  _lastSort = column;

  updateFilesList();
}

const SearchUtils = {
  debounceTimer: null,

  debounce(func, delay = 300) {
    clearTimeout(this.debounceTimer);
    this.debounceTimer = setTimeout(func, delay);
  }
};

function filterFiles() {
  SearchUtils.debounce(() => {
    const searchTerm = DOMCache.get("search-input").value.toLowerCase();
    const fileItems = DOMCache.querySelectorAll(".file-item");

    fileItems.forEach((item) => {
      const fileName = item.querySelector(".file-name a").textContent.toLowerCase();
      if (fileName === "..") {
        item.style.display = "grid";
        return;
      }

      let shouldShow = fileName.includes(searchTerm);

      if (!shouldShow && !_directory) {
        const tableNameElement = item.querySelector(".game-name");
        if (tableNameElement) {
          const tableName = tableNameElement.textContent.toLowerCase();
          shouldShow = tableName.includes(searchTerm);
        }
      }

      item.style.display = shouldShow ? "grid" : "none";
    });
  });
}

function formatFileSize(size) {
  if (size === 0) return "0 Bytes";
  const units = ["Bytes", "KB", "MB", "GB", "TB"];
  const i = Math.floor(Math.log(size) / Math.log(1024));

  if (i === 0) {
    return `${size} ${units[i]}`;
  }

  return `${(size / Math.pow(1024, i)).toFixed(2)} ${units[i]}`;
}

function newFolder() {
  var folderName = prompt("Enter the folder name:");
  if (folderName !== null && folderName.trim() !== "") {
    folderName = folderName.trim();
    const filePath = _directory ? `${_directory}/${folderName}` : folderName;
    fetch(`folder?q=${encodeURIComponent(filePath)}`, { method: "POST" })
      .then((response) => {
        if (response.ok) {
          showStatusMessage("main-status", "Folder created successfully!", "success");
          navigateToPath(_directory);
        } else {
          showStatusMessage("main-status", "Failed to create folder", "error");
        }
      })
      .catch((error) => {
        console.error("Error:", error);
        showStatusMessage("main-status", "Error creating folder", "error");
      });
  }
}

function uploadFile() {
  const fileInput = document.createElement("input");
  fileInput.type = "file";

  fileInput.addEventListener("change", () => {
    if (fileInput.files.length > 0) {
      var file = fileInput.files[0];
      var reader = new FileReader();
      reader.readAsArrayBuffer(file);
      reader.onload = () => {
        const filePath = _directory ? `${_directory}/${file.name}` : file.name;
        const data = new Uint8Array(reader.result);
        sendChunk(filePath, data, 0, 1024 * 512, "main-status", () => {
          navigateToPath(_directory);
        });
      };
    }
  });

  fileInput.click();
}

function openFile(fileName, editing = false) {
  const filePath = _directory ? `${_directory}/${fileName}` : fileName;
  _currentFilePath = filePath;

  if (!_isNavigating) {
    const currentPath = _directory || '';
    const historyState = {
      type: 'file',
      directory: currentPath,
      fileName: fileName,
      filePath: filePath
    };
    if (editing) historyState.editing = true;

    history.pushState(historyState, '', `#${currentPath ? encodeURIComponent(currentPath) + '/' : ''}${encodeURIComponent(fileName)}`);
  }

  fetch(`download?q=${encodeURIComponent(filePath)}`)
    .then((response) => {
      if (response.ok) {
        return response.text();
      } else {
        throw new Error("Failed to load file");
      }
    })
    .then(async (text) => {
      updateFileBreadcrumb(_directory || '', fileName, 'editor-breadcrumb');

      _originalContent = text;
      _hasChanges = false;
      _isReadOnly = false;

      try {
        const fileExt = fileName.split('.').pop()?.toLowerCase();
        const isReadOnlyFile = FileTypeHelper.isReadOnly(fileExt);

        await createEditor(text, isReadOnlyFile);

        updateEditorMenu();

        DOMCache.get("main").style.display = "none";
        DOMCache.get("editor").style.display = "flex";
        _isViewingFile = true;

        if (_editorView) {
          _editorView.focus();
        }
      } catch (editorError) {
        console.error("Failed to create editor:", editorError);
        showStatusMessage("main-status", "Failed to load editor. Check internet connection.", "error");
        const container = DOMCache.get("editor-content");
        container.innerHTML = `<pre style="padding: 16px; white-space: pre-wrap; font-family: monospace;">${text.replace(/</g, '&lt;').replace(/>/g, '&gt;')}</pre>`;

        DOMCache.get("main").style.display = "none";
        DOMCache.get("editor").style.display = "flex";
        _isViewingFile = true;
      }
    })
    .catch((error) => {
      console.error("Error:", error);
      showStatusMessage("main-status", "Error loading file", "error");
    });
}

function viewFile(fileName) {
  openFile(fileName, false);
}

function editFile(fileName) {
  openFile(fileName, true);
}

function viewImage(fileName) {
  const filePath = _directory ? `${_directory}/${fileName}` : fileName;
  _currentFilePath = filePath;

  if (!_isNavigating) {
    const currentPath = _directory || '';
    history.pushState({
      type: 'image',
      directory: currentPath,
      fileName: fileName,
      filePath: filePath
    }, '', `#${currentPath ? encodeURIComponent(currentPath) + '/' : ''}${encodeURIComponent(fileName)}`);
  }

  updateFileBreadcrumb(_directory || '', fileName, 'image-breadcrumb');
  const imageElement = DOMCache.get("image-display");
  imageElement.style.opacity = "0.5";
  imageElement.src = `download?q=${encodeURIComponent(filePath)}`;

  imageElement.onload = () => {
    imageElement.style.opacity = "1";
  };

  imageElement.onerror = () => {
    imageElement.style.opacity = "1";
    imageElement.alt = "Failed to load image";
  };

  window.currentImageFile = { fileName, filePath };

  DOMCache.get("main").style.display = "none";
  DOMCache.get("image-viewer").style.display = "flex";
  _isViewingFile = true;
}

function closeImageViewer() {
  DOMCache.get("main").style.display = "flex";
  DOMCache.get("image-viewer").style.display = "none";
  _currentFilePath = null;
  _isViewingFile = false;

  if (history.length > 1) {
    history.back();
  } else {
    navigateToPath(_directory);
  }
}

function closeEditor() {
  confirmUnsavedChanges(() => {
    if (_editorView) {
      _editorView.destroy();
      _editorView = null;
    }

    DOMCache.get("main").style.display = "flex";
    DOMCache.get("editor").style.display = "none";
    _currentFilePath = null;
    _isViewingFile = false;
    _hasChanges = false;

    if (history.length > 1) {
      _isNavigating = true;
      setTimeout(() => {
        history.back();
        _isNavigating = false;
      }, 0);
    } else {
      navigateToPath(_directory);
    }
  });
}

const FileOperations = {
  async performOperation(operation, fileName, options = {}) {
    const filePath = _directory ? `${_directory}/${fileName}` : fileName;

    const operations = {
      delete: {
        confirm: `Are you sure you want to delete ${fileName}?`,
        url: `delete?q=${encodeURIComponent(filePath)}`,
        method: 'DELETE',
        loadingMsg: 'Deleting file...',
        successMsg: 'File deleted successfully!',
        errorMsg: 'Failed to delete file'
      },
      extract: {
        url: `extract?q=${encodeURIComponent(filePath)}`,
        method: 'POST',
        loadingMsg: 'Extracting file...',
        successMsg: 'File extracted successfully!',
        errorMsg: 'Failed to extract file'
      },
      rename: {
        url: `rename?q=${encodeURIComponent(filePath)}&name=${encodeURIComponent(options.newName)}`,
        method: 'POST',
        loadingMsg: `Renaming ${fileName}...`,
        successMsg: 'File renamed successfully!',
        errorMsg: 'Failed to rename file',
        statusCodes: {
          409: 'File already exists',
          404: 'File not found'
        }
      }
    };

    const config = operations[operation];
    if (!config) throw new Error(`Unknown operation: ${operation}`);

    if (config.confirm && !confirm(config.confirm)) return;

    if (config.loadingMsg) {
      showStatusMessage("main-status", config.loadingMsg, "info");
    }

    try {
      const response = await fetch(config.url, { method: config.method });

      if (response.ok) {
        showStatusMessage("main-status", config.successMsg, "success");
        navigateToPath(_directory);
      } else {
        const errorMsg = config.statusCodes?.[response.status] || config.errorMsg;
        showStatusMessage("main-status", errorMsg, "error");
      }
    } catch (error) {
      console.error("Error:", error);
      showStatusMessage("main-status", `Error during ${operation}`, "error");
    }
  }
};

function deleteFile(fileName) {
  FileOperations.performOperation('delete', fileName);
}

function extractFile(fileName) {
  FileOperations.performOperation('extract', fileName);
}

function renameFile(fileName) {
  const newName = prompt(`Rename ${fileName} to:`, fileName);
  if (newName && newName !== fileName) {
    FileOperations.performOperation('rename', fileName, { newName });
  }
}

function renameTable(tableUuid) {
  const currentTableName = _tables && _tables[tableUuid] ? _tables[tableUuid] : tableUuid;
  const newName = prompt(`Rename table "${currentTableName}" to:`, currentTableName);
  if (newName && newName !== currentTableName) {
    showStatusMessage("main-status", `Renaming table to "${newName}"...`, "info");

    fetch(`table-rename?table=${encodeURIComponent(tableUuid)}&name=${encodeURIComponent(newName)}`, { method: "POST" })
      .then((response) => {
        if (response.ok) {
          showStatusMessage("main-status", "Table renamed successfully!", "success");
          navigateToPath(_directory);
        } else if (response.status === 409) {
          showStatusMessage("main-status", "Table name already exists", "error");
        } else if (response.status === 404) {
          showStatusMessage("main-status", "Table not found", "error");
        } else {
          showStatusMessage("main-status", "Failed to rename table", "error");
        }
      })
      .catch((error) => {
        console.error("Error:", error);
        showStatusMessage("main-status", "Error renaming table", "error");
      });
  }
}

function showContextMenu(event, fileName, fileExt, isDirectory = false, hasTable = false) {
  if (_currentContextMenu) {
    _currentContextMenu.remove();
    _currentContextMenu = null;
  }

  const menu = document.createElement('div');
  menu.className = 'context-menu';
  menu.style.position = 'fixed';
  menu.style.left = event.clientX + 'px';
  menu.style.top = event.clientY + 'px';
  menu.style.zIndex = '1000';

  const menuItems = [];

  if (!isDirectory && FileTypeHelper.isViewable(fileExt)) {
    const isEditable = FileTypeHelper.isEditable(fileExt);
    menuItems.push({
      icon: isEditable ? SVG_ICONS.edit : SVG_ICONS.logStream,
      text: isEditable ? 'Edit' : 'View',
      action: () => editFile(fileName)
    });
  }

  if (!isDirectory && FileTypeHelper.isExtractable(fileExt)) {
    menuItems.push({
      icon: SVG_ICONS.extract,
      text: 'Extract',
      action: () => extractFile(fileName)
    });
  }

  if (!isDirectory) {
    menuItems.push({
      icon: SVG_ICONS.download,
      text: 'Download',
      action: () => {
        const filePath = _directory ? `${_directory}/${fileName}` : fileName;
        downloadFile(fileName, filePath);
      }
    });
  }

  if (isDirectory && !_directory && hasTable) {
    menuItems.push({
      icon: SVG_ICONS.download,
      text: 'Export',
      action: () => exportTableByName(fileName)
    });
  }

  const isTableFolder = !_directory && _tables && _tables[fileName];
  if (isTableFolder) {
    menuItems.push({
      icon: SVG_ICONS.rename,
      text: 'Rename',
      action: () => renameTable(fileName)
    });
  } else {
    menuItems.push({
      icon: SVG_ICONS.rename,
      text: 'Rename',
      action: () => renameFile(fileName)
    });
  }

  menuItems.push({
    icon: SVG_ICONS.delete,
    text: 'Delete',
    action: () => deleteFile(fileName)
  });

  menuItems.forEach(item => {
    const menuItem = document.createElement('div');
    menuItem.className = 'context-menu-item';
    menuItem.innerHTML = `
      <span class="context-menu-icon">${item.icon}</span>
      <span class="context-menu-text">${item.text}</span>
    `;
    menuItem.onclick = () => {
      menu.remove();
      _currentContextMenu = null;
      item.action();
    };
    menu.appendChild(menuItem);
  });

  document.body.appendChild(menu);
  _currentContextMenu = menu;

  const rect = menu.getBoundingClientRect();
  if (rect.right > window.innerWidth) {
    menu.style.left = (event.clientX - rect.width) + 'px';
  }
  if (rect.bottom > window.innerHeight) {
    menu.style.top = (event.clientY - rect.height) + 'px';
  }

  setTimeout(() => {
    document.addEventListener('click', closeContextMenu, { once: true });
  }, 0);
}

function closeContextMenu() {
  if (_currentContextMenu) {
    _currentContextMenu.remove();
    _currentContextMenu = null;
  }
}

function sendCommand(cmd) {
  if (cmd == "shutdown") {
    if (confirm(`Are you sure you want to exit the current table?`)) {
      fetch(`command?cmd=${cmd}`, { method: "POST" })
        .then((response) => {
          if (response.ok) {
            showStatusMessage("main-status", "Exit table command sent", "info");
            setTimeout(fetchStatus, 1000); 
          }
        })
        .catch((error) => console.error("Error:", error));
    }
  } else {
    fetch(`command?cmd=${cmd}`, { method: "POST" })
      .then((response) => {
        if (response.ok) {
          showStatusMessage("main-status", `${cmd.toUpperCase()} command sent`, "info");
        }
      })
      .catch((error) => console.error("Error:", error));
  }
}

function showStatusMessage(elementId, message, type) {
  const statusElement = DOMCache.get(elementId);
  statusElement.textContent = message;

  statusElement.className = "status-message";
  if (type === "error") {
    statusElement.style.color = "var(--danger-color)";
  } else if (type === "success") {
    statusElement.style.color = "var(--success-color)";
  } else {
    statusElement.style.color = "var(--text-secondary)";
  }

  setTimeout(() => {
    statusElement.textContent = "";
  }, 3000);
}

function sendChunk(filePath, data, offset, chunkSize, statusId, callback) {
  const progressPercentage = (offset / data.length) * 100;
  const statusElement = DOMCache.get(statusId);
  statusElement.innerHTML = `Uploading... ${progressPercentage.toFixed(0)}%`;
  statusElement.style.color = "var(--primary-color)";

  let directory = "";
  let filename = filePath;

  const lastSlashIndex = filePath.lastIndexOf('/');
  if (lastSlashIndex !== -1) {
    directory = filePath.substring(0, lastSlashIndex);
    filename = filePath.substring(lastSlashIndex + 1);
  }

  var chunk = data.subarray(offset, offset + chunkSize) || "";
  fetch(`/upload?offset=${offset}&q=${encodeURIComponent(directory)}&file=${encodeURIComponent(filename)}&length=${data.length}`, { method: "POST", body: chunk })
    .then((res) => {
      if (res.ok && chunk.length > 0) {
        sendChunk(filePath, data, offset + chunk.length, chunkSize, statusId, callback);
      }
      return res.ok ? res.text() : Promise.reject(res.text());
    })
    .then((text) => {
      if (text === "0") {
        showStatusMessage(statusId, "Upload complete!", "success");
        if (callback) {
          callback();
        }
      }
    })
    .catch((error) => {
      console.error("Error:", error);
      showStatusMessage(statusId, "Upload failed", "error");
    });
}

function sendTableImportChunk(filename, data, offset, chunkSize, statusId, callback) {
  const progressPercentage = (offset / data.length) * 100;
  const statusElement = DOMCache.get(statusId);
  statusElement.innerHTML = `Importing... ${progressPercentage.toFixed(0)}%`;
  statusElement.style.color = "var(--primary-color)";

  var chunk = data.subarray(offset, offset + chunkSize) || "";
  fetch(`/table-import?offset=${offset}&file=${encodeURIComponent(filename)}&length=${data.length}`, { method: "POST", body: chunk })
    .then((res) => {
      if (res.ok && chunk.length > 0) {
        sendTableImportChunk(filename, data, offset + chunk.length, chunkSize, statusId, callback);
      }
      return res.ok ? res.text() : Promise.reject(res.text());
    })
    .then((text) => {
      if (text === "0") {
        showStatusMessage(statusId, "Table import completed!", "success");
        if (callback) {
          callback();
        }
      }
    })
    .catch((error) => {
      console.error("Error:", error);
      showStatusMessage(statusId, "Table import failed", "error");
    });
}

function setupDragAndDrop() {
  const dropOverlay = DOMCache.get("drop-overlay");
  let dragCounter = 0;

  const dragEnterHandler = (e) => {
    if (e.dataTransfer && e.dataTransfer.types && e.dataTransfer.types.includes('Files')) {
      e.preventDefault();
      dragCounter++;
      dropOverlay.style.display = "flex";
    }
  };

  const dragLeaveHandler = (e) => {
    if (e.dataTransfer && e.dataTransfer.types && e.dataTransfer.types.includes('Files')) {
      dragCounter--;
      if (dragCounter <= 0) {
        dragCounter = 0;
        dropOverlay.style.display = "none";
      }
    }
  };

  const dragOverHandler = (e) => {
    e.preventDefault();
  };

  const dropHandler = (e) => {
    e.preventDefault();
    e.stopPropagation();
    dragCounter = 0;
    dropOverlay.style.display = "none";

    const files = e.dataTransfer.files;
    if (files.length > 0) {
      for (let i = 0; i < files.length; i++) {
        const file = files[i];
        const reader = new FileReader();
        reader.onload = () => {
          const filePath = _directory ? `${_directory}/${file.name}` : file.name;
          const data = new Uint8Array(reader.result);
          sendChunk(filePath, data, 0, 1024 * 512, "main-status", () => {
            navigateToPath(_directory);
          });
        };
        reader.readAsArrayBuffer(file);
      }
    }
  };

  EventManager.addEventListener(document, "dragenter", dragEnterHandler);
  EventManager.addEventListener(document, "dragleave", dragLeaveHandler);
  EventManager.addEventListener(dropOverlay, "dragover", dragOverHandler);
  EventManager.addEventListener(dropOverlay, "drop", dropHandler);

  window.cleanupDragAndDrop = () => {
    EventManager.removeEventListener(document, "dragenter", dragEnterHandler);
    EventManager.removeEventListener(document, "dragleave", dragLeaveHandler);
    EventManager.removeEventListener(dropOverlay, "dragover", dragOverHandler);
    EventManager.removeEventListener(dropOverlay, "drop", dropHandler);
  };
}

function toggleLogViewer() {
  const logViewer = DOMCache.get('log-viewer');
  const dropOverlay = DOMCache.get('drop-overlay');
  const computedStyle = window.getComputedStyle(logViewer);
  const isVisible = computedStyle.display === 'flex' || logViewer.style.display === 'flex';

  if (isVisible) {
    logViewer.style.display = 'none';
    if (_logEventSource) {
      _logEventSource.close();
      _logEventSource = null;
      DOMCache.get('log-status').textContent = 'Disconnected';
    }
  } else {
    if (dropOverlay) {
      dropOverlay.style.display = 'none';
    }
    logViewer.style.display = 'flex';
    connectToLogStream();
  }
}

function connectToLogStream() {
  if (_logEventSource) {
    _logEventSource.close();
  }

  DOMCache.get('log-status').textContent = 'Connecting...';

  _logEventSource = new EventSource('/log-stream');

  _logEventSource.onopen = function() {
    DOMCache.get('log-status').textContent = 'Connected';
  };

  _logEventSource.onmessage = function(event) {
    addLogEntry(event.data);
  };

  _logEventSource.onerror = function() {
    DOMCache.get('log-status').textContent = 'Connection error';
    setTimeout(() => {
      if (_logEventSource && _logEventSource.readyState !== EventSource.CLOSED) {
        connectToLogStream();
      }
    }, 3000);
  };

  const logFilter = DOMCache.get('log-filter');
  logFilter.oninput = filterLogs;
}

function addLogEntry(logText) {
  _logEntries.push(logText);
  _logEntryCount++;

  while (_logEntries.length > _maxLogEntries) {
    _logEntries.shift();
  }

  displayLogEntry(logText);
  updateLogCount();

  if (DOMCache.get('auto-scroll').checked) {
    const logContent = DOMCache.get('log-content');
    logContent.scrollTop = logContent.scrollHeight;
  }
}

function displayLogEntry(logText) {
  const logContent = DOMCache.get('log-content');
  const logEntry = document.createElement('div');
  logEntry.className = 'log-entry';
  logEntry.setAttribute('data-text', logText.toLowerCase());

  logEntry.textContent = logText;

  logContent.appendChild(logEntry);
}

function escapeHtml(text) {
  const div = document.createElement('div');
  div.textContent = text;
  return div.innerHTML;
}

function clearLogs() {
  fetch('/command?cmd=cls', { method: 'POST' })
    .then(response => {
      if (response.ok) {
        return response.json();
      } else {
        throw new Error('Failed to clear logs on server');
      }
    })
    .then(data => {
      _logEntries = [];
      _logEntryCount = 0;
      DOMCache.get('log-content').innerHTML = '';
      updateLogCount();

      const logStatus = DOMCache.get('log-status');
      const originalText = logStatus.textContent;
      logStatus.textContent = 'Logs cleared';
      setTimeout(() => {
        logStatus.textContent = originalText;
      }, 2000);
    })
    .catch(error => {
      console.error('Error clearing logs:', error);
      _logEntries = [];
      _logEntryCount = 0;
      DOMCache.get('log-content').innerHTML = '';
      updateLogCount();

      const logStatus = DOMCache.get('log-status');
      const originalText = logStatus.textContent;
      logStatus.textContent = 'Error clearing server logs';
      setTimeout(() => {
        logStatus.textContent = originalText;
      }, 2000);
    });
}

function updateLogCount() {
  DOMCache.get('log-count').textContent = `${_logEntryCount} entries`;
}

function filterLogs() {
  SearchUtils.debounce(() => {
    const filter = DOMCache.get('log-filter').value.toLowerCase();
    const logEntries = DOMCache.querySelectorAll('.log-entry');

    let visibleCount = 0;
    logEntries.forEach(entry => {
      const logText = entry.getAttribute('data-text');

      if (filter === '' || logText.includes(filter)) {
        entry.style.display = 'block';
        visibleCount++;
      } else {
        entry.style.display = 'none';
      }
    });

    DOMCache.get('log-count').textContent = filter ? 
      `${visibleCount} visible (${_logEntryCount} total)` : 
      `${_logEntryCount} entries`;
  });
}

function navigateToPath(path) {
  if (_isNavigating) return;

  _isNavigating = true;

  if (_isViewingFile) {
    DOMCache.get("main").style.display = "flex";
    DOMCache.get("editor").style.display = "none";
    DOMCache.get("image-viewer").style.display = "none";
    _currentFilePath = null;
    _isViewingFile = false;
  }

  _directory = path;

  const url = path ? `#${encodeURIComponent(path)}` : '#';
  if (window.location.hash !== url) {
    history.pushState({path: path}, '', url);
  }

  updateBreadcrumb(path);
  fetchFiles(path);

  _isNavigating = false;
}

function updateBreadcrumb(currentPath, fileName = null, containerId = 'breadcrumb') {
  const breadcrumb = DOMCache.get(containerId);
  breadcrumb.innerHTML = '';

  const isEditor = containerId === 'editor-breadcrumb';
  const hasUnsavedChanges = isEditor && _hasChanges;

  const rootItem = document.createElement('a');
  rootItem.className = (!currentPath && !fileName) ? 'breadcrumb-item current' : 'breadcrumb-item';
  rootItem.href = '#';
  rootItem.onclick = (e) => {
    e.preventDefault();
    if (hasUnsavedChanges) {
      confirmUnsavedChanges(() => navigateToPath(''));
    } else {
      navigateToPath('');
    }
  };
  rootItem.innerHTML = 'My Drive';
  breadcrumb.appendChild(rootItem);

  if (currentPath) {
    const parts = currentPath.split('/').filter(part => part.length > 0);
    let buildPath = '';

    parts.forEach((part, index) => {
      const separator = document.createElement('span');
      separator.className = 'breadcrumb-separator';
      separator.textContent = '/';
      breadcrumb.appendChild(separator);

      buildPath += (buildPath ? '/' : '') + part;

      const item = document.createElement('a');
      const isLast = index === parts.length - 1;
      item.className = (isLast && !fileName) ? 'breadcrumb-item current' : 'breadcrumb-item';
      item.href = `#${encodeURIComponent(buildPath)}`;
      item.textContent = part;

      if (!isLast || fileName) {
        const pathToNavigate = buildPath;
        item.onclick = (e) => {
          e.preventDefault();
          if (hasUnsavedChanges) {
            confirmUnsavedChanges(() => navigateToPath(pathToNavigate));
          } else {
            navigateToPath(pathToNavigate);
          }
        };
      } else {
        item.onclick = (e) => e.preventDefault();
      }

      breadcrumb.appendChild(item);
    });
  }

  if (fileName) {
    const fileSeparator = document.createElement('span');
    fileSeparator.className = 'breadcrumb-separator';
    fileSeparator.textContent = '/';
    breadcrumb.appendChild(fileSeparator);

    const fileItem = document.createElement('span');
    fileItem.className = 'breadcrumb-item current';
    fileItem.id = isEditor ? 'editor-breadcrumb-filename' : '';

    if (hasUnsavedChanges) {
      fileItem.textContent = fileName + ' *';
    } else {
      fileItem.textContent = fileName;
    }

    breadcrumb.appendChild(fileItem);
  }
}

function updateFileBreadcrumb(currentPath, fileName, containerId = 'breadcrumb') {
  updateBreadcrumb(currentPath, fileName, containerId);
}

function handlePopState(event) {
  if (_isNavigating) return;

  closeAllDropdowns();

  if (_hasChanges && _isViewingFile) {
    const fileName = _currentFilePath ? _currentFilePath.split('/').pop() : 'file';
    const message = `You have unsaved changes in ${fileName}. Are you sure you want to leave without saving?`;

    if (!confirm(message)) {
      event.preventDefault && event.preventDefault();
      event.stopPropagation && event.stopPropagation();

      _isNavigating = true;

      history.go(1);

      setTimeout(() => {
        _isNavigating = false;
      }, 100);

      return;
    } else {
      _hasChanges = false;
    }
  }

  _isNavigating = true;

  if (event.state && (event.state.type === 'file' || event.state.type === 'image')) {
    const fileName = event.state.fileName;
    const directory = event.state.directory;

    if (directory !== _directory) {
      updateBreadcrumb(directory);
      fetchFiles(directory);
    }

    setTimeout(() => {
      if (event.state.type === 'image') {
        viewImage(fileName);
      } else {
        viewFile(fileName);
      }
      _isNavigating = false;
    }, 0);

  } else {
    if (_isViewingFile) {
      DOMCache.get("main").style.display = "flex";
      DOMCache.get("editor").style.display = "none";
      DOMCache.get("image-viewer").style.display = "none";
      _currentFilePath = null;
      _isViewingFile = false;
    }

    const path = event.state && event.state.path !== undefined ? event.state.path : 
                 window.location.hash ? window.location.hash.substring(1) : '';

    updateBreadcrumb(path);
    fetchFiles(path);
    _isNavigating = false;
  }
}

function handleHashChange(hash) {
  if (!hash) {
    navigateToPath('');
    return;
  }

  let decodedHash;
  try {
    decodedHash = decodeURIComponent(hash);
  } catch (e) {
    console.warn('Failed to decode hash:', hash, e);
    decodedHash = hash;
  }

  const lastSlash = decodedHash.lastIndexOf('/');
  const lastPart = lastSlash === -1 ? decodedHash : decodedHash.substring(lastSlash + 1);

  const hasFileExtension = lastPart.includes('.') && lastPart.split('.').pop().length <= 5;

  if (hasFileExtension) {
    let directory, fileName;
    if (lastSlash === -1) {
      directory = '';
      fileName = decodedHash;
    } else {
      directory = decodedHash.substring(0, lastSlash);
      fileName = decodedHash.substring(lastSlash + 1);
    }

    _isNavigating = true;

    const openFileWhenReady = () => {
      _isNavigating = false;

      const fileExt = fileName.split('.').pop().toLowerCase();
      const isImageFile = FileTypeHelper.isImage(fileExt);

      if (isImageFile) {
        viewImage(fileName);
      } else {
        viewFile(fileName);
      }
    };

    if (directory !== _directory) {
      _directory = directory;
      updateBreadcrumb(directory);

      fetchFiles(directory).then(() => {
        setTimeout(openFileWhenReady, 150);
      }).catch((error) => {
        console.error('Error fetching directory files:', error);
        _isNavigating = false;
      });
    } else {
      setTimeout(openFileWhenReady, 50);
    }
  } else {
    navigateToPath(decodedHash);
  }
}

function startup() {
  setupDragAndDrop();

  const clickHandler = (e) => {
    if (_currentContextMenu && !_currentContextMenu.contains(e.target)) {
      closeContextMenu();
    }

    closeAllDropdowns(e);
  };

  const beforeUnloadHandler = (e) => {
    if (_hasChanges && _isViewingFile) {
      const fileName = _currentFilePath ? _currentFilePath.split('/').pop() : 'file';
      const message = `You have unsaved changes in ${fileName}. Are you sure you want to leave?`;
      e.preventDefault();
      e.returnValue = message;
      return message;
    }
  };

  const hashChangeHandler = () => {
    if (!_isNavigating) {
      const hash = window.location.hash ? window.location.hash.substring(1) : '';
      handleHashChange(hash);
    }
  };

  const keydownHandler = (e) => {
    if (e.key === 'Escape') {
      if (_isViewingFile) {
        if (DOMCache.get('editor').style.display !== 'none') {
          closeEditor();
        } else if (DOMCache.get('image-viewer').style.display !== 'none') {
          closeImageViewer();
        }
      }
    }

    if (e.ctrlKey && e.key === 's' && _isViewingFile && !_isReadOnly) {
      e.preventDefault();
      const saveButton = DOMCache.get('save-button');
      if (saveButton && saveButton.style.display !== 'none') {
        saveButton.click();
      }
    }
  };

  EventManager.addEventListener(document, 'click', clickHandler);
  EventManager.addEventListener(window, 'popstate', handlePopState);
  EventManager.addEventListener(window, 'beforeunload', beforeUnloadHandler);
  EventManager.addEventListener(window, 'hashchange', hashChangeHandler);
  EventManager.addEventListener(document, 'keydown', keydownHandler);

  window.cleanupGlobalEvents = () => {
    EventManager.removeEventListener(document, 'click', clickHandler);
    EventManager.removeEventListener(window, 'popstate', handlePopState);
    EventManager.removeEventListener(window, 'beforeunload', beforeUnloadHandler);
    EventManager.removeEventListener(window, 'hashchange', hashChangeHandler);
    EventManager.removeEventListener(document, 'keydown', keydownHandler);
  };

  window.cleanupAllEvents = () => {
    if (window.cleanupGlobalEvents) window.cleanupGlobalEvents();
    if (window.cleanupDragAndDrop) window.cleanupDragAndDrop();
    EventManager.cleanupRemovedElements();
    DOMCache.clear();
  };

  setInterval(() => {
    EventManager.cleanupRemovedElements();
  }, 30000);

  const initialHash = window.location.hash ? window.location.hash.substring(1) : '';

  fetchInfo()
    .then(data => {
      connectStatusWebSocket();

      if (initialHash) {
        handleHashChange(initialHash);
      } else {
        updateBreadcrumb('');
        fetchFiles('');
      }
    })
    .catch(error => {
      console.error('Error fetching initial info:', error);
      _connectionFailed = true;
      clearStatusDisplay();
      if (initialHash) {
        handleHashChange(initialHash);
      } else {
        updateBreadcrumb('');
        fetchFiles('');
      }
    });
}

const DropdownManager = {
  toggle: (id) => {
    const dropdown = DOMCache.get(id);
    if (dropdown) dropdown.classList.toggle('show');
  },

  hide: (id) => {
    const dropdown = DOMCache.get(id);
    if (dropdown) dropdown.classList.remove('show');
  },

  hideAll: (event, dropdownConfigs) => {
    dropdownConfigs.forEach(config => {
      const dropdown = DOMCache.get(config.dropdownId);
      const button = DOMCache.get(config.buttonId);

      if (dropdown && !dropdown.contains(event.target) && (!button || !button.contains(event.target))) {
        dropdown.classList.remove('show');
      }
    });
  }
};

function toggleActionsDropdown() {
  DropdownManager.toggle('actions-dropdown');
}

function hideActionsDropdown() {
  DropdownManager.hide('actions-dropdown');
}

const DROPDOWN_CONFIGS = [
  { dropdownId: 'main-actions-dropdown', buttonId: 'main-actions-button' },
  { dropdownId: 'image-actions-dropdown', buttonId: 'image-actions-button' },
  { dropdownId: 'log-actions-dropdown', buttonId: 'log-actions-button' },
  { dropdownId: 'editor-actions-dropdown', buttonId: 'editor-actions-button' }
];

function closeAllDropdowns(event) {
  DropdownManager.hideAll(event, DROPDOWN_CONFIGS);
}

function toggleImageActionsDropdown() {
  DropdownManager.toggle('image-actions-dropdown');
}

function hideImageActionsDropdown() {
  DropdownManager.hide('image-actions-dropdown');
}

function downloadCurrentImage() {
  if (window.currentImageFile) {
    downloadFile(window.currentImageFile.fileName, window.currentImageFile.filePath);
  }
}

function toggleMainActionsDropdown() {
  DropdownManager.toggle('main-actions-dropdown');
}

function hideMainActionsDropdown() {
  DropdownManager.hide('main-actions-dropdown');
}

function toggleEditorActionsDropdown() {
  DropdownManager.toggle('editor-actions-dropdown');
}

function hideEditorActionsDropdown() {
  DropdownManager.hide('editor-actions-dropdown');
}

function downloadCurrentFile() {
  if (_currentFilePath) {
    const fileName = _currentFilePath.split('/').pop();
    downloadFile(fileName, _currentFilePath);
  }
}

function saveCurrentFile() {
  if (_editorView && _currentFilePath) {
    let content = '';

    if (_editorView.editor && _editorView.editor.getValue) {
      content = _editorView.editor.getValue();
    } else if (_editorView.state && _editorView.state.doc) {
      content = _editorView.state.doc.toString();
    } else if (_editorView.textarea) {
      content = _editorView.textarea.value;
    }

    const data = new Uint8Array(new TextEncoder().encode(content));
    sendChunk(_currentFilePath, data, 0, 1024 * 512, "editor-status", () => {
      showStatusMessage("editor-status", "File saved successfully!", "success");
      _originalContent = content;
      _hasChanges = false;
      updateEditorMenu();

      const filenameElement = DOMCache.get('editor-breadcrumb-filename');
      if (filenameElement && _currentFilePath) {
        const fileName = _currentFilePath.split('/').pop();
        filenameElement.textContent = fileName;
        filenameElement.classList.remove('has-changes');
      }

      if (_isViewingFile && !_isNavigating) {
        const currentPath = _directory || '';
        const fileName = _currentFilePath ? _currentFilePath.split('/').pop() : '';
        history.replaceState({
          type: 'file',
          directory: currentPath,
          fileName: fileName,
          filePath: _currentFilePath,
          hasUnsavedChanges: false
        }, '', `#${currentPath ? encodeURIComponent(currentPath) + '/' : ''}${encodeURIComponent(fileName)}`);
      }
    });
  }
}

function revertChanges() {
  if (_editorView && _originalContent !== null) {
    _hasChanges = false;

    const filenameElement = DOMCache.get('editor-breadcrumb-filename');
    if (filenameElement && _currentFilePath) {
      const fileName = _currentFilePath.split('/').pop();
      filenameElement.textContent = fileName;
      filenameElement.classList.remove('has-changes');
    }

    const fileName = _currentFilePath ? _currentFilePath.split('/').pop() : '';
    const fileExt = fileName.split('.').pop()?.toLowerCase();
    const isReadOnlyFile = FileTypeHelper.isReadOnly(fileExt);

    createEditor(_originalContent, isReadOnlyFile);
    updateEditorMenu();

    if (_isViewingFile && !_isNavigating) {
      const currentPath = _directory || '';
      const fileName = _currentFilePath ? _currentFilePath.split('/').pop() : '';
      history.replaceState({
        type: 'file',
        directory: currentPath,
        fileName: fileName,
        filePath: _currentFilePath,
        hasUnsavedChanges: false
      }, '', `#${currentPath ? encodeURIComponent(currentPath) + '/' : ''}${encodeURIComponent(fileName)}`);
    }

    showStatusMessage("editor-status", "Changes reverted", "info");
  }
}

function checkForChanges() {
  if (_editorView && _originalContent !== null && !_isReadOnly) {
    let currentContent = '';

    if (_editorView.editor && _editorView.editor.getValue) {
      currentContent = _editorView.editor.getValue();
    } else if (_editorView.state && _editorView.state.doc) {
      currentContent = _editorView.state.doc.toString();
    } else if (_editorView.textarea) {
      currentContent = _editorView.textarea.value;
    }

    const hasChanges = currentContent !== _originalContent;

    if (hasChanges !== _hasChanges) {
      _hasChanges = hasChanges;
      updateEditorMenu();

      const filenameElement = DOMCache.get('editor-breadcrumb-filename');
      if (filenameElement && _currentFilePath) {
        const fileName = _currentFilePath.split('/').pop();
        filenameElement.textContent = hasChanges ? fileName + ' *' : fileName;

        if (hasChanges) {
          filenameElement.classList.add('has-changes');
        } else {
          filenameElement.classList.remove('has-changes');
        }
      }

      if (_isViewingFile && !_isNavigating) {
        const currentPath = _directory || '';
        const fileName = _currentFilePath ? _currentFilePath.split('/').pop() : '';
        history.replaceState({
          type: 'file',
          directory: currentPath,
          fileName: fileName,
          filePath: _currentFilePath,
          hasUnsavedChanges: hasChanges
        }, '', `#${currentPath ? encodeURIComponent(currentPath) + '/' : ''}${encodeURIComponent(fileName)}`);
      }
    }
  }
}

function confirmUnsavedChanges(callback) {
  if (_hasChanges) {
    const fileName = _currentFilePath ? _currentFilePath.split('/').pop() : 'file';
    const message = `You have unsaved changes in ${fileName}. Are you sure you want to leave without saving?`;

    if (confirm(message)) {
      callback();
    }
  } else {
    callback();
  }
}

function updateEditorMenu() {
  const saveAction = DOMCache.get('editor-save-action');
  const revertAction = DOMCache.get('editor-revert-action');
  const downloadAction = DOMCache.get('editor-download-action');

  if (saveAction && revertAction) {
    const fileName = _currentFilePath ? _currentFilePath.split('/').pop() : '';
    const fileExt = fileName.split('.').pop()?.toLowerCase();
    const isReadOnlyFile = FileTypeHelper.isReadOnly(fileExt);

    if (isReadOnlyFile) {
      saveAction.style.display = 'none';
      revertAction.style.display = 'none';
    } else {
      saveAction.style.display = 'block';
      revertAction.style.display = 'block';

      if (_hasChanges && !_isReadOnly) {
        saveAction.classList.remove('disabled');
        revertAction.classList.remove('disabled');
        saveAction.style.pointerEvents = 'auto';
        revertAction.style.pointerEvents = 'auto';
        saveAction.style.opacity = '1';
        revertAction.style.opacity = '1';
      } else {
        saveAction.classList.add('disabled');
        revertAction.classList.add('disabled');
        saveAction.style.pointerEvents = 'none';
        revertAction.style.pointerEvents = 'none';
        saveAction.style.opacity = '0.5';
        revertAction.style.opacity = '0.5';
      }
    }
  }

  if (downloadAction) {
    if (_hasChanges) {
      downloadAction.classList.add('disabled');
      downloadAction.style.pointerEvents = 'none';
      downloadAction.style.opacity = '0.5';
    } else {
      downloadAction.classList.remove('disabled');
      downloadAction.style.pointerEvents = 'auto';
      downloadAction.style.opacity = '1';
    }
  }
}

window.toggleMainActionsDropdown = toggleMainActionsDropdown;
window.hideMainActionsDropdown = hideMainActionsDropdown;
window.toggleImageActionsDropdown = toggleImageActionsDropdown;
window.hideImageActionsDropdown = hideImageActionsDropdown;
window.downloadCurrentImage = downloadCurrentImage;
window.toggleLogActionsDropdown = toggleLogActionsDropdown;
window.hideLogActionsDropdown = hideLogActionsDropdown;
window.downloadLogs = downloadLogs;
window.closeLogViewer = closeLogViewer;
window.clearLogs = clearLogs;
window.toggleEditorActionsDropdown = toggleEditorActionsDropdown;
window.hideEditorActionsDropdown = hideEditorActionsDropdown;
window.downloadCurrentFile = downloadCurrentFile;
window.saveCurrentFile = saveCurrentFile;
window.revertChanges = revertChanges;
window.checkForChanges = checkForChanges;
window.newFolder = newFolder;
window.uploadFile = uploadFile;
window.toggleLogViewer = toggleLogViewer;

function toggleLogActionsDropdown() {
  DropdownManager.toggle('log-actions-dropdown');
}

function hideLogActionsDropdown() {
  DropdownManager.hide('log-actions-dropdown');
}

function downloadLogs() {
  const logText = _logEntries.join('\n');
  const blob = new Blob([logText], { type: 'text/plain' });
  const url = URL.createObjectURL(blob);

  const link = document.createElement('a');
  link.href = url;
  link.download = `vpinball-logs-${new Date().toISOString().slice(0, 19).replace(/:/g, '-')}.txt`;
  link.style.display = 'none';
  document.body.appendChild(link);
  link.click();
  document.body.removeChild(link);

  URL.revokeObjectURL(url);
}

function closeLogViewer() {
  const logViewer = DOMCache.get('log-viewer');
  if (logViewer) {
    logViewer.style.display = 'none';
    if (_logEventSource) {
      _logEventSource.close();
      _logEventSource = null;
      DOMCache.get('log-status').textContent = 'Disconnected';
    }
  }
}

function importTable() {
  const fileInput = document.createElement("input");
  fileInput.type = "file";
  fileInput.accept = ".vpx,.vpxz";

  fileInput.addEventListener("change", () => {
    if (fileInput.files.length > 0) {
      const file = fileInput.files[0];
      const fileName = file.name.toLowerCase();

      if (!fileName.endsWith('.vpx') && !fileName.endsWith('.vpxz')) {
        showStatusMessage("main-status", "Please select a .vpx or .vpxz file", "error");
        return;
      }

      showStatusMessage("main-status", `Importing ${file.name}...`, "info");

      const reader = new FileReader();
      reader.readAsArrayBuffer(file);
      reader.onload = () => {
        const data = new Uint8Array(reader.result);

        sendTableImportChunk(file.name, data, 0, 1024 * 512, "main-status", () => {
          showStatusMessage("main-status", `Table "${file.name}" imported successfully!`, "success");
          waitForRefresh();
        });
      };

      reader.onerror = () => {
        showStatusMessage("main-status", "Error reading file", "error");
      };
    }
  });

  fileInput.click();
}

function exportTableByName(tableName) {
  const actualTableName = _tables && _tables[tableName] ? _tables[tableName] : tableName;

  showStatusMessage("main-status", `Exporting ${actualTableName}...`, "info");

  const link = document.createElement('a');
  link.href = `table-export?table=${encodeURIComponent(tableName)}`;
  link.download = `${actualTableName.replace(/ /g, '_')}.vpxz`;
  link.style.display = 'none';
  link.target = '_self';
  document.body.appendChild(link);
  link.click();
  document.body.removeChild(link);

  showStatusMessage("main-status", `Exporting ${actualTableName}...`, "success");
}

function waitForRefresh(maxTimeMs = 40000) {
  const startTimestamp = Date.now();

  const originalOnMessage = _statusWs ? _statusWs.onmessage : null;

  const timeout = setTimeout(() => {
    if (_statusWs && originalOnMessage) {
      _statusWs.onmessage = originalOnMessage;
    }
    navigateToPath(_directory);
    showStatusMessage("main-status", "Import completed (timeout)", "success");
  }, maxTimeMs);

  const updateHandler = (event) => {
    try {
      const data = JSON.parse(event.data);

      if (data.lastUpdate && data.lastUpdate > startTimestamp) {
        clearTimeout(timeout);
        if (_statusWs && originalOnMessage) {
          _statusWs.onmessage = originalOnMessage;
        }

        navigateToPath(_directory);
        showStatusMessage("main-status", "Table imported successfully!", "success");
      } else if (originalOnMessage) {
        originalOnMessage(event);
      }
    } catch (error) {
      console.error('Error parsing WebSocket message during refresh wait:', error);
      if (originalOnMessage) {
        originalOnMessage(event);
      }
    }
  };

  if (_statusWs && _statusWs.readyState === WebSocket.OPEN) {
    _statusWs.onmessage = updateHandler;
  } else {
    clearTimeout(timeout);
    setTimeout(() => {
      navigateToPath(_directory);
      showStatusMessage("main-status", "Import completed", "success");
    }, 5000);
  }
}

document.addEventListener("DOMContentLoaded", startup);