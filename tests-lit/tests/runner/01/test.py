import sys
import subprocess
exec = sys.argv[1]
test_name = sys.argv[2]

if test_name == "first test case":
    subprocess.run([exec, "first test"], check=True)

if test_name == "second test case":
    subprocess.run([exec, "second test"], check=True)
