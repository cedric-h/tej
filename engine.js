export function wrapper({
  temp_str,
  memory,

  bitmap_set_from_str,
  pattern_set,

  solid_set,

  sprite_set_push,
  sprite_move,

  map_set,
  map_index,
  map_match,

  map_match_x,
  map_match_y,
}) {
  const mem = new Uint8Array(memory.buffer, temp_str, 1 << 11);
  const encoder = new TextEncoder();
  function setTempStr(str) {
    mem.fill(0);
    encoder.encodeInto(str.trim(), mem);
    return temp_str;
  }

  function legendEntry([char, lhs]) {
    if (typeof lhs == "string")
      bitmap_set_from_str(char.charCodeAt(0), setTempStr(lhs));
    else
      pattern_set(char.charCodeAt(0), lhs.kind, setTempStr(lhs.str));
  };

  class Tile {
    constructor(x, y) {
      this._x = x;
      this._y = y;
    }

    set x(n) {
      sprite_move(map_index(this._x, this._y), this._x - n, 0);
      return this._x = n;
    }
    set y(n) {
      sprite_move(map_index(this._x, this._y), 0, this._y - n);
      return this._y = n;
    }

    get x() { return this._x; }
    get y() { return this._y; }
  }

  let afterInputFn = () => {};
  return {
    setLegend: obj => Object.entries(obj).forEach(legendEntry),
    setSolids: arr => arr.forEach(x => solid_set(x.charCodeAt(0))),
    setPushables: obj => {
      for (const [k, v] of Object.entries(obj))
        for (const pushable of v)
          sprite_set_push(
                   k.charCodeAt(0),
            pushable.charCodeAt(0)
          );
    },
    setMap: str => map_set(setTempStr(str)),
    getFirst: char => {
      map_match(
        setTempStr(""), /* this is cursed */
        char.charCodeAt(0)
      );
      return new Tile(
        map_match_x(temp_str),
        map_match_y(temp_str),
      );
    },
    match: pattern => {
      const ret = [];
      setTempStr("");
      while (map_match(
        temp_str,
        pattern.charCodeAt(0)
      )) ret.push(new Tile(
        map_match_x(temp_str),
        map_match_y(temp_str),
      ));
      return ret;
    },
    afterInput: fn => afterInputFn = fn,
    onInput: (() => {
      const handlers = {};
      window.onkeydown = e => {
        const fn = handlers[{
          "w": "up",
          "s": "down",
          "a": "left",
          "d": "right",
        }[e.key]];
        if (fn) fn();
        afterInputFn();
      };
      return (key, fn) => handlers[key] = fn;
    })(),
    anyOf: (...args) => ({ kind: 1, str: args.join('') }),
    allOf: (...args) => ({ kind: 0, str: args.join('') }),
  }
}
