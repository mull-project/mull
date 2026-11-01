import subprocess
import sys

sys.exit(subprocess.run([sys.argv[1]]).returncode)
