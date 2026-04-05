# Where should we write the GIF?
Output demo.webm
Output demo.mp4

# Set up a 1200x600 terminal with 46px font.
Set FontSize 24
Set Width 1480
Set Height 1420
Set Framerate 24

# Type a command in the terminal.
Type "echo 'Welcome to Mull!'"

# Pause for dramatic effect...
Sleep 500ms

# Run the command by pressing enter.
Enter

Type "batcat main.c"
Sleep 500ms
Enter
Sleep 500ms

Type "clang-22 \"
Enter
Type "  -fpass-plugin=/usr/lib/mull-ir-frontend-22 \"
Enter
Type "  -g -grecord-command-line \"
Enter
Type "  main.c -o range_tests"
Enter
Sleep 1000ms

Type "mull-runner-22 range_tests"
Sleep 500ms
Enter

# Admire the output for a bit.
Sleep 10s
