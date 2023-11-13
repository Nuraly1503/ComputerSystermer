#!/bin/bash

# Define inputs
username="©©©©©©©©©©©©©©©©©"
password="©©©©©©©©©©©©©©©©©©"
expected_prompt_username="Enter a username to proceed:"
expected_prompt_password="Enter your password to proceed:"
hamlet="hamlet.txt"
# Path to your networking program and its config file
networking_cmd="./networking"
config_path="../config.yaml"

# Change to the correct directory
cd ..

# Function to check exit status
# check_status() {
#     if [ $1 -eq 0 ]; then
#         echo "Success: $2"
#     else
#         echo "Error: $2"
#         exit 1
#     fi
# }

# Function to check exit status
# check_prompt() {
#     if [[ "$1" == *"$2"* ]]; then
#         echo "Successfully received expected prompt: $2"
#     else
#         echo "Failed to receive expected prompt: $2"
#         echo "Actual output: $1"
#         exit 1
#     fi
# }


# Use 'expect' to automate the interaction
/usr/bin/expect <<EOF
spawn $networking_cmd $config_path
expect {
    "$expected_prompt_username" { 
        send "$username\\r"; 
        exp_continue 
    }
    "$expected_prompt_password" { 
        send "$password\\r"; 
        exp_continue 
    }
    "Got response: Something went wrong. 'utf-8' codec can't decode byte 0xa9 in position 1: invalid start byte" {
        send "$hamlet\\r";
        exp_continue
    }
    "Something went wrong. 'utf-8' codec can't decode byte 0xa9 in position 1: invalid start byte" {
        send "quit\\r";
        exp_continue
    }
    eof { }
}
EOF
check_status $? "Networking interaction"

echo "All operations completed successfully."