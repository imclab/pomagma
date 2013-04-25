/** 
 * Abstract syntax trees
 *
 *   expr ::= var
 *          | QUOTE expr
 *          | LET patt expr expr
 *            # allows recursion when fv(patt) appear in first expr
 *          | ABSTRACT patt expr
 *          | APPLY expr expr
 *          | JOINT expr expr
 *          | COMPOSE expr expr
 *          | BOX expr
 *          | HOLE
 *          | CURSOR expr
 *
 *   patt ::= var
 *          | QUOTE patt
 *          | BOX patt
 *          | TYPE patt expr
 *          | CURSOR patt
 */

//----------------------------------------------------------------------------
// Construction

var Hole = function () {
  this.below = [];
  this.above = null;
};

var Vary = function (name) {
  this.name = name;
  this.below = [];
  this.above = null;
};

var Abstract = function (patt, body) {
  this.below = [patt, body];
  this.above = null;
  patt.above = this;
  body.above = this;
};

var Apply = function (fun, arg) {
  this.below = [fun, arg];
  this.above = null;
  fun.above = this;
  arg.above = this;
};

var Quote = function (body) {
  this.below = [body];
  this.above = null;
  body.above = this;
};

var Cursor = function (body) {
  this.below = [body];
  this.above = null;
  body.above = this;
};

//----------------------------------------------------------------------------
// Printing to polish notation

Hole.prototype.polish = function () {
  return 'HOLE';
};

Vary.prototype.polish = function () {
  return 'VARY ' + this.name;
};

Abstract.prototype.polish = function () {
  var patt = this.below[0];
  var body = this.below[1];
  return 'ABSTRACT ' + patt.polish() + ' ' + body.polish();
};

Apply.prototype.polish = function () {
  var fun = this.below[0];
  var arg = this.below[1];
  return 'APPLY ' + fun.polish() + ' ' + arg.polish();
};

Quote.prototype.polish = function () {
  var body = this.below[0]
  return 'QUOTE ' + body.polish();
};

Cursor.prototype.polish = function () {
  var body = this.below[0]
  return 'CURSOR ' + body.polish();
};

//----------------------------------------------------------------------------
// Parsing from polish notation

var Parse = function (string) {
  this.tokens = string.split(' ');
  this.pos = 0;
};

Parse.prototype.pop = function () {
  return this.tokens[this.pos++];
};

Parse.prototype.parse = function () {
  var head = this.pop();
  return parse_head[head](this);
};

var parse_head = {
  HOLE: function (state) {
    return Hole();
  },
  VARY: function(state) {
    var name = state.pop();
    return Variable(name);
  },
  ABSTRACT: function (state) {
    var patt = state.parse();
    var body = state.parse();
    return Abstract(patt, body);
  },
  APPLY: function (state) {
    var fun = state.parse();
    var arg = state.parse();
    return Apply(fun, arg);
  },
  QUOTE: function(state) {
    var body = state.parse();
    return Quote(body);
  },
  CURSOR: function(state) {
    var body = state.parse();
    return Cursor(body);
  },
};

var parse = function (string) {
  return Parse(string).parse();
};

test('Simple parsing', function(){
  var cases = [
    'VARY x',
    'QUOTE APPLY ABSTRACT CURSOR VARY x VARY x HOLE'
    ];
  for (var i = 0; i < cases.length; ++i) {
    var string = cases[i];
    var expr = parse(string);
    var polish = expr.polish();
    assertEqual(string, polish);
  }
});

//----------------------------------------------------------------------------
// Cursor movement

Cursor.prototype.remove = function () {
  var above = this.above;
  this.below[0].above = above;
  if (above) {
    var pos = above.args.indexOf(this);
    above.below[pos] = this.below[0];
    return pos;
  }
};

Cursor.prototype.insert_below = function (above, pos) {
  var arg = above.below[pos];
  expr.below[pos] = this;
  this.below[0] = arg;
  arg.above = this;
};

Cursor.prototype.insert_above = function (below) {
  this.below[0] = below;
  var above = below.above;
  below.above = this;
  this.above = above;
  if (above !== null) {
    var pos = above.args.indexOf(below);
    above.below.args[pos] = this;
  }
};

Cursor.prototype.try_move_down = function () {
  var pivot = this.below[0];
  if (pivot.below.length === 0) {
    return false;
  } else {
    this.remove();
    this.insert_below(pivot, 0);
    return true;
  }
};

Cursor.prototype.try_move_up = function () {
  var pivot = this.above;
  if (pivot === null) {
    return false;
  } else {
    this.remove();
    var above = pivot.above;
    if (above === null) {
      this.insert_above(pivot);
    } else {
      var pos = above.below.indexOf(pivot);
      this.insert_below(above, pos);
    }
    return true;
  }
};

Cursor.prototype.try_move_sideways = function (direction) {
  var pivot = this.above;
  if ((pivot === null) || (pivot.args.length === 1)) {
    return false;
  } else {
    var pos = this.remove();
    pos = (pos + direction + pivot.below.length) % pivot.below.length;
    this.insert_below(pivot, pos);
    return true;
  }
};

Cursor.prototype.try_move_left = function () {
  return this.try_move_sideways(-1);
};

Cursor.prototype.try_move_right = function () {
  return this.try_move_sideways(+1);
};

Cursor.prototype.try_move = function (direction) {
  switch (direction) {
    case 'U': return this.try_move_up();
    case 'D': return this.try_move_down();
    case 'L': return this.try_move_left();
    case 'R': return this.try_move_right();
  }
};

test('Cursor movement', function(){
  var start = 'ABSTRACT QUOTE VAR this APPLY VAR is APPLY VAR a VAR test';
  var expr = parse(start);
  var cursor = Cursor(expr);
  var path =  'UDDDULRUL';
  var trace = '011010011';
  for (var i = 0; i < path.length; ++i) {
    assertEqual(cursor.try_move(path[i]), trace[i]);
  }
});

//----------------------------------------------------------------------------
// Focus

var PointedExpression = function (root) {
  assert(root.above == null);
  this.cursor = Cursor(root);
  this.root = this.cursor;
};

//----------------------------------------------------------------------------
// Transformations

// TODO
