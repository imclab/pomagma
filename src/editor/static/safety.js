/**
 * Tools for safe coding.
 */

'use strict';

//----------------------------------------------------------------------------
// unfinished code

/** @constructor */
var TodoException = function (message) {
  this.message = message || '(unfinished code)';
};

TodoException.prototype.toString = function () {
  return 'TODO: ' + this.message;
};

var TODO = function (message) {
  throw new TodoException(message);
};

//----------------------------------------------------------------------------
// assertions

/** @constructor */
var AssertException = function (message) {
  this.message = message || '(unspecified)';
};

AssertException.prototype.toString = function () {
  return 'Assertion Failed: ' + this.message;
};

var assert = function (condition, message) {
  if (!condition) {
    throw new AssertException(message);
  }
};

assert.equal = function (actual, expected, message) {
  assert(_.isEqual(actual, expected),
    (message || '') +
    '\n    actual = ' + JSON.stringify(actual) +
    '\n    expected = ' + JSON.stringify(expected));
};

assert.forward = function (fwd, pairs) {
  pairs.forEach(function(pair, lineno){
    try {
      assert.equal(fwd(pair[0]), pair[1]);
    } catch (e) {
      throw new AssertException('forward example ' + (1 + lineno) + ', ' + e);
    }
  });
};

assert.backward = function (bwd, pairs) {
  pairs.forEach(function(pair, lineno){
    try {
      assert.equal(bwd(pair[1]), pair[0]);
    } catch (e) {
      throw new AssertException('backward example ' + (1 + lineno) + ', ' + e);
    }
  });
};

assert.inverses = function (fwd, bwd, items) {
  items.forEach(function(item, lineno){
    try {
      assert.equal(bwd(fwd(item)), item);
    } catch (e) {
      throw new AssertException('inverses example ' + (1 + lineno) + ', ' + e);
    }
  });
};

//----------------------------------------------------------------------------
// web workers

/** @constructor */
var WorkerException = function (message) {
  this.message = message || '(unspecified)';
};
WorkerException.prototype.toString = function () {
  return 'Worker Error: ' + this.message;
};
