# fibsync

The fibsync project provides a simple REST based API for returning the
Fibonacci number sequence in JSON format.

# Supported API endpoints

 http://localhost:3000/
 
 Displays an example usage of using the API.

 http://localhost:3000/api/1/

 Displays if the API version is supported or not. Currently only version 1
 is supported.

 http://localhost:3000/api/1/generate/X

 Replace X with a number between 0 and 1477. It will return the sequence in
 JSON format. The supported format is:

 { "id": INTEGER, "result": CALCULATED_VALUE }

# Starting the service

Requires Node.JS version 4.5.0 (LTS) or later. To start the service:

 cd fibsync
 npm install
 node ./index.js

Then point a browser at the address displayed on the console. Or use curl:

 curl -H "Content-Type: application/json" -X GET http://localhost:3000/api/1/generate/5


