#!/bin/bash
# Clean script for all NPS Lab programs
# This script removes all compiled binaries

echo "=========================================="
echo "NPS Lab Clean Script"
echo "=========================================="
echo ""

removed_count=0

# Function to remove binary if it exists
remove_binary() {
    local binary="$1"
    if [ -f "$binary" ]; then
        rm "$binary"
        echo "Removed: $binary"
        ((removed_count++))
    fi
}

# Clean Lab 1 binaries
echo "Cleaning Lab 1 binaries..."
cd "Lab 1"
remove_binary "client"
remove_binary "server"
remove_binary "udp_client"
remove_binary "udp_server"
remove_binary "conv_tcp_client"
remove_binary "conv_tcp_server"
remove_binary "conv_udp_client"
remove_binary "conv_udp_server"
remove_binary "palin_serv"
cd ..

# Clean Lab 3 binaries
echo "Cleaning Lab 3 binaries..."
cd "Lab 3"
remove_binary "concurrent-server"
remove_binary "concurrent-client"
remove_binary "Q1_server"
remove_binary "Q1_Client"
remove_binary "Q2_server"
remove_binary "Q2_Client"
remove_binary "Q3_server"
remove_binary "Q3_client"
remove_binary "AQ1_server"
remove_binary "AQ1_client"
cd ..

# Clean Lab 4 binaries
echo "Cleaning Lab 4 binaries..."
cd "Lab 4"
remove_binary "server"
remove_binary "client"
remove_binary "dns_server"
remove_binary "dns_client"
cd ..

# Clean Lab 5 binaries
echo "Cleaning Lab 5 binaries..."
cd "Lab 5"
remove_binary "Q1_server"
remove_binary "Q1_client1"
remove_binary "Q1_client2"
remove_binary "Q1_client_extra"
remove_binary "Q2_server_iterative"
remove_binary "Q2_client"
cd ..

echo ""
echo "=========================================="
echo "Clean Summary"
echo "=========================================="
echo "Removed $removed_count binaries"
echo "Done!"
