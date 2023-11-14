#!/bin/bash

# Define inputs
username="Rose"
password="abc1234Y"
expected_prompt_username="Enter a username to proceed:"
expected_prompt_password="Enter your password to proceed:"
nonexisting_file="nonexisting.txt"
nonexisting_file2="nonexisting2.txt"
hamlet="hamlet.txt"
# Path to your networking program and its config file
networking_cmd="./networking"
config_path="../config.yaml"

# Change to the correct directory
cd ..

# Function to check exit status
check_status() {
    if [ $1 -eq 0 ]; then
        echo "Success: $2"
    else
        echo "Error: $2"
        exit 1
    fi
}

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
    "Type filename to retrieve file, or 'quit' to quit:" {
        send "$nonexisting_file\\r";
        exp_continue
    }
    "Requested content $nonexisting_file does not exist" {
        send "$nonexisting_file2\\r";
        exp_continue
    }
    "Requested content $nonexisting_file2 does not exist" {
        send "quit\\r";
        exp_continue
    }
    eof { }
}
EOF
check_status $? "Networking interaction"

echo "All operations completed successfully."