define(['log'], function (log) {

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

  return assert;
});
