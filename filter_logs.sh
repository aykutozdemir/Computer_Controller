#!/bin/bash

# Function to show usage
show_usage() {
    echo "Usage: ./filter_logs.sh [option]"
    echo "Options:"
    echo "  computer    - Show only ComputerController logs"
    echo "  command     - Show only CommandHandler logs"
    echo "  error       - Show only error logs"
    echo "  warn        - Show only warning logs"
    echo "  info        - Show only info logs"
    echo "  debug       - Show only debug logs"
    echo "  all         - Show all logs"
}

# Check if argument is provided
if [ $# -eq 0 ]; then
    show_usage
    exit 1
fi

# Process the argument
case "$1" in
    "computer")
        pio device monitor | sed -n '/ComputerController/p'
        ;;
    "command")
        pio device monitor | sed -n '/CommandHandler/p'
        ;;
    "error")
        pio device monitor | sed -n '/E (/p'
        ;;
    "warn")
        pio device monitor | sed -n '/W (/p'
        ;;
    "info")
        pio device monitor | sed -n '/I (/p'
        ;;
    "debug")
        pio device monitor | sed -n '/D (/p'
        ;;
    "all")
        pio device monitor
        ;;
    *)
        echo "Invalid option: $1"
        show_usage
        exit 1
        ;;
esac 