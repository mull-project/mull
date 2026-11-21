#!/usr/bin/env bash

# Opens an issue assigned to $ISSUE_ASSIGNEE: ci_report_issue.sh <title> <body file>
# An open issue with the same title gets a comment instead

set -euo pipefail

title="$1"
body_file="$2"
assignee="${ISSUE_ASSIGNEE:?}"

number=$(gh issue list --state open --search "in:title \"$title\"" --json number,title \
  --jq "map(select(.title == \"$title\")) | .[0].number // empty")
if [ -n "$number" ]; then
  gh issue edit "$number" --add-assignee "$assignee"
  gh issue comment "$number" --body-file "$body_file"
else
  gh issue create --title "$title" --body-file "$body_file" --assignee "$assignee"
fi
