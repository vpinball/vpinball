let _monacoLoaded = false;
let _monacoLoading = false;
let _monacoLoadPromise = null;

async function loadMonacoBundle() {
  if (_monacoLoaded) {
    return Promise.resolve();
  }

  if (_monacoLoading) {
    return _monacoLoadPromise;
  }

  _monacoLoading = true;
  _monacoLoadPromise = new Promise((resolve, reject) => {
    const script = document.createElement('script');
    script.src = 'monaco/monaco.js';
    script.onload = () => {
      setTimeout(() => {
        if (typeof window.monaco !== 'undefined') {
          _monacoLoaded = true;
          _monacoLoading = false;
          resolve();
        } else {
          _monacoLoading = false;
          _monacoLoadPromise = null;
          reject(new Error('Monaco not available after loading'));
        }
      }, 100);
    };
    script.onerror = () => {
      _monacoLoading = false;
      _monacoLoadPromise = null;
      reject(new Error('Failed to load Monaco bundle'));
    };
    document.head.appendChild(script);
  });

  return _monacoLoadPromise;
}

window.createMonacoEditor = async function(container, content, options = {}) {
  container.innerHTML = '<div style="display: flex; align-items: center; justify-content: center; height: 100vh; color: #666; font-size: 16px;">Loading...</div>';

  try {
    await loadMonacoBundle();

    await new Promise(resolve => setTimeout(resolve, 100));

    container.innerHTML = '';

    let language = 'plaintext';
    if (options.fileName) {
      const fileName = options.fileName.toLowerCase();
      if (fileName.endsWith('.vbs') || fileName.endsWith('.vb')) {
        language = 'vb';
      } else if (fileName.endsWith('.ini')) {
        language = 'ini';
      } else if (fileName.endsWith('.xml')) {
        language = 'xml';
      }
    }

    const theme = options.theme === 'dark' ? 'vs-dark' : 'vs';

    const editor = window.monaco.editor.create(container, {
      value: content,
      language: language,
      theme: theme,
      automaticLayout: true,
      lineNumbers: 'on',
      minimap: { enabled: false },
      scrollBeyondLastLine: false,
      wordWrap: 'off',
      readOnly: options.readOnly || false,
      fontSize: 14,
      fontFamily: "'Consolas', 'Monaco', 'Menlo', 'Ubuntu Mono', monospace",
      tabSize: 2,
      insertSpaces: true,
      contextmenu: false,
      folding: false,
      glyphMargin: false,
      lightbulb: { enabled: false },
      occurrencesHighlight: false,
      selectionHighlight: false,
      renderValidationDecorations: 'off',
      largeFileOptimizations: true,
      maxTokenizationLineLength: 20000
    });

    editor.addCommand(window.monaco.KeyMod.CtrlCmd | window.monaco.KeyCode.KeyS, () => {
      if (window.saveCurrentFile) {
        window.saveCurrentFile();
      }
    });

    editor.onDidChangeModelContent(() => {
      if (window.checkForChanges) {
        window.checkForChanges();
      }
    });

    return {
      editor: editor,
      state: {
        doc: {
          toString: () => editor.getValue()
        }
      },
      focus: () => editor.focus(),
      destroy: () => editor.dispose()
    };

  } catch (error) {
    console.error('Failed to load Monaco editor:', error);
    container.innerHTML = `<div style="display: flex; align-items: center; justify-content: center; height: 100vh; color: red; font-size: 16px; text-align: center;">Failed to load editor: ${error.message}</div>`;
    throw error;
  }
};