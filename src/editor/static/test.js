/**
 * Unit testing.
 */

define(['jquery', 'log', 'assert'], function (jquery, log, assert) {

  var testing = false;
  var hasRun = false;
  var passed = false;
  var allTests = [];

  /** @const */
  var test = function (title, callback) {
    callback = callback || function(){ $.globalEval(title); };
    callback.title = title;
    allTests.push(callback);
  };

  test.testing = function () { return testing; }
  test.hasRun = function () { return hasRun; }
  test.passed = function () { return passed; }


  test.runAll = function (onExit) {

    var $log = $('<div>')
      .attr({id:'testLog'})
      .css({
            'position': 'absolute',
            'width': '100%',
            'top': '0%',
            'left': '0%',
            'text-align': 'left',
            'color': 'black',
            'background-color': 'white',
            'border': 'solid 8px white',
            'font-size': '10pt',
            'font-family': 'Courier,Courier New,Nimbus Mono L,fixed,monospace',
            'z-index': '99'
          })
      .appendTo(document.body);

    log.pushListener(function (message) {
      $('<pre>').text(message).appendTo($log);
    });

    if (onExit !== undefined) {

      $log.css({
            'width': '80%',
            'left': '10%',
            'border-radius': '16px',
            });

      var $shadow = $('<div>')
        .css({
              'position': 'fixed',
              'width': '100%',
              'height': '100%',
              'top': '0%',
              'left': '0%',
              'background-color': 'black',
              'opacity': '0.5',
              'z-index': '98'
            })
        .attr({title:'click to exit test results'})
        .click(function(){
              $log.remove();
              $shadow.remove();
              testing = false;
              onExit();
            })
        .appendTo(document.body);
    }

    log('[ Running ' + allTests.length + ' unit tests ]');
    testing = true;

    passed = true;
    var failed = [];
    for (var i = 0; i < allTests.length; ++i) {
      var callback = allTests[i];
      try {
        callback($log);
      }
      catch (err) {
        passed = false;
        log('FAILED ' + callback.title + '\n  ' + err);
        failed.push(callback);
      }
    }
    hasRun = true;

    if (failed.length) {
      log('[ Failed ' + failed.length + ' tests ]');
      $log.css({
            'background-color': '#ffaaaa',
            'border-color': '#ffaaaa'
          });
    } else {
      log('[ Passed all tests :) ]');
      $log.css({
            'background-color': '#aaffaa',
            'border-color': '#aaffaa'
          });
    }

    log.popListener();

    // call all failed tests to get stack traces
    failed.forEach(function(failedTest){
      setTimeout(failedTest, 0);
    });
  };

  return test;
});
