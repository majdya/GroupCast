#!/bin/bash
# Bulk-import tasks from .opencode/tasks/current.md as GitHub Issues
# Usage: .opencode/create_issues.sh | bash
# Or dry-run (print commands): .opencode/create_issues.sh
#
# Automatically creates missing labels before creating issues.

CURRENT=".opencode/tasks/current.md"

# ── Pass 1: collect unique labels ──
declare -A LABELS
while IFS= read -r line; do
  if echo "$line" | grep -qE '^### '; then
    RAW=$(echo "$line" | sed 's/^### //' | sed 's/—/-/g')
    LABELS["$RAW"]=1
  fi
done < "$CURRENT"

for label in "${!LABELS[@]}"; do
  echo "gh label create \"${label}\" --color \"c5def5\" 2>/dev/null || true"
done

# ── Pass 2: create issues ──
LABEL=""
while IFS= read -r line; do

  if echo "$line" | grep -qE '^### '; then
    LABEL=$(echo "$line" | sed 's/^### //' | sed 's/—/-/g')
    continue
  fi

  if echo "$line" | awk '/^- \[ \] [0-9]+\.[0-9]+ /{exit 0}{exit 1}'; then
    NUM=$(echo "$line" | awk '{print $4}')
    DESC=$(echo "$line" | awk '{$1=$2=$3=$4=""; sub(/^ */, ""); print}')
    DESC_SAFE=$(echo "$DESC" | sed 's/[`"$]/\\&/g')
    echo "gh issue create --title \"[${NUM}] ${DESC_SAFE}\" --label \"${LABEL}\" --body \"Task ${NUM}\""
  fi

done < "$CURRENT"
