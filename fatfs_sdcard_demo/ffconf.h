#define FF_USE_STRFUNC	1
/* This option switches string functions, f_gets(), f_putc(), f_puts() and f_printf().
/
/  0: Disable string functions.
/  1: Enable without LF-CRLF conversion.
/  2: Enable with LF-CRLF conversion. */

#define FF_USE_FIND		1
/* This option switches filtered directory read functions, f_findfirst() and
/  f_findnext(). (0:Disable, 1:Enable 2:Enable with matching altname[] too) */

#define FF_USE_LABEL	1
/* This option switches volume label functions, f_getlabel() and f_setlabel().
/  (0:Disable or 1:Enable) */

#define FF_CODE_PAGE	437
/* This option specifies the OEM code page to be used on the target system.
/  Incorrect code page setting can cause a file open failure.
/
/   437 - U.S.
/   720 - Arabic
/   737 - Greek
/   771 - KBL
/   775 - Baltic
/   850 - Latin 1
/   852 - Latin 2
/   855 - Cyrillic
/   857 - Turkish
/   860 - Portuguese
/   861 - Icelandic
/   862 - Hebrew
/   863 - Canadian French
/   864 - Arabic
/   865 - Nordic
/   866 - Russian
/   869 - Greek 2
/   932 - Japanese (DBCS)
/   936 - Simplified Chinese (DBCS)
/   949 - Korean (DBCS)
/   950 - Traditional Chinese (DBCS)
/     0 - Include all code pages above and configured by f_setcp()
*/



#define FF_FS_RPATH		2
/* This option configures support for relative path.
/
/   0: Disable relative path and remove related functions.
/   1: Enable relative path. f_chdir() and f_chdrive() are available.
/   2: f_getcwd() function is available in addition to 1.
*/

/*---------------------------------------------------------------------------/
/ Drive/Volume Configurations
/---------------------------------------------------------------------------*/

#define FF_VOLUMES		1
/* Number of volumes (logical drives) to be used. (1-10) */

#include_next <ffconf.h>