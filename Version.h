#pragma once

#define PHASE_ALPHA -alpha
#define PHASE_BETA -beta
#define PHASE_PRERELEASE -prerelease
#define PHASE_RELEASE -release

#define VERSION_MAJOR 2
#define VERSION_MINOR 0
#define VERSION_PATCH 1
#define VERSION_BUILD 422
#define VERSION_PHASE PHASE_RELEASE

#define stringify(a) stringify_(a)
#define stringify_(a) #a

