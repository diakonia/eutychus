// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the LIBSCRIPT_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MOD_EUTYCHUS_SERVICE_RECORDER_DLL_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.


/* Magic for dynamic library import and export. This is unfortunately
 * compiler-specific because there isn't a standard way to do it. Currently it
 * works with the Visual Studio compiler for windows, and for GCC 4+. Anything
 * else gets all symbols made public, which gets messy */
/* The Visual Studio implementation */
#ifdef _MSC_VER
   #define MOD_EUTYCHUS_SERVICE_RECORDER_DLL_IMPORT _declspec(dllimport)
   #ifdef BUILDING_MOD_EUTYCHUS_SERVICE_RECORDER
      #define MOD_EUTYCHUS_SERVICE_RECORDER_DLL_API _declspec(dllexport)
   #elif _DLL
      #define MOD_EUTYCHUS_SERVICE_RECORDER_DLL_API _declspec(dllimport)
   #else
      #define AUDACITY_DLL_API
   #endif
#endif //_MSC_VER

/* The GCC implementation */
#ifdef CC_HASVISIBILITY // this is provided by the configure script, is only
// enabled for suitable GCC versions
/* The incantation is a bit weird here because it uses ELF symbol stuff. If we 
 * make a symbol "default" it makes it visible (for import or export). Making it
 * "hidden" means it is invisible outside the shared object. */
   #define MOD_EUTYCHUS_SERVICE_RECORDER_DLL_IMPORT __attribute__((visibility("default")))
   #ifdef BUILDING_MOD_EUTYCHUS_SERVICE_RECORDER
      #define MOD_EUTYCHUS_SERVICE_RECORDER_DLL_API __attribute__((visibility("default")))
   #else
      #define MOD_EUTYCHUS_SERVICE_RECORDER_DLL_API __attribute__((visibility("default")))
   #endif
#endif

