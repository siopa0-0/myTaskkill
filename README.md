# myTaskkill
myTaskkill is a Windows command-line tool that emulates the functionality of taskkill.exe, allowing users to terminate processes by their PID, image name, or user. It also provides additional options for forced termination or terminating child processes associated with a specific process.

Options:

/PID or /pid

    Specifies the PID of the process to be terminated. You can optionally use /F to forcefully terminate the process or /T to terminate the specified process and any child processes.
    Syntax: myTaskkill /PID <PID> [/F | /T]
    Example: myTaskkill /PID 1234 /F
    Forcefully terminates the process with PID 1234.

/IM or /im

    Specifies the image name (executable) of the process to be terminated. You can optionally use /F to forcefully terminate the process.
    Syntax: myTaskkill /IM <image_name> [/F]
    Example: myTaskkill /IM notepad.exe /F
    Forcefully terminates all instances of notepad.exe.

/U or /u

    Terminates all processes belonging to the specified user context. This option does not require /F or /T.
    Syntax: myTaskkill /U <user_name>
    Example: myTaskkill /U domain\username
    Terminates all processes running under the specified user account.

Help (/?)

    Displays help information and lists available options.
    Syntax: myTaskkill /?

Notes:
    /F forces termination without waiting for the process to close gracefully.
    
    /T ensures all child processes spawned by the target process are also terminated.
    
    /F and /T flags can only be used with /PID or /IM as appropriate.
    
    /U by itself terminates all processes associated with the specified user context, and does not require additional flags like /F or /T.

Error Handling:
    If the process cannot be terminated, an error message will be displayed with the relevant error code.
    Invalid flag combinations (e.g., /IM with /T or /PID with an invalid argument) will result in an error message.

Requirements:
    myTaskkill is designed to run on Windows platforms and requires the appropriate permissions to terminate processes (admin privileges may not be sufficient to open certain processes). Use PsTools and spawn a shell with elevated privileges by running:
   .\PsExec -s -i -d cmd.exe on a command line prompt.
  
