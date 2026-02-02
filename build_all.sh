#!/bin/bash
# Build script for all NPS Lab programs
# This script compiles all C programs in each lab directory

echo "=========================================="
echo "NPS Lab Build Script"
echo "=========================================="
echo ""

# Color codes
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

build_count=0
error_count=0

# Function to build a program
build_program() {
    local src_file="$1"
    local output_file="$2"
    local flags="$3"
    
    echo -n "Building $src_file... "
    if gcc $flags -o "$output_file" "$src_file" 2>/dev/null; then
        echo -e "${GREEN}OK${NC}"
        ((build_count++))
        return 0
    else
        echo -e "${RED}FAILED${NC}"
        gcc $flags -o "$output_file" "$src_file" 2>&1
        ((error_count++))
        return 1
    fi
}

# Build Lab 1 programs
echo "Building Lab 1 programs..."
cd "Lab 1"

build_program "client.c" "client" ""
build_program "server.c" "server" ""
build_program "udp_client.c" "udp_client" ""
build_program "udp_server.c" "udp_server" ""
build_program "Conversation_Client_TCP.c" "conv_tcp_client" ""
build_program "Conversation_Server_TCP.c" "conv_tcp_server" ""
build_program "Conversation_Client_UDP.c" "conv_udp_client" ""
build_program "Conversation_Server_UDP.c" "conv_udp_server" ""
build_program "palin_serv.c" "palin_serv" ""

cd ..
echo ""

# Build Lab 3 programs
echo "Building Lab 3 programs..."
cd "Lab 3"

build_program "concurrent-server.c" "concurrent-server" "-pthread"
build_program "concurrent-client.c" "concurrent-client" "-pthread"
build_program "Q1_server.c" "Q1_server" "-pthread"
build_program "Q1_Client.c" "Q1_Client" "-pthread"
build_program "Q2_server.c" "Q2_server" "-pthread"
build_program "Q2_Client.c" "Q2_Client" "-pthread"
build_program "Q3_server.c" "Q3_server" "-pthread"
build_program "Q3_client.c" "Q3_client" "-pthread"
build_program "AQ1_server.c" "AQ1_server" "-pthread"
build_program "AQ1_client.c" "AQ1_client" "-pthread"

cd ..
echo ""

# Build Lab 4 programs
echo "Building Lab 4 programs..."
cd "Lab 4"

build_program "server.c" "server" ""
build_program "client.c" "client" ""
build_program "dns_server.c" "dns_server" ""
build_program "dns_client.c" "dns_client" ""

cd ..
echo ""

# Build Lab 5 programs
echo "Building Lab 5 programs..."
cd "Lab 5"

build_program "Q1_server.c" "Q1_server" "-pthread"
build_program "Q1_client1.c" "Q1_client1" ""
build_program "Q1_client2.c" "Q1_client2" ""
build_program "Q1_client_extra.c" "Q1_client_extra" ""
build_program "Q2_server_iterative.c" "Q2_server_iterative" ""
build_program "Q2_client.c" "Q2_client" ""

cd ..
echo ""

# Summary
echo "=========================================="
echo "Build Summary"
echo "=========================================="
echo -e "Programs built successfully: ${GREEN}$build_count${NC}"
if [ $error_count -gt 0 ]; then
    echo -e "Programs with errors: ${RED}$error_count${NC}"
    exit 1
else
    echo -e "All programs built successfully!"
    exit 0
fi
