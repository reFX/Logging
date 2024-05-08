#ifdef REFX_DEBUGGING_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of reFX cpp file"
#endif

#ifdef _WIN32
 #include <Windows.h>
 #include <winnt.h>
#endif

#include <refx_utilities/refx_utilities.h>

#include "refx_logging.h"

#include "Source/refx_logging.cpp"
#include "Source/refx_loggingWindow.cpp"
