module cavan;
import :fail;

namespace cavan {
  class strm {
    const char * m_begin;
    const char * m_end;

  public:
    constexpr strm(jute::view v) : m_begin { v.begin() }, m_end { v.end() } {}

    [[nodiscard]] constexpr char operator*() const { return *m_begin; }
    [[nodiscard]] constexpr char operator[](int idx) const {
      return idx >= m_end - m_begin ? 0 : m_begin[idx]; 
    }

    [[nodiscard]] constexpr const char * pointer() const { return m_begin; }
    [[nodiscard]] constexpr jute::view look_ahead(int n) const {
      int rem = m_end - m_begin;
      unsigned nn = rem > n ? n : rem;
      return { m_begin, nn };
    }

    constexpr strm & operator++() {
      if (m_begin < m_end) m_begin++;
      return *this;
    }
    constexpr strm & operator+=(int n) {
      if (m_begin + n < m_end) m_begin += n;
      return *this;
    }
  };

static auto blank(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static auto read_tag(strm & b) {
  const char *start = b.pointer();
  while (*b && *b != '>') ++b;

  if (!*b) fail("expecting '>' got EOF");

  const char *end;
  for (end = start; end < b.pointer(); end++)
    if (blank(*end))
      break;

  auto type = (b[-1] == '/') ? T_TAG : T_OPEN_TAG;

  ++b; // consume '>'

  return token {
    .text = { start + 1, static_cast<unsigned>(end - start - 1) },
    .type = type,
  };
}

static auto read_end_tag(strm & b) {
  auto t = read_tag(b);
  t.text = t.text.subview(1).after;
  t.type = T_CLOSE_TAG;
  return t;
}

static auto read_directive(strm & b) {
  auto t = read_tag(b);
  t.text = t.text.subview(1).after;
  t.type = T_DIRECTIVE;
  return token {}; //t;
}

static auto read_comment(strm & b) {
  b += 3;
  while (*++b) {
    if (*b != '-')
      continue;

    if (b.look_ahead(3) != "-->")
      continue;

    b += 3;
    return token {};
  }

  fail("missing end of comment");
  return token {};
}

static auto read_cdata(strm & b) {
  auto * start = b.pointer();

  jute::view prefix = b.look_ahead(9);
  if (prefix != "<![CDATA[") fail("could not parse around "_s + prefix);

  b += 8;
  while (*++b) {
    if (*b != ']')
      continue;

    if (b.look_ahead(3) != "]]>")
      continue;

    b += 3;
    return token {
      .text = { start, static_cast<unsigned>(b.pointer() - start) },
      .type = T_TEXT,
    };
  }

  fail("missing end of cdata");
  return token {};
}

static auto read_comment_ish(strm & b) {
  jute::view prefix = b.look_ahead(4);
  if (prefix == "<!--") return read_comment(b);
  if (prefix == "<![C") return read_cdata(b);
    
  fail("could not parse around ["_s + prefix + "]");
  return token {};
}

static auto read_text(strm & b) {
  const char *start = b.pointer();
  while (*b && *b != '<') ++b;

  const char *end = b.pointer();
  while (blank(*end) && end > start)
    end--;

  if (end == start) return token {};

  return token {
    .text = { start, static_cast<unsigned>(end - start) },
    .type = T_TEXT,
  };
}

static auto read_tagish(strm & b) {
  switch (b[1]) {
  case '?':
    return read_directive(b);
  case '/':
    return read_end_tag(b);
  case '!':
    return read_comment_ish(b);
  default:
    return read_tag(b);
  }
}

tokens split_tokens(jute::view xml) {
  strm buffer { xml }; 

  tokens ts { 10240 };
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
      ++buffer;
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
