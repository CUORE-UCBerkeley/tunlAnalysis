#!/bin/bash
set -o pipefail
for file in *.sh; do
  source "$file"
done
