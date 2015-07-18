#include <stdio.h>
#include <gpath.h>

void test1()
{
    gpath_init();

    gpath_setDrivePath(0, "/resource/");
    gpath_setDrivePath(1, "/documents/");
    gpath_setDrivePath(2, "/temporary/");

    gpath_addDrivePrefix(0, "|R|");
    gpath_addDrivePrefix(1, "|D|");
    gpath_addDrivePrefix(2, "|T|");

    gpath_setDefaultDrive(0);

    printf("%d\n", gpath_getPathDrive("/hebe.png"));
    printf("%d\n", gpath_getPathDrive("hebe.png"));
    printf("%d\n", gpath_getPathDrive("|R|hebe.png"));
    printf("%d\n", gpath_getPathDrive("|D|hebe.png"));
    printf("%d\n", gpath_getPathDrive("|T|hebe.png"));

    printf("%s\n", gpath_transform("/hebe.png"));
    printf("%s\n", gpath_transform("hebe.png"));
    printf("%s\n", gpath_transform("|R|hebe.png"));
    printf("%s\n", gpath_transform("|D|hebe.png"));
    printf("%s\n", gpath_transform("|T|hebe.png"));

    gpath_cleanup();
}

void test2()
{
    gpath_init();

//    gpath_setDrivePath(0, ".");
    gpath_setDriveFlags(0, GPATH_RW | GPATH_REAL);
    gpath_setDefaultDrive(0);
    gpath_setAbsolutePathFlags(GPATH_RW | GPATH_REAL);
    gpath_addDrivePrefix(0, "|R|");

    printf("%d\n", gpath_getPathDrive("/hebe.png"));
    printf("%d\n", gpath_getPathDrive("hebe.png"));
    printf("%d\n", gpath_getPathDrive("|R|hebe.png"));

    printf("%s\n", gpath_transform("/hebe.png"));
    printf("%s\n", gpath_transform("hebe.png"));
    printf("%s\n", gpath_transform("./hebe.png"));
    printf("%s\n", gpath_transform("|R|./hebe.png"));

    gpath_cleanup();
}

int main(void)
{
    test1();
    printf("-------------\n");
    test2();
    printf("-------------\n");
    return 0;
}

