module cavan;
import silog;

namespace cavan {
static inline void fail(const char * msg) {
  silog::log(silog::error, "Failed to tokenise: %s", msg);
  throw error {};
}
template<auto N>
static inline void fail(jute::twine<N> msg) { fail(msg.cstr().begin()); }

static auto blank(const char *c) {
  return *c == ' ' || *c == '\t' || *c == '\n' || *c == '\r';
}

static auto read_tag(const char *&b) {
  const char *start = b;
  while (*b && *b != '>')
    b++;

  if (!*b) fail("expecting '>' got EOF");

  const char *end;
  for (end = start; end < b; end++)
    if (blank(end))
      break;

  auto type = (b[-1] == '/') ? T_TAG : T_OPEN_TAG;

  b++; // consume '>'

  auto id = jute::view{start + 1, static_cast<unsigned>(end - start - 1)};
  return token { id.cstr(), type };
}

static auto read_end_tag(const char *&b) {
  auto t = read_tag(b);
  t.id = jute::view{t.id}.subview(1).after.cstr();
  t.type = T_CLOSE_TAG;
  return t;
}

static auto read_directive(const char *&b) {
  auto t = read_tag(b);
  t.id = jute::view{t.id}.subview(1).after.cstr();
  t.type = T_DIRECTIVE;
  return t;
}

static auto read_comment(const char *&b) {
  jute::view prefix{b, 4};
  if (prefix != "<!--") fail("could not get comment opening around ["_s + prefix + "]");

  b += 3;
  while (*++b) {
    if (*b != '-')
      continue;

    if (jute::view{b, 3} != "-->")
      continue;

    b += 3;
    return token {};
  }

  fail("missing end of comment");
  return token {};
}

static auto read_text(const char *&b) {
  const char *start = b;
  while (*b && *b != '<') {
    b++;
  }

  const char *end = b;
  while (blank(end) && end > start)
    end--;

  if (end == start) return token {};

  auto id = jute::view{start, static_cast<unsigned>(end - start)};
  return token { id.cstr(), T_TEXT };
}

static auto read_tagish(const char *&b) {
  switch (b[1]) {
  case '?':
    return read_directive(b);
  case '/':
    return read_end_tag(b);
  case '!':
    return read_comment(b);
  default:
    return read_tag(b);
  }
}

tokens split_tokens(const hai::cstr &cstr) {
  const char *buffer = cstr.begin();

  tokens ts { 1024 };
  while (*buffer) {
    token t {};

    switch (*buffer) {
    case '<':
      t = read_tagish(buffer);
      break;
    case ' ':
    case '\t':
    case '\r':
    case '\n':
      buffer++;
      continue;
    default:
      t = read_text(buffer);
      break;
    }

    if (t.type != T_NULL) ts.push_back_doubling(traits::move(t));
  }
  
  ts.push_back_doubling(token { {}, T_END });
  return ts;
}
} // namespace cavan
