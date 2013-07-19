define(['log', 'test'],
function(log, test)
{
  var pattern = {};

  var variable = pattern.variable = (function(){
    /** @constructor */
    var Variable = function Variable (name) {
      this.name = name;
    };
    
    Variable.prototype.toString = function () {
      return 'Variable(' + this.name + ')';
    };

    return function (name) {
      return new Variable(name);
    };
  })();

  var isVariable = function (thing) {
    if (thing && thing.constructor && thing.constructor.name === 'Variable') {
      return true;
    } else {
      return false;
    }
  };

  test('pattern.isVariable', function(){
    var examples = [
      [variable('a'), true],
      ['asdf', false],
      [{}, false],
      [[], false],
      [undefined, false]
    ];
    examples.forEach(function(pair){
      var thing = pair[0];
      assert(isVariable(thing) === pair[1], 'isVariable failed on ' + thing);
    });
  });

  var isPattern = (function(){
    var isPattern = function (patt, avoid) {
      if (isVariable(patt)) {
        if (avoid[patt.name] === undefined) {
          avoid[patt.name] = null;
          return true;
        } else {
          return false;
        }
      } else if (_.isString(patt)) {
        return true;
      } else if (_.isArray(patt)) {
        for (var i = 0; i < patt.length; ++i) {
          if (!isPattern(patt[i], avoid)) {
            return false;
          }
        }
        return true;
      } else {
        return false;
      }
    };
    return function (patt) {
      return isPattern(patt, {});
    };
  })();

  test('pattern.isPattern', function(){
    var examples = [
      [variable('a'), true],
      ['asdf', true],
      [{}, false],
      [[['asdf', variable('x')], variable('y')], true],
      [[variable('x'), variable('x')], false],
      [undefined, false]
    ];
    examples.forEach(function(pair){
      var thing = pair[0];
      assert(isPattern(thing) === pair[1], 'isPattern failed on ' + thing);
    });
  });

  var unify = function (patt, struct, matched) {
    if (_.isEqual(patt, struct)) {
      return matched;
    } else if (isVariable(patt)) {
      //matched = _.clone(matched);  // only needed when backtracking
      matched[patt.name] = struct;
      return matched;
    } else if (_.isArray(patt) && _.isArray(struct)) {
      if (patt.length === struct.length) {
        for (var i = 0; i < struct.length; ++i) {
          matched = unify(patt[i], struct[i], matched);
          if (matched === undefined) {
            return;
          }
        }
        return matched;
      }
    }
  };

  pattern.match = function (pattHandlers) {
    // static check
    assert(pattHandlers.length % 2 == 0, 'bad pattern,handler list');
    var lineCount = pattHandlers.length / 2;
    for (var line = 0; line < lineCount; ++line) {
      var patt = pattHandlers[2 * line];
      var handler = pattHandlers[2 * line + 1];
      assert(isPattern(patt), 'bad pattern at pos ' + line + ': ' + patt);
      assert(_.isFunction(handler), 'bad handler at line ' + line);
    }
    // run optimized
    return function (struct) {
      for (var line = 0; line < lineCount; ++line) {
        var patt = pattHandlers[2 * line];
        var matched = unify(patt, struct, {});
        if (matched !== undefined) {
          var handler = pattHandlers[2 * line + 1];
          return handler(matched);
        }
      }
    };
  };

  return pattern;
});
