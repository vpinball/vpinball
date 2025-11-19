# Pinball Plugins

This folder contains the plugin interface in the `plugins` folder and some generic plugins.

The plugin interface consists of a group of API that enables building portable plugin for a variety of usecases, ranging from virtual pinball to real pinball-restoration or -improvement.

Each plugin only depends on the interfaces it imports from the `plugins` folder:
- MsgPlugin: the core API that allows plugin lifecycle and communication
- LoggingPlugin: a simple shared logging API
- ScriptablePlugin: allows a plugin to expose scriptable objects in a language agnostic way
- ControllerPlugin: a generic pinball controller API
- VPXPlugin: expose some Visual Pinball X features to external plugins

The `plugins` folder also offers some implementation helpers:
- MsgPluginManager: a generic plugin manager that implements the core feature each plugin host must provide
- ResURIResolver: a unified resource resolver for the generic ControllerPlugin API
