requirejs = require('requirejs');
requirejs.config({
  baseUrl: './static/',
  nodeRequire: require
});

//requirejs('underscore');
//requirejs('jquery');
//requirejs('safety.js');
//requirejs('testing.js');
//requirejs('corpus.js');
//requirejs('views.js');
//requirejs('compiler.js');
//requirejs('editor.js');
//requirejs('analyst.js');
//requirejs('ui.js');

requirejs(['test', 'corpus'], function (test, corpus) {
  test.runAll();
  setTimeout(process.exit, 2000);
});
