#!/bin/bash

# Define inputs
username="Bobby"
password="123123"
file1="hamlet.txt"
file2="tiny.txt"

# path to your networking program and its config file
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
expect "Enter a username to proceed:"
send "$username\\r"
expect "Enter your password to proceed:"
send "$password\\r"
expect "Got response: Cannot register user under name 'Bobby', already exists"
send "$file1\\r"
expect "Signature hashes do not match for Bobby."
send "$file2\\r"
send "quit\\r"
expect eof
EOF
check_status $? "Networking interaction"


echo "All operations completed successfully."
