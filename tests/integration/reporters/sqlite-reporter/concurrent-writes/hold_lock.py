"""Holds an exclusive SQLite lock for the given duration, then releases it."""

import sqlite3
import sys
import time


def log(msg):
    ts = time.strftime("%H:%M:%S")
    print(f"[{ts}] {msg}", flush=True)


db_path = sys.argv[1]
ready_file = sys.argv[2]
duration = float(sys.argv[3])

conn = sqlite3.connect(db_path, timeout=0)
log("DB CONNECTED")
conn.execute("BEGIN EXCLUSIVE")
log("DB LOCKED")

# Signal that the lock is held
with open(ready_file, "w") as f:
    f.write("ready\n")

time.sleep(duration)
conn.execute("ROLLBACK")
log("DB UNLOCKED")
conn.close()
