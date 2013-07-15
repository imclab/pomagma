require.config({
  'baseUrl': '/static',
  //shim: {
  //  'underscore': {
  //    exports: '_'
  //  },
  //  'jquery': {
  //    exports: '$'
  //  }
  //}
  paths: {
    'jquery': '/scripts/lib/jquery',
    'underscore': '/scripts/lib/underscore',
    //'jquery': '//ajax.googleapis.com/ajax/libs/jquery/2.0.0/jquery.min',
  }
});

require(['jquery', 'test', 'corpus'], function (jquery, test) {

  $(function(){
      test.runAll(function() {
          var parts = window.location.toString().split('/');
          parts.pop();
          window.location.assign(parts.join('/'));
      })
  });

});
