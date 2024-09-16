#!/usr/bin/env zsh

#API_HOST="http://localhost:8001/api/v1/namespaces/default/pods/compassionatedhawan-pod:6969/proxy/allData"
API_HOST=http://localhost:6969/allData


curl \
  --data-binary "@-" \
  -X POST \
  -H "Content-Type: application/json" \
  "$API_HOST"
