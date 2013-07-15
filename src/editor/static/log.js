
/**
 * Tools for safe coding.
 *
 * Copyright (c) 2012, Fritz Obermeyer
 * Dual licensed under the MIT or GPL Version 2 licenses.
 * http://www.opensource.org/licenses/MIT
 * http://www.opensource.org/licenses/GPL-2.0
 */

//------------------------------------------------------------------------------
// Logging in the main window & web workers

define([], function () {

  var log;
  var listeners = [];

  if (this.document) { // in main window

    if (window.console && window.console.log) {
      log = function (message) {
        listeners.forEach(function(cb){ cb(message); })
        console.log(message);
      };
    } else {
      log = function (message) {
        listeners.forEach(function(cb){ cb(message); })
      };
    }

  } else { // in a web worker

    log = function (message) {
      listeners.forEach(function(cb){ cb(message); })
      self.postMessage({'type':'log', 'data':message});
    };
  }

  log.pushListener = function (cb) {
    listeners.push(cb);
  };

  log.popListener = function (cb) {
    listeners.pop(cb);
  };

  return log;
});
