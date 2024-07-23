#pragma leco tool

import gopt;
import hai;
import jute;
import traits;
import yoyo;
import silog;

static void usage() {
  // TODO: document usage
  silog::log(silog::error, "invalid usage");
  throw 1;
}

using namespace jute::literals;

enum type {
  T_NULL,
  T_OPEN_TAG,
  T_CLOSE_TAG,
  T_TAG,
  T_TEXT,
  T_END,
};
struct token {
  hai::cstr id{};
  type type{};
};
using tokens = hai::varray<token>;

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

static auto split_tokens(const hai::cstr &cstr) {
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

[[nodiscard]] mno::req<const char *> take_tag(jute::view exp_id,
                                              const token *&t) {
  t++;
  if (t->type != T_TEXT)
    return mno::req<const char *>::failed("expecting text inside tag");

  const auto *res = t->id.begin();

  t++;
  if (t->type != T_CLOSE_TAG || exp_id != t->id)
    return mno::req<const char *>::failed("missing close tag");

  return mno::req{res};
}

mno::req<void> print_dep(const token *&t) {
  const char *grp{};
  const char *art{};
  const char *ver{};
  const char *scp{"compile"};

  for (; t->type != T_END; t++) {
    mno::req<void> res{};
    if (t->type == T_OPEN_TAG && "groupId"_s == t->id) {
      res = take_tag("groupId", t).map([&](auto g) { grp = g; });
    } else if (t->type == T_OPEN_TAG && "artifactId"_s == t->id) {
      res = take_tag("artifactId", t).map([&](auto g) { art = g; });
    } else if (t->type == T_OPEN_TAG && "version"_s == t->id) {
      res = take_tag("version", t).map([&](auto g) { ver = g; });
    } else if (t->type == T_OPEN_TAG && "scope"_s == t->id) {
      res = take_tag("scope", t).map([&](auto g) { scp = g; });
    } else if (t->type == T_CLOSE_TAG && "dependency"_s == t->id) {
      t++;
      break;
    } else {
      silog::log(silog::debug, "%d %s--", t->type, t->id.begin());
      res = mno::req<void>::failed("unknown stuff found inside depencies");
    }
    if (!res.is_valid())
      return res;
  }

  silog::log(silog::debug, "dependency: %s:%s:%s:%s", grp, art, ver, scp);
  return mno::req{};
}

mno::req<void> list_deps(const token *t) {
  mno::req count{0};
  for (; t->type != T_END && count.is_valid(); t++) {
    if (t->type != T_OPEN_TAG || "dependency"_s != t->id)
      continue;

    count = print_dep(++t).fmap([&] { return count; }).map([](auto c) {
      return c + 1;
    });
  }
  return count.map([](auto count) {
    silog::log(silog::debug, "found %d dependencies", count);
  });
}

int main(int argc, char **argv) try {
  auto input = yoyo::file_reader::std_in();
  auto opts = gopt_parse(argc, argv, "i:", [&](auto ch, auto val) {
    switch (ch) {
    case 'i':
      input = yoyo::file_reader::open(val);
      break;
    default:
      usage();
    }
  });

  if (opts.argc != 0)
    usage();

  input.fmap(yoyo::size())
      .map([](auto sz) { return hai::cstr{static_cast<unsigned>(sz)}; })
      .fpeek([&](auto &buf) {
        return input.fmap(yoyo::read(buf.begin(), buf.size()));
      })
      .peek([](auto &buffer) {
        silog::log(silog::info, "read %d bytes", buffer.size());
      })
      .fmap(split_tokens)
      .peek([](auto &tokens) {
        silog::log(silog::info, "got %d tokens", tokens.size());
      })
      .fmap([](auto &tokens) { return list_deps(tokens.begin()); })
      .log_error([] { throw 1; });
} catch (...) {
  return 1;
}
