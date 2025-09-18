#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERS 5
#define MAX_RESOURCES 5
#define MAX_NAME_LEN 20

typedef enum{ 
    READ = 1,
    WRITE = 2,
    EXECUTE = 4
}Permission;

typedef struct{
    char name[MAX_NAME_LEN];
}User;

typedef struct{
    char name[MAX_NAME_LEN];
}Resource;

typedef struct{
    char Username[MAX_NAME_LEN];
    int Permissions;
}ACLEntry;

typedef struct{
    Resource resource;
    ACLEntry ACLEntries[10];
    int ACLCount;
}ACLControlledResource;

typedef struct{
    char ResourceName[MAX_NAME_LEN];
    int Permissions;
}Capability;

typedef struct{
    User user;
    Capability Capabilities[10];
    int CapabilityCount;
}CapabilityUser;

void printPermissions(int perm){
    int first = 1;
    if (perm & READ) {
        printf("Read");
        first = 0;
    }
    if (perm & EXECUTE) {
        if (!first) printf(", ");
        printf("Execute");
    }
    if (perm & WRITE) {
        if (!first) printf(", ");
        printf("Write");
        first = 0;
    }
}

int hasPermission(int userPerm, int requiredPerm){
    return (userPerm & requiredPerm) == requiredPerm;
}

void checkACLAccess(int perm, const char *Username, ACLControlledResource *res){
    int get = 0;
    for (int i = 0; i < res->ACLCount; i++) {
        if (strcmp(res->ACLEntries[i].Username, Username) == 0) {
            get = 1;
            printf("ACL Check: User %s requests ", Username);
            printPermissions(perm);
            printf(" on %s: ", res->resource.name);
            if (hasPermission(res->ACLEntries[i].Permissions, perm)) {
                printf("Access GRANTED\n");
            } else {
                printf("Access DENIED\n");
            }
            break;
        }
    }
    if (!get) {
        printf("ACL Check: User %s has NO entry for resource %s: Access DENIED\n", Username, res->resource.name);
    }
}

void checkCapabilityAccess(int perm, CapabilityUser *user, const char *ResourceName){
    int get = 0;
    for (int i = 0; i < user->CapabilityCount; i++) {
        if (strcmp(user->Capabilities[i].ResourceName, ResourceName) == 0) {
            get = 1;
            printf("Capability Check: User %s requests ", user->user.name);
            printPermissions(perm);
            printf(" on %s: ", ResourceName);
            if (hasPermission(user->Capabilities[i].Permissions, perm)) {
                printf("Access GRANTED\n");
            } else {
                printf("Access DENIED\n");
            }
            break;
        }
    }
    if (!get) {
        printf("Capability Check: User %s has NO capability for %s: Access DENIED\n", user->user.name, ResourceName);
    }
}

void addACLEntry(ACLControlledResource *res, const char *Username, int Permissions) {
    if (res->ACLCount < 10) {
        strcpy(res->ACLEntries[res->ACLCount].Username, Username);
        res->ACLEntries[res->ACLCount].Permissions = Permissions;
        res->ACLCount++;
    } else {
        printf("Maximum entries reached, can't add more ACL Entry.\n");
    }
}

void addCapability(CapabilityUser *user, const char *ResourceName, int Permissions) {
    if (user->CapabilityCount < 10) {
        strcpy(user->Capabilities[user->CapabilityCount].ResourceName, ResourceName);
        user->Capabilities[user->CapabilityCount].Permissions = Permissions;
        user->CapabilityCount++;
    } else {
        printf("Maximum entries reached, can't add more capablities.\n");
    }
}

int main(){
    User users[5] = { {"Alice"}, {"Bob"}, {"Charlie"}, {"Rauf"}, {"Rafiz"} };
    Resource resources[5] = { {"File1"}, {"File2"}, {"File3"}, {"File4"}, {"File5"} };

    ACLControlledResource aclResources[5];
    for (int i = 0; i < 5; i++) {
        aclResources[i].resource = resources[i];
        aclResources[i].ACLCount = 0;
    }

    addACLEntry(&aclResources[0], "Alice", READ | WRITE);
    addACLEntry(&aclResources[0], "Bob", READ);
    addACLEntry(&aclResources[1], "Charlie", READ | EXECUTE);
    addACLEntry(&aclResources[1], "Alice", READ);
    addACLEntry(&aclResources[2], "Bob", WRITE);
    addACLEntry(&aclResources[2], "Rauf", READ | WRITE | EXECUTE);
    addACLEntry(&aclResources[3], "Rafiz", READ | EXECUTE);
    addACLEntry(&aclResources[3], "Charlie", WRITE);
    addACLEntry(&aclResources[4], "Rauf", EXECUTE);
    addACLEntry(&aclResources[4], "Alice", WRITE | EXECUTE);

    CapabilityUser UsersCapability[5];
    for (int i = 0; i < 5; i++) {
        UsersCapability[i].user = users[i];
        UsersCapability[i].CapabilityCount = 0;
    }

    addCapability(&UsersCapability[0], "File1", READ | WRITE);
    addCapability(&UsersCapability[0], "File2", READ);
    addCapability(&UsersCapability[0], "File5", WRITE | EXECUTE);
    addCapability(&UsersCapability[1], "File1", READ);
    addCapability(&UsersCapability[1], "File3", WRITE);
    addCapability(&UsersCapability[2], "File2", READ | EXECUTE);
    addCapability(&UsersCapability[2], "File4", WRITE);
    addCapability(&UsersCapability[3], "File3", READ | WRITE | EXECUTE);
    addCapability(&UsersCapability[3], "File5", EXECUTE);
    addCapability(&UsersCapability[4], "File4", READ | EXECUTE);
    addCapability(&UsersCapability[4], "File1", READ);

    checkACLAccess(READ, "Alice", &aclResources[0]);
    checkACLAccess(WRITE, "Bob", &aclResources[0]);
    checkACLAccess(READ, "Charlie", &aclResources[0]);
    checkACLAccess(EXECUTE, "Charlie", &aclResources[1]);
    checkACLAccess(EXECUTE, "Rauf", &aclResources[2]);
    checkACLAccess(EXECUTE, "Rafiz", &aclResources[3]);
    checkACLAccess(WRITE, "Rauf", &aclResources[4]);
    checkACLAccess(EXECUTE, "Alice", &aclResources[4]);

    checkCapabilityAccess(WRITE, &UsersCapability[0], "File1");
    checkCapabilityAccess(WRITE, &UsersCapability[1], "File1");
    checkCapabilityAccess(EXECUTE, &UsersCapability[2], "File2");
    checkCapabilityAccess(READ, &UsersCapability[3], "File3");
    checkCapabilityAccess(READ, &UsersCapability[4], "File4");
    checkCapabilityAccess(READ, &UsersCapability[4], "File2");
    checkCapabilityAccess(EXECUTE, &UsersCapability[0], "File5");
    checkCapabilityAccess(WRITE, &UsersCapability[3], "File5");

    return 0;
}