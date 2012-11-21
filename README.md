snapit
======

This application runs in the background and makes all windows (and their edges)
snap to each other. The functionality is similar to allSnap application.

The idea is quite simple:
* install global hook on WH_CALLWNDPROC (separately for 32 and 64 bit)
* subclass/unsubclass window procedure on WM_ENTERSIZEMOVE/WM_EXITSIZEMOVE
* adjust window position/edges during WM_WINDOWPOSCHANGING

This method does not work with windows that discard WM_ENTERSIZEMOVE and
WM_EXITSIZEMOVE message. For this windows it is possible to subclass window
procedure an each WM_WINDOWPOSCHANGING events (not yet implemented).

For more details just read the code :)
