/** CSS lexer
  * Reference:
  * https://www.w3.org/TR/css-syntax-3/ */
module Types = Css_types;
module Parser = Css_parser;
module Location = Ppxlib.Location;

/** Signals a lexing error at the provided source location. */
exception LexingError((Lexing.position, string));

/* Regexes */
let newline = [%sedlex.regexp? '\n' | "\r\n" | '\r' | '\012'];

let whitespace = [%sedlex.regexp? " " | '\t' | newline];

let whitespaces = [%sedlex.regexp? Star(whitespace)];

let digit = [%sedlex.regexp? '0' .. '9'];

let hex_digit = [%sedlex.regexp? digit | 'A' .. 'F' | 'a' .. 'f'];

let up_to_6_hex_digits = [%sedlex.regexp? Rep(hex_digit, 1 .. 6)];

let unicode = [%sedlex.regexp? ('\\', up_to_6_hex_digits, Opt(whitespace))];

let unicode_range = [%sedlex.regexp?
  Rep(hex_digit | '?', 1 .. 6) |
  (up_to_6_hex_digits, '-', up_to_6_hex_digits)
];

let escape = [%sedlex.regexp? '\\'];

let alpha = [%sedlex.regexp? 'a' .. 'z' | 'A' .. 'Z'];

let non_ascii = [%sedlex.regexp? '\160' .. '\255'];

let ident_start = [%sedlex.regexp? '_' | alpha | '$' | non_ascii | escape];

let ident_char = [%sedlex.regexp?
  '_' | alpha | digit | '-' | non_ascii | escape
];

let ident = [%sedlex.regexp?
  (Opt('-'), Opt('-'), ident_start, Star(ident_char))
];

let variable_ident_char = [%sedlex.regexp?
  '_' | alpha | digit | non_ascii | escape | '\''
];
let variable_name = [%sedlex.regexp? Star(variable_ident_char)];
let module_variable = [%sedlex.regexp? (variable_name, '.')];
let variable = [%sedlex.regexp?
  ('$', '(', Opt(Star(module_variable)), variable_name, ')')
];

let string_quote = [%sedlex.regexp?
  (
    '"',
    Star(Compl('\n' | '\r' | '\012' | '"') | ('\\', newline) | escape),
    '"',
  )
];

let string_apos = [%sedlex.regexp?
  (
    '\'',
    Star(Compl('\n' | '\r' | '\012' | '\'') | ('\\', newline) | escape),
    '\'',
  )
];

let string = [%sedlex.regexp? string_quote | string_apos];

let name = [%sedlex.regexp? Plus(ident_char)];

let number = [%sedlex.regexp?
  (
    Opt('-'),
    Plus(digit),
    Opt('.', Plus(digit)),
    Opt('e' | 'E', '+' | '-', Plus(digit)),
  ) |
  (Opt('-'), '.', Plus(digit), Opt('e' | 'E', '+' | '-', Plus(digit)))
];

let operator = [%sedlex.regexp? "~=" | "|=" | "^=" | "$=" | "*=" | "="];

let combinator = [%sedlex.regexp? '>' | '+' | '~' | "||"];

let at_rule_without_body = [%sedlex.regexp?
  ("@", "charset" | "import" | "namespace")
];
let at_rule = [%sedlex.regexp? ("@", ident)];
let at_media = [%sedlex.regexp? ("@", "media")];
let at_keyframes = [%sedlex.regexp? ("@", "keyframes")];

let non_ascii_code_point = [%sedlex.regexp? Sub(any, '\000' .. '\128')]; // greater than \u0080

let identifier_start_code_point = [%sedlex.regexp?
  alpha | non_ascii | non_ascii_code_point | '_'
];
let starts_with_a_valid_escape = [%sedlex.regexp? ('\\', Sub(any, '\n'))];

let starts_an_identifier = [%sedlex.regexp?
  ('-', '-' | identifier_start_code_point | starts_with_a_valid_escape) |
  identifier_start_code_point
];
/* Added "'" to identifier to enable Language Variables */
let identifier_code_point = [%sedlex.regexp?
  identifier_start_code_point | digit | '-' | "'"
];
let non_printable_code_point = [%sedlex.regexp?
  '\000' .. '\b' | '\011' | '\014' .. '\031' | '\127'
];

let starts_a_number = [%sedlex.regexp?
  ("+" | "-", digit) | ("+" | "-", ".", digit) | ('.', digit)
];

let is_tag =
  fun
  | "a"
  | "abbr"
  | "address"
  | "area"
  | "article"
  | "aside"
  | "audio"
  | "b"
  | "base"
  | "bdi"
  | "bdo"
  | "blockquote"
  | "body"
  | "br"
  | "button"
  | "canvas"
  | "caption"
  | "cite"
  | "code"
  | "col"
  | "colgroup"
  | "data"
  | "datalist"
  | "dd"
  | "del"
  | "details"
  | "dfn"
  | "dialog"
  | "div"
  | "dl"
  | "dt"
  | "em"
  | "embed"
  | "fieldset"
  | "figcaption"
  | "figure"
  | "footer"
  | "form"
  | "h1"
  | "h2"
  | "h3"
  | "h4"
  | "h5"
  | "h6"
  | "head"
  | "header"
  | "hgroup"
  | "hr"
  | "html"
  | "i"
  | "iframe"
  | "img"
  | "input"
  | "ins"
  | "kbd"
  | "label"
  | "legend"
  | "li"
  | "link"
  | "main"
  | "map"
  | "mark"
  | "math"
  | "menu"
  | "menuitem"
  | "meta"
  | "meter"
  | "nav"
  | "noscript"
  | "object"
  | "ol"
  | "optgroup"
  | "option"
  | "output"
  | "p"
  | "param"
  | "picture"
  | "pre"
  | "progress"
  | "q"
  | "rb"
  | "rp"
  | "rt"
  | "rtc"
  | "ruby"
  | "s"
  | "samp"
  | "script"
  | "section"
  | "select"
  | "slot"
  | "small"
  | "source"
  | "span"
  | "strong"
  | "style"
  | "sub"
  | "summary"
  | "sup"
  | "svg"
  | "table"
  | "tbody"
  | "td"
  | "template"
  | "textarea"
  | "tfoot"
  | "th"
  | "thead"
  | "time"
  | "title"
  | "tr"
  | "track"
  | "u"
  | "ul"
  | "var"
  | "video"
  | "wbr" => true
  | _ => false;

let _a = [%sedlex.regexp? 'A' | 'a'];
let _b = [%sedlex.regexp? 'B' | 'b'];
let _c = [%sedlex.regexp? 'C' | 'c'];
let _d = [%sedlex.regexp? 'D' | 'd'];
let _e = [%sedlex.regexp? 'E' | 'e'];
let _f = [%sedlex.regexp? 'F' | 'f'];
let _g = [%sedlex.regexp? 'G' | 'g'];
let _h = [%sedlex.regexp? 'H' | 'h'];
let _i = [%sedlex.regexp? 'I' | 'i'];
let _j = [%sedlex.regexp? 'J' | 'j'];
let _k = [%sedlex.regexp? 'K' | 'k'];
let _l = [%sedlex.regexp? 'L' | 'l'];
let _m = [%sedlex.regexp? 'M' | 'm'];
let _n = [%sedlex.regexp? 'N' | 'n'];
let _o = [%sedlex.regexp? 'O' | 'o'];
let _p = [%sedlex.regexp? 'P' | 'p'];
let _q = [%sedlex.regexp? 'Q' | 'q'];
let _r = [%sedlex.regexp? 'R' | 'r'];
let _s = [%sedlex.regexp? 'S' | 's'];
let _t = [%sedlex.regexp? 'T' | 't'];
let _u = [%sedlex.regexp? 'U' | 'u'];
let _v = [%sedlex.regexp? 'V' | 'v'];
let _w = [%sedlex.regexp? 'W' | 'w'];
let _x = [%sedlex.regexp? 'X' | 'x'];
let _y = [%sedlex.regexp? 'Y' | 'y'];
let _z = [%sedlex.regexp? 'Z' | 'z'];

let important = [%sedlex.regexp?
  ("!", whitespaces, _i, _m, _p, _o, _r, _t, _a, _n, _t)
];

let length = [%sedlex.regexp?
  (_c, _a, _p) | (_c, _h) | (_e, _m) | (_e, _x) | (_i, _c) | (_l, _h) |
  (_r, _e, _m) |
  (_r, _l, _h) |
  (_v, _h) |
  (_v, _w) |
  (_v, _i) |
  (_v, _b) |
  (_v, _m, _i, _n) |
  (_v, _m, _a, _x) |
  (_c, _m) |
  (_m, _m) |
  _q |
  (_i, _n) |
  (_p, _c) |
  (_p, _t) |
  (_p, _x) |
  (_f, _r)
];

let angle = [%sedlex.regexp?
  (_d, _e, _g) | (_g, _r, _a, _d) | (_r, _a, _d) | (_t, _u, _r, _n)
];

let time = [%sedlex.regexp? _s | (_m, _s)];

let frequency = [%sedlex.regexp? (_h, _z) | (_k, _h, _z)];

// https://drafts.csswg.org/css-syntax-3/#starts-with-a-valid-escape
let check_if_two_code_points_are_a_valid_escape = lexbuf =>
  switch%sedlex (lexbuf) {
  | ("\\", '\n') => false
  | ("\\", any) => true
  | _ => false
  };

// https://drafts.csswg.org/css-syntax-3/#would-start-an-identifier
let check_if_three_codepoints_would_start_an_identifier = lexbuf => {
  switch%sedlex (lexbuf) {
  | ('-', identifier_start_code_point | '-') => true
  // TODO: test the code_points case
  | '-' => check_if_two_code_points_are_a_valid_escape(lexbuf)
  | identifier_start_code_point => true
  | _ => check_if_two_code_points_are_a_valid_escape(lexbuf)
  };
};

// https://drafts.csswg.org/css-syntax-3/#starts-with-a-number
let check_if_three_code_points_would_start_a_number = buf =>
  switch%sedlex (buf) {
  | ("+" | "-", digit)
  | ("+" | "-", ".", digit) => true
  | ('.', digit) => true
  | _ => false
  };

let check = (f, buf) => {
  // TODO: why this second int?
  Sedlexing.mark(buf, 0);
  let value = f(buf);
  let _ = Sedlexing.backtrack(buf);
  value;
};

let string_of_uchar = char => {
  let buf = Buffer.create(0);
  Buffer.add_utf_8_uchar(buf, char);
  Buffer.contents(buf);
};

let uchar_of_int = n => Uchar.of_int(n) |> string_of_uchar;

let is_surrogate = char_code => char_code >= 0xD800 && char_code <= 0xDFFF;

let unreachable = () =>
  failwith(
    "This match case is unreachable. sedlex needs a last case as wildcard _. If this error appears, means that there's a bug in the lexer.",
  );

let (~:) = f => f();

let (let.ok) = Result.bind;

let consume_whitespace = buf =>
  switch%sedlex (buf) {
  | Star(whitespace) => Parser.WS
  | _ => Parser.WS
  };

let lexeme = Sedlexing.Utf8.lexeme;

// https://drafts.csswg.org/css-syntax-3/#consume-an-escaped-code-point
let consume_escaped = buf => {
  switch%sedlex (buf) {
  // TODO: spec typo? No more than 5?
  | Rep(hex_digit, 1 .. 6) =>
    let hex_string = "0x" ++ lexeme(buf);
    let char_code = int_of_string(hex_string);
    let char = uchar_of_int(char_code);
    let _ = consume_whitespace(buf);
    char_code == 0 || is_surrogate(char_code)
      // U+FFFD is a character used as a substitute for an uninterpretable character from another encoding
      ? Error(("U+FFFD", Tokens.Invalid_code_point)) : Ok(char);
  | eof => Error(("U+FFFD", Tokens.Eof))
  | any => Ok(lexeme(buf))
  | _ => ~:unreachable
  };
};

// https://drafts.csswg.org/css-syntax-3/#consume-name
let consume_identifier = buf => {
  let rec read = acc =>
    switch%sedlex (buf) {
    | identifier_code_point => read(acc ++ lexeme(buf))
    | escape =>
      // TODO: spec, what should happen when fails?
      let.ok char = consume_escaped(buf);
      read(acc ++ char);
    | _ => Ok(acc)
    };
  read(lexeme(buf));
};

// https://drafts.csswg.org/css-syntax-3/#consume-remnants-of-bad-url
let rec consume_remnants_bad_url = buf =>
  switch%sedlex (buf) {
  | ")"
  | eof => ()
  | escape =>
    let _ = consume_escaped(buf);
    consume_remnants_bad_url(buf);
  | any => consume_remnants_bad_url(buf)
  | _ => ~:unreachable
  };

module Tokenizer = {
  // https://drafts.csswg.org/css-syntax-3/#consume-url-token
  let consume_url = buf => {
    let _ = consume_whitespace(buf);
    let rec read = acc => {
      let when_whitespace = () => {
        let _ = consume_whitespace(buf);
        switch%sedlex (buf) {
        | ')' => Ok(Parser.URL(acc))
        | eof => Error((Parser.URL(acc), Tokens.Eof))
        | _ =>
          consume_remnants_bad_url(buf);
          Ok(BAD_URL);
        };
      };
      switch%sedlex (buf) {
      | ')' => Ok(Parser.URL(acc))
      | eof => Error((Parser.URL(acc), Tokens.Eof))
      | whitespace => when_whitespace()
      | '"'
      | '\''
      | '('
      | non_printable_code_point =>
        consume_remnants_bad_url(buf);
        // TODO: location on error
        Error((BAD_URL, Tokens.Invalid_code_point));
      | escape =>
        switch (consume_escaped(buf)) {
        | Ok(char) => read(acc ++ char)
        | Error((_, error)) => Error((BAD_URL, error))
        }
      | any => read(acc ++ lexeme(buf))
      | _ => ~:unreachable
      };
    };
    read(lexeme(buf));
  };

  let handle_consume_identifier =
    fun
    | Error((_, error)) => Error((Parser.BAD_IDENT, error))
    | Ok(string) => Ok(string);

  let consume_function = string => {
    switch (string) {
    | "nth-last-child"
    | "nth-child"
    | "nth-of-type"
    | "nth-last-of-type" => Parser.NTH_FUNCTION(string)
    | _ => Parser.FUNCTION(string)
    };
  };

  // https://drafts.csswg.org/css-syntax-3/#consume-ident-like-token
  let consume_ident_like = buf => {
    let read_url = string => {
      // TODO: the whitespace trickery here?
      let _ = consume_whitespace(buf);
      let is_function =
        check(_ =>
          switch%sedlex (buf) {
          | '\''
          | '"' => true
          | _ => false
          }
        );
      is_function(buf) ? Ok(consume_function(string)) : consume_url(buf);
    };

    let.ok string = consume_identifier(buf) |> handle_consume_identifier;
    switch%sedlex (buf) {
    | "(" =>
      switch (string) {
      | "url" => read_url(string)
      | _ => Ok(consume_function(string))
      }
    | _ => is_tag(string) ? Ok(TAG(string)) : Ok(IDENT(string))
    };
  };
};

let handle_tokenizer_error = buf => {
  let (_, curr_pos) = Sedlexing.lexing_positions(buf);
  fun
  | Ok(value) => value
  | Error((_, msg)) => {
      let error: string = Tokens.show_error(msg);
      raise @@ LexingError((curr_pos, error));
    };
};

let skip_whitespace = ref(false);

let latin1 = (~skip=0, ~drop=0, lexbuf) => {
  let len = Sedlexing.lexeme_length(lexbuf) - skip - drop;
  Sedlexing.Latin1.sub_lexeme(lexbuf, skip, len);
};

let rec get_next_token = buf => {
  switch%sedlex (buf) {
  | eof => Parser.EOF
  | "/*" => discard_comments(buf)
  | '.' => DOT
  | ';' => SEMI_COLON
  | '}' =>
    skip_whitespace.contents = false;
    RIGHT_BRACE;
  | '{' =>
    skip_whitespace.contents = true;
    LEFT_BRACE;
  | "::" => DOUBLE_COLON
  | ':' => COLON
  | '(' => LEFT_PAREN
  | ')' => RIGHT_PAREN
  | '[' => LEFT_BRACKET
  | ']' => RIGHT_BRACKET
  | '%' => PERCENT
  | '&' =>
    skip_whitespace.contents = false;
    AMPERSAND;
  | '*' => ASTERISK
  | ',' => COMMA
  | variable =>
    INTERPOLATION(
      latin1(~skip=2, ~drop=1, buf) |> String.split_on_char('.'),
    )
  | operator => OPERATOR(latin1(buf))
  | combinator => COMBINATOR(latin1(buf))
  | string => STRING(latin1(~skip=1, ~drop=1, buf))
  | important => IMPORTANT
  | at_media =>
    skip_whitespace.contents = false;
    AT_MEDIA(latin1(~skip=1, buf));
  | at_keyframes =>
    skip_whitespace.contents = false;
    AT_KEYFRAMES(latin1(~skip=1, buf));
  | at_rule =>
    skip_whitespace.contents = false;
    AT_RULE(latin1(~skip=1, buf));
  | at_rule_without_body =>
    skip_whitespace.contents = false;
    AT_RULE_STATEMENT(latin1(~skip=1, buf));
  /* NOTE: should be placed above ident, otherwise pattern with
   * '-[0-9a-z]{1,6}' cannot be matched */
  | (_u, '+', unicode_range) => UNICODE_RANGE(latin1(buf))
  | ('#', name) => HASH(latin1(~skip=1, buf))
  | number => get_dimension(latin1(buf), buf)
  | whitespaces =>
    if (skip_whitespace^) {
      get_next_token(buf);
    } else {
      WS;
    }
  /* | "n" => N */
  | ("-", ident) => IDENT(latin1(buf))
  /* --variable */
  | ("-", "-", ident) => IDENT(latin1(buf))
  | identifier_start_code_point =>
    let _ = Sedlexing.backtrack(buf);
    Tokenizer.consume_ident_like(buf) |> handle_tokenizer_error(buf);
  | any => DELIM(latin1(buf))
  | _ => assert(false)
  };
}
and get_dimension = (n, buf) => {
  open Sedlexing.Utf8;
  switch%sedlex (buf) {
  | length => FLOAT_DIMENSION((n, lexeme(buf)))
  | angle => FLOAT_DIMENSION((n, lexeme(buf)))
  | time => FLOAT_DIMENSION((n, lexeme(buf)))
  | frequency => FLOAT_DIMENSION((n, lexeme(buf)))
  | 'n' => DIMENSION((n, lexeme(buf)))
  | _ => NUMBER(n)
  };
}
and discard_comments = buf => {
  let (_, curr_pos) = Sedlexing.lexing_positions(buf);
  switch%sedlex (buf) {
  | "*/" => get_next_token(buf)
  | any => discard_comments(buf)
  | eof =>
    raise(
      LexingError((
        curr_pos,
        "Unterminated comment at the end of the string",
      )),
    )
  | _ => get_next_token(buf)
  };
};

let get_next_tokens_with_location = buf => {
  let (_, position_end) = Sedlexing.lexing_positions(buf);
  let token = get_next_token(buf);
  let (_, position_end_after) = Sedlexing.lexing_positions(buf);

  (token, position_end, position_end_after);
};

module Consume = {
  open Sedlexing.Utf8;
  open Tokens;

  let check_if_three_codepoints_would_start_an_identifier =
    check(check_if_three_codepoints_would_start_an_identifier);
  let check_if_three_code_points_would_start_a_number =
    check(check_if_three_code_points_would_start_a_number);

  // TODO: floats in OCaml are compatible with numbers in CSS?
  let convert_string_to_number = str => float_of_string(str);

  let consume_whitespace_ = buf =>
    switch%sedlex (buf) {
    | Star(whitespace) => Tokens.WS
    | _ => Tokens.WS
    };

  // TODO: check 5. without the 0
  let consume_number = lexbuf => {
    let append = repr => repr ++ lexeme(lexbuf);

    let kind = `Integer; // 1
    let repr = "";
    let repr =
      switch%sedlex (lexbuf) {
      | (Opt("+" | "-"), Plus(digit)) => append(repr)
      | _ => repr
      }; // 2 - 3
    let (kind, repr) =
      switch%sedlex (lexbuf) {
      | (".", Plus(digit)) => (`Number, append(repr))
      | _ => (kind, repr)
      }; // 4
    let (kind, repr) =
      switch%sedlex (lexbuf) {
      | ('E' | 'e', Opt('+' | '-'), Plus(digit)) => (
          `Number,
          append(repr),
        )
      | _ => (kind, repr)
      }; // 5
    let value = convert_string_to_number(repr); // 6
    (value, kind); // 7
  };

  // https://drafts.csswg.org/css-syntax-3/#consume-url-token
  let consume_url_ = lexbuf => {
    let _ = consume_whitespace_(lexbuf);
    let rec read = acc => {
      let when_whitespace = () => {
        let _ = consume_whitespace_(lexbuf);
        switch%sedlex (lexbuf) {
        | ')' => Ok(Tokens.URL(acc))
        | eof => Error((Tokens.URL(acc), Tokens.Eof))
        | _ =>
          consume_remnants_bad_url(lexbuf);
          Ok(BAD_URL);
        };
      };
      switch%sedlex (lexbuf) {
      | ')' => Ok(Tokens.URL(acc))
      | eof => Error((Tokens.URL(acc), Tokens.Eof))
      | whitespace => when_whitespace()
      | '"'
      | '\''
      | '('
      | non_printable_code_point =>
        consume_remnants_bad_url(lexbuf);
        // TODO: location on error
        Error((BAD_URL, Tokens.Invalid_code_point));
      | escape =>
        switch (consume_escaped(lexbuf)) {
        | Ok(char) => read(acc ++ char)
        | Error((_, error)) => Error((BAD_URL, error))
        }
      | any => read(acc ++ lexeme(lexbuf))
      | _ => ~:unreachable
      };
    };
    read(lexeme(lexbuf));
  };

  // https://drafts.csswg.org/css-syntax-3/#consume-string-token
  // TODO: when EOF is bad-string-token or string-token
  // TODO: currently it is a little bit different than the specification
  let consume_string = (ending_code_point, lexbuf) => {
    let rec read = acc => {
      // TODO: fix sedlex nested problem
      let read_escaped = () =>
        switch%sedlex (lexbuf) {
        | eof => Error((acc, Tokens.Eof))
        | '\n' => read(acc)
        | _ =>
          // TODO: is that tail call recursive? I'm not sure
          let.ok char = consume_escaped(lexbuf);
          read(acc ++ char);
        };
      switch%sedlex (lexbuf) {
      | '\''
      | '"' =>
        let code_point = lexeme(lexbuf);
        code_point == ending_code_point
          ? Ok(acc) : read(acc ++ lexeme(lexbuf));
      | eof => Error((acc, Eof))
      | newline => Error((acc, New_line))
      | escape => read_escaped()
      | any => read(acc ++ lexeme(lexbuf))
      | _ => failwith("should be unreachable")
      };
    };

    switch (read("")) {
    | Ok(string) => Ok(Tokens.STRING(string))
    | Error((string, error)) => Error((Tokens.STRING(string), error))
    };
  };

  let handle_consume_identifier_ =
    fun
    | Error((_, error)) => Error((Tokens.BAD_IDENT, error))
    | Ok(string) => Ok(string);

  // https://drafts.csswg.org/css-syntax-3/#consume-ident-like-token
  let consume_ident_like_ = lexbuf => {
    let read_url = string => {
      // TODO: the whitespace trickery here?
      let _ = consume_whitespace_(lexbuf);
      let is_function =
        check(lexbuf =>
          switch%sedlex (lexbuf) {
          | '\''
          | '"' => true
          | _ => false
          }
        );
      is_function(lexbuf)
        ? Ok(Tokens.FUNCTION(string)) : consume_url_(lexbuf);
    };

    // TODO: should it return IDENT() when error?
    let.ok string = consume_identifier(lexbuf) |> handle_consume_identifier_;

    switch%sedlex (lexbuf) {
    | '(' =>
      switch (string) {
      | "url" => read_url(string)
      | _ => Ok(FUNCTION(string))
      }
    | _ => Ok(IDENT(string))
    };
  };

  // https://drafts.csswg.org/css-syntax-3/#consume-numeric-token
  let consume_numeric = lexbuf => {
    // TODO: kind matters?
    let (number, _kind) = consume_number(lexbuf);
    if (check_if_three_codepoints_would_start_an_identifier(lexbuf)) {
      // TODO: should it be BAD_IDENT?
      let.ok string =
        consume_identifier(lexbuf) |> handle_consume_identifier_;
      Ok(Tokens.DIMENSION(number, string));
    } else {
      switch%sedlex (lexbuf) {
      | '%' => Ok(PERCENTAGE(number))
      | _ => Ok(NUMBER(number))
      };
    };
  };

  let consume = lexbuf => {
    let consume_hash = () =>
      switch%sedlex (lexbuf) {
      | identifier_code_point
      | starts_with_a_valid_escape =>
        Sedlexing.rollback(lexbuf);
        switch%sedlex (lexbuf) {
        | identifier_start_code_point =>
          Sedlexing.rollback(lexbuf);
          let.ok string =
            consume_identifier(lexbuf) |> handle_consume_identifier_;
          Ok(Tokens.HASH(string, `ID));
        | _ =>
          let.ok string =
            consume_identifier(lexbuf) |> handle_consume_identifier_;
          Ok(Tokens.HASH(string, `UNRESTRICTED));
        };
      | _ => Ok(DELIM("#"))
      };
    let consume_minus = () =>
      switch%sedlex (lexbuf) {
      | starts_a_number =>
        Sedlexing.rollback(lexbuf);
        consume_numeric(lexbuf);
      | starts_an_identifier =>
        Sedlexing.rollback(lexbuf);
        consume_ident_like_(lexbuf);
      | _ =>
        let _ = Sedlexing.next(lexbuf);
        Ok(DELIM("-"));
      };
    switch%sedlex (lexbuf) {
    | whitespace => Ok(consume_whitespace_(lexbuf))
    | "\"" => consume_string("\"", lexbuf)
    | "#" => consume_hash()
    | "'" => consume_string("'", lexbuf)
    | "(" => Ok(LEFT_PAREN)
    | ")" => Ok(RIGHT_PAREN)
    | "+" =>
      let _ = Sedlexing.backtrack(lexbuf);
      if (check_if_three_code_points_would_start_a_number(lexbuf)) {
        consume_numeric(lexbuf);
      } else {
        let _ = Sedlexing.next(lexbuf);
        Ok(DELIM("+"));
      };
    | "," => Ok(COMMA)
    | "-" =>
      Sedlexing.rollback(lexbuf);
      consume_minus();
    | "." =>
      let _ = Sedlexing.backtrack(lexbuf);
      if (check_if_three_code_points_would_start_a_number(lexbuf)) {
        consume_numeric(lexbuf);
      } else {
        let _ = Sedlexing.next(lexbuf);
        Ok(DELIM("."));
      };
    | ":" => Ok(COLON)
    | ";" => Ok(SEMI_COLON)
    | "<" => Ok(DELIM("<"))
    | "@" =>
      if (check_if_three_codepoints_would_start_an_identifier(lexbuf)) {
        // TODO: grr BAD_IDENT
        let.ok string =
          consume_identifier(lexbuf) |> handle_consume_identifier_;
        Ok(Tokens.AT_KEYWORD(string));
      } else {
        Ok(DELIM("@"));
      }
    | "[" => Ok(LEFT_BRACKET)
    | "\\" =>
      Sedlexing.rollback(lexbuf);
      switch%sedlex (lexbuf) {
      | starts_with_a_valid_escape =>
        Sedlexing.rollback(lexbuf);
        consume_ident_like_(lexbuf);
      // TODO: this error should be different
      | _ => Error((DELIM("/"), Invalid_code_point))
      };
    | "]" => Ok(RIGHT_BRACKET)
    | digit =>
      let _ = Sedlexing.backtrack(lexbuf);
      consume_numeric(lexbuf);
    | identifier_start_code_point =>
      let _ = Sedlexing.backtrack(lexbuf);
      consume_ident_like_(lexbuf);
    | eof => Ok(EOF)
    | any => Ok(DELIM(lexeme(lexbuf)))
    | _ => ~:unreachable
    };
  };
};

type token_with_location = {
  txt: result(Tokens.token, (Tokens.token, Tokens.error)),
  loc: Location.t,
};

// TODO: that's definitly ugly
/* TODO: Use lex_buffer from parser to keep track of the file */
let from_string = string => {
  let buf = Sedlexing.Utf8.from_string(string);
  let rec read = acc => {
    let (loc_start, _) = Sedlexing.lexing_positions(buf);
    let value = Consume.consume(buf);
    let (_, loc_end) = Sedlexing.lexing_positions(buf);

    let token_with_loc: token_with_location = {
      txt: value,
      loc: {
        loc_start,
        loc_end,
        loc_ghost: false,
      },
    };

    let acc = [token_with_loc, ...acc];
    switch (value) {
    | Ok(EOF) => Ok(acc)
    | _ when loc_start.pos_cnum == loc_end.pos_cnum => Error(`Frozen)
    | _ => read(acc)
    };
  };

  read([]);
};
