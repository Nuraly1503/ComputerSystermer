#!/bin/bash

# inputs
username="Bobby"
password="12341"
expected_prompt_username="Enter a username to proceed:"
expected_prompt_password="Enter your password to proceed:"
hamlet="hamlet.txt"
tiny="tiny.txt"

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
        send "$hamlet\\r";
        exp_continue
    }
    "(209/209)" {
        send "$tiny\\r";
        exp_continue
    }
    "block: 0 (1/1)" {
        send "quit\\r";
        exp_continue
    }
    eof { }
}
EOF

check_status $? "Networking interaction"

# Perform the diff operation
diff hamlet.txt ../python/server/hamlet.txt
check_status $? "Diff operation on hamlet.txt"
diff tiny.txt ../python/server/tiny.txt
check_status $? "Diff operation on tiny.txt"

echo "All operations completed successfully."



# # Use 'expect' to automate the interaction with the networking program
# /usr/bin/expect <<EOF
# spawn $networking_cmd $config_path
# expect "Enter a username to proceed:"
# send "$username\r"
# expect "Enter your password to proceed:"
# send "$password\r"
# expect "Type filename to retrieve file, or 'quit' to quit:"
# send "$filename\r"
# expect "(209/209)"
# send "$tiny_file\r"
# expect "block: 0 (1/1)"
# send "quit\r"
# expect eof
# EOF
# check_status $? "Networking interaction"

# # Perform the diff operation
# diff hamlet.txt ../python/server/hamlet.txt
# check_status $? "Diff operation on hamlet.txt"
# diff tiny.txt ../python/server/tiny.txt
# check_status $? "Diff operation on tiny.txt"

# echo "All operations completed successfully."











# #!/bin/bash

# # Define inputs and expected outputs
# username="Bobby"
# password="123123"
# expected_prompt_username="Enter a username to proceed:"
# expected_prompt_password="Enter your password to proceed:"
# # Add more expected prompts as needed

# # Path to your networking program and its config file
# networking_cmd="./networking"
# config_path="../config.yaml"

# # Function to check if the expected prompt matches the actual prompt
# check_prompt() {
#     if [[ "$1" == *"$2"* ]]; then
#         echo "Successfully received expected prompt: $2"
#     else
#         echo "Failed to receive expected prompt: $2"
#         echo "Actual output: $1"
#         exit 1
#     fi
# }

# # Use 'expect' to automate the interaction
# /usr/bin/expect <<EOF
# spawn $networking_cmd $config_path
# expect {
#     "$expected_prompt_username" { 
#         send "$username\\r"; 
#         exp_continue 
#     }
#     "$expected_prompt_password" { 
#         send "$password\\r"; 
#         exp_continue 
#     }
#     # Add more patterns as needed
#     eof { }
# }
# EOF

# echo "Edge case test completed successfully."
