================================================================================
    MICROSOFT FOUNDATION CLASS LIBRARY : Win32_WML Project Overview
===============================================================================

The application wizard has created this Win32_WML application for 
you.  This application not only demonstrates the basics of using the Microsoft 
Foundation Classes but is also a starting point for writing your application.

This file contains a summary of what you will find in each of the files that
make up your Win32_WML application.

Win32_WML.vcproj
    This is the main project file for VC++ projects generated using an application wizard. 
    It contains information about the version of Visual C++ that generated the file, and 
    information about the platforms, configurations, and project features selected with the
    application wizard.

Win32_WML.h
    This is the main header file for the application.  It includes other
    project specific headers (including Resource.h) and declares the
    CWin32_WMLApp application class.

Win32_WML.cpp
    This is the main application source file that contains the application
    class CWin32_WMLApp.

Win32_WML.rc
    This is a listing of all of the Microsoft Windows resources that the
    program uses.  It includes the icons, bitmaps, and cursors that are stored
    in the RES subdirectory.  This file can be directly edited in Microsoft
    Visual C++. Your project resources are in 1033.

res\Win32_WML.ico
    This is an icon file, which is used as the application's icon.  This
    icon is included by the main resource file Win32_WML.rc.

res\Win32_WML.rc2
    This file contains resources that are not edited by Microsoft 
    Visual C++. You should place all resources not editable by
    the resource editor in this file.

/////////////////////////////////////////////////////////////////////////////

The application wizard creates one dialog class:
Win32_WMLDlg.h, Win32_WMLDlg.cpp - the dialog
    These files contain your CWin32_WMLDlg class.  This class defines
    the behavior of your application's main dialog.  The dialog's template is
    in Win32_WML.rc, which can be edited in Microsoft Visual C++.
/////////////////////////////////////////////////////////////////////////////

Help Support:

hlp\Win32_WML.hpj
    This file is the help project file used by the help compiler to create
    your application's help file.

hlp\*.bmp
    These are bitmap files required by the standard HELP file topics for
    Microsoft Foundation Class Library standard commands.

hlp\*.rtf
    These files contain the standard help topics for standard MFC
    commands and screen objects.
/////////////////////////////////////////////////////////////////////////////

Other Features:

ActiveX Controls
    The application includes support to use ActiveX controls.
/////////////////////////////////////////////////////////////////////////////

Other standard files:

StdAfx.h, StdAfx.cpp
    These files are used to build a precompiled header (PCH) file
    named Win32_WML.pch and a precompiled types file named StdAfx.obj.

Resource.h
    This is the standard header file, which defines new resource IDs.
    Microsoft Visual C++ reads and updates this file.

Win32_WML.manifest
	Application manifest files are used by Windows XP to describe an applications 
	dependency on specific versions of Side-by-Side assemblies. The loader uses this 
	information to load the appropriate assembly from the assembly cache or private 
	from the application. The Application manifest  maybe included for redistribution 
	as an external .manifest file that is installed in the same folder as the application 
	executable or it may be included in the executable in the form of a resource. 
/////////////////////////////////////////////////////////////////////////////

Other notes:

The application wizard uses "TODO:" to indicate parts of the source code you
should add to or customize.

If your application uses MFC in a shared DLL, and your application is in a 
language other than the operating system's current language, you will need 
to copy the corresponding localized resources MFC70XXX.DLL from the Microsoft
Visual C++ CD-ROM under the Win\System directory to your computer's system or 
system32 directory, and rename it to be MFCLOC.DLL.  ("XXX" stands for the 
language abbreviation.  For example, MFC70DEU.DLL contains resources 
translated to German.)  If you don't do this, some of the UI elements of 
your application will remain in the language of the operating system.

/////////////////////////////////////////////////////////////////////////////
