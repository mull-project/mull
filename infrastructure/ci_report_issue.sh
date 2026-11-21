#!/usr/bin/env bash

# Opens an issue assigned to $ISSUE_ASSIGNEE: ci_report_issue.sh <title> <body file>
# An open issue with the same title gets the new body instead, edits don't notify

set -euo pipefail

title="$1"
body_file="$2"
assignee="${ISSUE_ASSIGNEE:?}"

number=$(gh issue list --state open --search "in:title \"$title\"" --json number,title \
  --jq "map(select(.title == \"$title\")) | .[0].number // empty")
if [ -n "$number" ]; then
  echo "" >>"$body_file"
  echo "Still failing on $(date -u +%Y-%m-%d)." >>"$body_file"
  gh issue edit "$number" --body-file "$body_file" --add-assignee "$assignee"
else
  gh issue create --title "$title" --body-file "$body_file" --assignee "$assignee"
fi
