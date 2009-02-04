This is the Pd port of the teabox~ Max/MSP externals.


COMPILED BINARIES

The zip archive of these objects includes pre-compiled binaries for use on the Windows and Mac platforms.  For other platforms you must compile from source as detailed below.

Regardless of whether you use the pre-compiled binaries, or you compile them yourself, the instructions below must be followed for adding the library to Pd so that the objects can be properly instantiated.


BUILDING FROM SOURCE CODE

For Macintosh Universal Binary builds, please skip to the bottom.  For all other platforms, keep reading.

In order to run it you have to load it as a library. Start Pd
with the '-lib path/to/teabox' flag. In case this worked correctly
you can use the objects 'teabox~', 'teabox.bits~' and 'teabox.count~'
like any normal Pd object.
The provided makefile should work for all platforms that Pd is 
currently running on.


MAC UNIVERSAL BINARY

To build a Universal Binary for the Mac, you can use the included Xcode project (rather than the Makefile).  The Xcode project expects that you will copy the m_pd.h header file into the src folder.  A 'Debug' build will be only for the architecture of your current machine while a 'Release' build will compile a Universal Binary that will run on both PPC and Intel processors.

The compiled binary (teabox~.pd_darwin) will need to be added to Pd as a startup item.
1. Choose the   Pd > Preferences > Startup   menu item
2. Add the path to the compiled binary in one of the empty text fields
   Note: the path should include the name of the binary (teabox~) but not the extension (.pd_darwin)
3. Save your settings and restart Pd.

