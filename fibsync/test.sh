#!/bin/bash

echo "Starting node"
node ./index.js &
echo "waiting for node to start..."
sleep 5s

echo "Starting test"

echo "Test 1 /api/1/generate/5/" > /tmp/test-1.log
curl -H "Content-Type: application/json" -X GET http://localhost:3000/api/1/generate/5 1>>/tmp/test-1.log 2>>/tmp/test-1.err
echo "Test 2 /api/1/generate/1478/" >> /tmp/test-1.log
curl -H "Content-Type: application/json" -X GET http://localhost:3000/api/1/generate/1478 1>>/tmp/test-1.log 2>>/tmp/test-1.err
echo "Test 3 /api/1/generate/ABCD/" >> /tmp/test-1.log
curl -H "Content-Type: application/json" -X GET http://localhost:3000/api/1/generate/ABCD 1>>/tmp/test-1.log 2>>/tmp/test-1.err
echo "Test 4 /api/1/generate/-1/" >> /tmp/test-1.log
curl -H "Content-Type: application/json" -X GET http://localhost:3000/api/1/generate/-1 1>>/tmp/test-1.log 2>>/tmp/test-1.err
echo "Test 5 /api/1/" >> /tmp/test-1.log
curl -H "Content-Type: application/json" -X GET http://localhost:3000/api/1/ 1>>/tmp/test-1.log 2>>/tmp/test-1.err

echo "Results:"
cat /tmp/test-1.log

