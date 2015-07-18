#include <gaudio.h>
#include <gpath.h>
#include <gvfs-native.h>
#include <windows.h>

void callback(int type, void *event, void *udata)
{
    printf("%d %x %x\n", type, event, udata);
}

int main(int argc, char *argv[])
{
    gpath_init();

    gpath_setDriveFlags(0, GPATH_RW | GPATH_REAL);
    gpath_setDrivePath(0, "");
    gpath_setDefaultDrive(0);

    gvfs_init();

    gevent_Init();

    gaudio_Init();

    Sleep(100);

    gaudio_Error error;
    g_id sound = gaudio_SoundCreateFromFile("be_cool.mp3", g_true, &error);
    g_id channel = gaudio_ChannelCreate(sound, g_false);
    //gaudio_ChannelSetLooping(channel, g_true);

    gaudio_ChannelAddCallback(channel, callback, NULL);

    for (int i = 0; i < 300; ++i)
    {
        gevent_Tick();
        Sleep(16);
        //printf("%d %d\n", gaudio_ChannelGetPosition(channel), gaudio_SoundGetLength(sound));
    }

    gaudio_Cleanup();

    gevent_Cleanup();

    gvfs_cleanup();

    gpath_cleanup();

    return 0;
}
