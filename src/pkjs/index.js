// Import the Clay package
var Clay = require('pebble-clay');
// Load our Clay configuration file
var clayConfig = require('./config');
// Initialize Clay. This gives us an error but we need it for clay
// settings.
var clay = new Clay(clayConfig);

/*
Pebble.addEventListener('ready', function() {
  // PebbleKit JS is ready!
  console.log('PebbleKit JS has started.');
});
*/