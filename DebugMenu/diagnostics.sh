#!/bin/bash

printf "===============================\n\r"
printf "   Example SSH Diagnostics\n\r"
printf "===============================\n\r"
stty -a

# Menu display
show_menu() {
    printf "Select an option:\n\r"
    printf "To transfer a file run scp localfile <user>@<ip>:/tmp\n\r"
    printf "1. Print a file to stdout\n\r"
    printf "2. Run a simple program\n\r"
    printf "3. Enter an interactive Bash shell\n\r"
    printf "4. Exit\n\r"
    printf "Enter your choice [1-4]: \n\r"
}


print_file() {
    echo -n "Enter the file to print out: "
    read file
    if [[ -f "$file" ]]; then
        echo "Contents of $file:"
        cat "$file"
    else
        echo "File not found!"
    fi
}

# Function to run a simple program
run_program() {
    printf "Running an example  simple diagnostic program...\n\r"
    printf "System Uptime:\n\r"
    uptime
    printf "Disk Usage:\n\r"
    df -h
    printf "Memory Usage:\n\r"
    free -h
}

# Main loop
while true; do
    show_menu
    read choice
    case $choice in
        1)
            print_file
            ;;
        2)
            run_program
            ;;
        3)
            printf "Entering interactive Bash shell. Type 'exit' to return to the menu.\n\r"
            bash
            ;;
        4)
            printf "Exiting SSH Diagnostics Server. Goodbye!\n\r"
            break
            ;;
        *)
            printf "Invalid choice, please try again.\n\r"
            ;;
    esac
done

exit 0
