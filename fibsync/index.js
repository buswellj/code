// Application Specific Information
var vsappname = "Fibonacci Sequence API";
var vsversion = "0.0.1";
var vscopyright = "Copyright (c) 2016 John Buswell";

// Load dependencies
var express = require('express');
var fs = require('fs');
var morgan = require('morgan');

// Create instance
var app = express();

// Setup logging
var accessLogStream = fs.createWriteStream('/tmp/vsapp-access.log',{flags: 'a'});

// Configure Express to use middleware
app.use(morgan('combined', {stream: accessLogStream}));

// API Endpoints

app.get('/', function (req, res) {
 res.status(200).send('<a href="/api/1/generate/5">Use /api/1/generate/X</a> where X is a value from 0 to 1476.<br />Request Received from ' + req.ip);
});

app.get('/api/:apiver/', function (req, res) {
 if (req.params.apiver != '1') {
  res.status(404).send('Unsupported API version. Use version 1');
 } else {
  res.status(200).send('API version 1.0 request from ' + req.ip);
 }
});

app.get('/api/:apiver/generate/:fibn/', function (req, res) {
 var fibn = req.params.fibn;

 // First we check the API version number
 // We check the value is an actual number
 // We check the value is less than zero for negative number case
 // We check the value is less than 1477 (prevent abuse / exceeded type bounds)
 if (req.params.apiver != '1') {
  res.status(404).send('Error: Unsupported API version. Use version 1');
 } else if (isNaN(fibn)) {
  res.status(500).send('Error: Value must be a number');
 } else if (fibn < '0') {
  res.status(500).send('Error: Value cannot be a negative number');
 } else if (fibn > 1477) {
  res.status(403).send('Forbidden: Value must be less than 1477');
 } else {
  fscalc(req,res,fibn);
 }
});

// This function places the application on port 3000
// Throws some information on the console
var server = app.listen(3000, function () {

 var host = server.address().address;
 var port = server.address().port;

 console.log('');
 console.log('%s, version %s', vsappname, vsversion);
 console.log('%s', vscopyright);
 console.log('');
 console.log('Listening on http://%s:%s', host, port);
 console.log('');
});

// This function does basic fibonacci sequence generation
// Returns the values in JSON format using:
// { id: X, result: Y } where id is the sequence id, and result is the result
// capture special cases for first two values

var fscalc = function(req, res, fibn) {
 var fsn = 1;
 var fsnX = 0;
 var fsnY = 1;
 var fnseq = [];

 res.type('json');

 if (fibn == 0) {
  fnseq.push({ id: fsnX, result: fsnX });
 } else if (fibn == 1) {
  fnseq.push({ id: fsnX, result: fsnX });
  fnseq.push({ id: fsnY, result: fsnY });
 } else { 
  fnseq.push({ id: fsnX, result: fsnX });
  fnseq.push({ id: fsnY, result: fsnY });
  for (i = 2; i < fibn; i++) {
   fsn = fsnX + fsnY;
   fnseq.push({ id: i, result: fsn });   
   fsnX = fsnY;
   fsnY = fsn;
  }
 }
 res.json({ sequence: fnseq });
}
