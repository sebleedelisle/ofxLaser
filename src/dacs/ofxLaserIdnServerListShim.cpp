// Build the Helios IDN server list source through a local wrapper so its
// file-local logging helpers don't collide with idn.cpp on GCC/MSYS2.
#define logError idnServerListLogError
#define logInfo idnServerListLogInfo
#include "../../libs/libera-laser/libs/helios_dac/sdk/cpp/idn/idnServerList.cpp"
