Example overriding the SFTP commands and adding in restrictions on which
commands can be used based on the username. This example is based on the
microchip filesystem but the pattern of overriding and adding in custom checks
can be applied to any filesystem.

When building wolfSSH define the macro WOLFSSH_USER_FILESYSTEM to enable a user
defined filesystem. This is usually defined in a user_settings.h file if
building with an IDE. Having this macro set makes wolfSSH search for a
myFilesystem.h file in the include path. One way to add the myFilesystem.h to
the include path is to move it into the wolfSSH headers directory.

In this example “Safe” operations are ones required for navigation of folders
and downloading files. Where restricted operations can modify and remove files.

For the example use wolfSSH_SetFilesystemHandle(ssh, (void*)ssh) to set the
WOLFSSH struct as the file system handle pointer. This would then be passed on
to the filesystem calls.

An example of integrating this into the example MPLABX build would be:
1) Opening the wolfSSH library project
2) Adding myFilesystem.c as a source file to the project
3) Defining WOLFSSH_USER_FILESYSTEM macro in user_settings.h
4) Recompling the library

Then when making use of the library in the example wolfssh.c file it would be
a change similar to:

```
    case APP_SSH_SFTP_START:
        SYS_CONSOLE_PRINT("Setting starting SFTP directory to [%s]\r\n",
                "/mnt/myDrive1");
        if (wolfSSH_SFTP_SetDefaultPath(ssh, "/mnt/myDrive1") != WS_SUCCESS) {
            SYS_CONSOLE_PRINT("Error setting starting directory\r\n");
            SYS_CONSOLE_PRINT("Error = %d\r\n", wolfSSH_get_error(ssh));
            appData.state = APP_SSH_CLEANUP;
        }
        wolfSSH_SetFilesystemHandle(ssh, (void*)ssh);
        appData.state = APP_SSH_SFTP;
        break;
```
