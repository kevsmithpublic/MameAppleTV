#include "shared.h"
#include <stdio.h>

extern const char *getDocumentsFolder();
extern const char *getBundleFolder();
extern const char *getRomFolder();

const char* get_resource_path(char* file)
{
    static char resource_path[1024];

//  sprintf(resource_path, "/Applications/iMAME4all.app/%s", file);
    sprintf(resource_path, "%s/%s", getBundleFolder(), file);

    return resource_path;
}

const char* get_documents_path(char* file)
{
    static char documents_path[1024];
    
//  sprintf(documents_path, IMAMEBASEPATH "/%s", file);
    sprintf(documents_path, "%s/%s", getRomFolder(), file);
    
    return documents_path;
}
