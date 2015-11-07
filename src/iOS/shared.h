#ifndef __SHARED_H__
#define __SHARED_H__

#if defined(__cplusplus)
extern "C" {
#endif

extern const char* get_resource_path(char* file);
extern const char* get_documents_path(char* file);
    
#define IMAMEBASEPATH "/var/mobile/Media/ROMs/iMAME4all"
    
#ifdef MYDEBUG
#define IMAMEBASEPATH "/var/mobile/Applications/50E5E139-D569-45BC-AE6F-258FB2B69CB3/iMAME4all.app/ROMS"
#endif
    
#if defined(__cplusplus)
}
#endif

#endif
