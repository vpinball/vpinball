# Command Line

VPX supports the following command line options:

| Command | Description |
| --- | - |
| -? | Displays the following info |
| -UnregServer   | Unregister VP functions |
| -RegServer     | Register VP functions |
| | |
| -DisableTrueFullscreen   | Force-disable True Fullscreen setting |
| -EnableTrueFullscreen    | Force-enable True Fullscreen setting |
| -Minimized               | Start VP in the 'invisible' minimized window mode |
| -ExtMinimized            | Start VP in the 'invisible' minimized window mode, but with enabled Pause Menu |
| -Primary                 | Force VP to render on the Primary/Pixel(0,0) Monitor |
| | |
| -GLES [value]            | Overrides the global emission scale (day/night setting, value range: 0.115..0.925) |
| | |
| -LessCPUthreads          | Limit the amount of parallel execution |
| | |
| -Edit [filename]         | Load file into VP |
| -Play [filename]         | Load and play file |
| | |
| -PovEdit [filename]      | Load and run file in camera mode, then export new pov on exit |
| -Pov [filename]          | Load, export pov and close |
| | |
| -ExtractVBS [filename]   | Load, export table script and close |
| | |
| -Ini [filename]          | Use a custom settings file instead of loading it from the default location |
| | |
| -c1 [customparam] .. -c9 [customparam]   | Custom user parameters that can be accessed in the script via GetCustomParam(X) |



Also, to do a rough profiling of the table loading:
  if a file 'Profile.txt' exists in the same directory as VPX, then statistics for the loaded tables will be appended