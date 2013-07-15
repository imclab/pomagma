define(['jquery', 'log'], function ($, log) {

  var testing = false;
  var allTests = [];

  /** @const */
  var test = function (title, callback) {
    callback = callback || function(){ $.globalEval(title); };
    callback.title = title;
    allTests.push(callback);
  };

  test.testing = function () { return testing; }

  test.runAll = function () {

    log('[ Running ' + allTests.length + ' unit tests ]');
    testing = true;

    var failed = [];
    for (var i = 0; i < allTests.length; ++i) {
      var callback = allTests[i];
      try {
        callback(log);
      }
      catch (err) {
        log('FAILED ' + callback.title + '\n  ' + err);
        failed.push(callback);
      }
    }

    if (failed.length) {
      log('[ Failed ' + failed.length + ' tests ]');
    } else {
      log('[ Passed all tests :) ]');
    }

    // call all failed tests to get stack traces
    for (var i = 0; i < failed.length; ++i) {
      (function(i){
        setTimeout(failed[i], 0);
      })(i);
    }
  };

  return test;
});
