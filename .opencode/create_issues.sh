#!/bin/bash
# Bulk-import tasks from .opencode/tasks/current.md as GitHub Issues
# Usage: .opencode/create_issues.sh | bash
# Or dry-run (print commands): .opencode/create_issues.sh

CURRENT=".opencode/tasks/current.md"
LABEL=""

while IFS= read -r line; do

  # Phase headers → labels (e.g. "### gen_dlist.c — Core Operations")
  if echo "$line" | grep -qE '^### '; then
    LABEL=$(echo "$line" | sed 's/^### //')
    continue
  fi

  # Tasks → issues: "- [ ] 0.N  desc"
  if echo "$line" | awk '/^- \[ \] [0-9]+\.[0-9]+ /{exit 0}{exit 1}'; then
    NUM=$(echo "$line" | awk '{print $4}')
    DESC=$(echo "$line" | awk '{$1=$2=$3=$4=""; sub(/^ */, ""); print}')
    echo "gh issue create --title \"[${NUM}] ${DESC}\" --label \"${LABEL}\" --body \"Task ${NUM}\""
  fi

done < "$CURRENT"
