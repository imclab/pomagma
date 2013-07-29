define(['log', 'test'],
function(log, test)
{
  /**
   * @constructor
   * @param {string}
   * @param {(function(*): boolean) | undefined}
   */
  var Variable = function Variable (name, constraint) {
    this.name = name;
    this.constraint = (constraint !== undefined) ? constraint : null;
  };
  
  Variable.prototype.toString = function () {
    return 'Variable(' + this.name + ')';
  };

  var variable = function (name, constraint) {
    return new Variable(name, constraint);
  };

  var isVariable = function (thing) {
    return !!(thing && thing.constructor === Variable);
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
        if (_.has(avoid, patt.name)) {
          return false;
        } else {
          avoid[patt.name] = null;
          return true;
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
    assert.forward(isPattern, examples);
  });

  var unify = function (patt, struct, matched) {
    if (isVariable(patt)) {
      if (patt.constraint === null || patt.constraint(struct)) {
        matched = _.extend({}, matched);  // copy to allow backtracking
        matched[patt.name] = struct;
        return matched;
      }
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
    } else if (patt === struct) {
      return matched;
    }
  };

  var match = function (pattHandlers) {
    pattHandlers = Array.prototype.concat.apply([], arguments);
    // check statically
    assert(pattHandlers.length % 2 == 0, 'bad pattern,handler list');
    var lineCount = pattHandlers.length / 2;
    var patts = [];
    var handlers = [];
    for (var line = 0; line < lineCount; ++line) {
      var patt = pattHandlers[2 * line];
      var handler = pattHandlers[2 * line + 1];
      assert(isPattern(patt), 'bad pattern at line ' + line + ':\n  ' + patt);
      assert(_.isFunction(handler), 'bad handler at line ' + line);
      patts.push(patt);
      handlers.push(handler);
    }
    // TODO construct optimal decision tree
    // run optimized
    var slice = Array.prototype.slice;
    return function (struct) {
      for (var line = 0; line < lineCount; ++line) {
        var matched = unify(patts[line], struct, {});
        if (matched !== undefined) {
          var args = [matched].concat(slice.call(arguments, 1));
          return handlers[line].apply(this, args);
        }
      }
      throw 'Unmatched Expression:\n  ' + JSON.stringify(struct);
    };
  };

  test('pattern.match', function(){
    var x = variable('x');
    var y = variable('y');
    var z = variable('z');
    var string = variable('string', _.isString);
    var array = variable('array', _.isArray);

    var t = match([
      ['APP', 'I', x], function (m) {
        var tx = t(m.x);
        return tx;
      },
      ['APP', ['APP', 'K', x], y], function (m) {
        var tx = t(m.x);
        return tx;
      },
      ['APP', ['APP', ['APP', 'B', x], y], z], function (m) {
        var xyz = ['APP', m.x, ['APP', m.y, m.z]];
        return t(xyz);
      },
      ['APP', x, y], function (m) {
        var tx = t(m.x);
        var ty = t(m.y);
        return ['APP', tx, ty];
      },
      ['typed:', string], function (m) {
        return 'string';
      },
      ['typed:', array], function (m) {
        return 'array';
      },
      x, function (m) {
        return m.x;
      },
    ]);

    var examples = [
      [['APP', 'I', 'a'], 'a'],
      [['APP', ['APP', 'K', 'a'], 'b'], 'a'],
      [['typed:', 'test'], 'string'],
      [['typed:', []], 'array']
    ];
    assert.forward(t, examples);
  });

  return {
    variable: variable,
    unify: function (patt, struct) {
      assert(isPattern(patt), 'bad pattern: ' + patt);
      return unify(patt, struct, {});
    },
    match: match
  };
});
