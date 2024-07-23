module cavan;

namespace cavan {
static auto blank(const char *c) {
  return *c == ' ' || *c == '\t' || *c == '\n' || *c == '\r';
}

static auto read_tag(const char *&b) {
  const char *start = b;
  while (*b && *b != '>')
    b++;

  if (!*b)
    return mno::req<token>::failed("expecting '>' got EOF");

  const char *end;
  for (end = start; end < b; end++)
    if (blank(end))
      break;

  b++; // consume '>'

  auto id = jute::view{start + 1, static_cast<unsigned>(end - start - 1)};
  return mno::req{token{id.cstr(), T_OPEN_TAG}};
}

static auto read_end_tag(const char *&b) {
  return read_tag(b).peek([](auto &t) {
    t.id = jute::view{t.id}.subview(1).after.cstr();
    t.type = T_CLOSE_TAG;
  });
}

static auto read_text(const char *&b) {
  const char *start = b;
  while (*b && *b != '<') {
    b++;
  }

  const char *end = b;
  while (blank(end) && end > start)
    end--;

  if (end == start)
    return mno::req<token>{};

  auto id = jute::view{start, static_cast<unsigned>(end - start)};
  return mno::req{token{id.cstr(), T_TEXT}};
}

mno::req<tokens> split_tokens(const hai::cstr &cstr) {
  const char *buffer = cstr.begin();

  mno::req<tokens> ts{tokens{1024}};
  while (*buffer && ts.is_valid()) {
    mno::req<token> t{};

    switch (*buffer) {
    case '<':
      t = (buffer[1] == '/') ? read_end_tag(buffer) : read_tag(buffer);
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

    ts = mno::combine(
        [](auto &ts, auto &t) {
          if (t.type != T_NULL)
            ts.push_back_doubling(traits::move(t));
          return traits::move(ts);
        },
        ts, t);
  }
  return ts.peek([](auto &ts) { ts.push_back_doubling(token{{}, T_END}); });
}

mno::req<tokens> read_tokens(yoyo::reader &r) {
  return r.size()
      .map([](auto sz) { return hai::cstr{static_cast<unsigned>(sz)}; })
      .fpeek([&](auto &buf) { return r.read(buf.begin(), buf.size()); })
      .fmap(cavan::split_tokens);
}
} // namespace cavan