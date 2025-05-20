Example overriding the SFTP commands and adding in restrictions on which
commands can be used based on the username.

When building wolfSSH define the macro WOLFSSH_USER_FILESYSTEM and add this file
to the include path. One way to do this is to move myFilesystem.h into the
wolfSSH directory.

“Safe” operations are ones required for navigation of folders and downloading
files.

