require([], function () {

  var main = function () {
    // TODO
  };

  if (window.location.hash && window.location.hash.substr(1) === 'test') {

    document.title = 'Pomagma Editor Test';
    test.runAll(function(){
          document.title = 'Pomagma Editor';
          window.location.hash = '';
          main();
        });

  } else {

    main();

  }

});
