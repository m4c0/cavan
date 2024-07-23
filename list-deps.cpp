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

struct dep {
  hai::cstr grp{};
  hai::cstr art{};
  hai::cstr ver{};
  hai::cstr scp{"compile"_s.cstr()};
};
using deps = hai::varray<dep>;

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

[[nodiscard]] mno::req<void> take_tag(jute::view exp_id, const token *&t,
                                      hai::cstr *out) {
  t++;
  if (t->type != T_TEXT)
    return mno::req<void>::failed("expecting text inside tag");

  *out = jute::view{t->id}.cstr();

  t++;
  if (t->type != T_CLOSE_TAG || exp_id != t->id)
    return mno::req<void>::failed("missing close tag");

  return mno::req{};
}

mno::req<dep> take_dep(const token *&t) {
  dep d{};

  for (; t->type != T_END; t++) {
    mno::req<void> res{};
    if (t->type == T_OPEN_TAG && "groupId"_s == t->id) {
      res = take_tag("groupId", t, &d.grp);
    } else if (t->type == T_OPEN_TAG && "artifactId"_s == t->id) {
      res = take_tag("artifactId", t, &d.art);
    } else if (t->type == T_OPEN_TAG && "version"_s == t->id) {
      res = take_tag("version", t, &d.ver);
    } else if (t->type == T_OPEN_TAG && "scope"_s == t->id) {
      res = take_tag("scope", t, &d.scp);
    } else if (t->type == T_CLOSE_TAG && "dependency"_s == t->id) {
      break;
    } else {
      res = mno::req<void>::failed("unknown stuff found inside depencies");
    }
    if (!res.is_valid())
      return res.map([] { return dep{}; });
  }

  // silog::log(silog::debug, "dependency: %s:%s:%s:%s", grp, art, ver, scp);
  return mno::req{traits::move(d)};
}

mno::req<deps> list_deps(const tokens &ts) {
  auto *t = ts.begin();

  for (; t->type != T_END; t++) {
    if (t->type == T_OPEN_TAG && "dependencies"_s == t->id)
      break;
  }
  if (t->type == T_END)
    return mno::req<deps>::failed("missing <dependencies>");

  mno::req<deps> res{deps{128}};
  for (; t->type != T_END && res.is_valid(); t++) {
    if (t->type == T_CLOSE_TAG && "dependencies"_s == t->id)
      return res;

    if (t->type != T_OPEN_TAG || "dependency"_s != t->id)
      continue;

    auto dep = take_dep(++t);
    res = mno::combine(
        [](auto &ds, auto &d) {
          ds.push_back_doubling(d);
          return traits::move(ds);
        },
        res, dep);
  }
  return mno::req<deps>::failed("missing </dependencies>");
  ;
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
      .fmap(list_deps)
      .map([](auto &deps) {
        silog::log(silog::info, "found %d dependencies", deps.size());

        for (auto &d : deps) {
          silog::log(silog::debug, "%s:%s:%s:%s", d.grp.begin(), d.art.begin(),
                     d.ver.begin(), d.scp.begin());
        }
      })
      .log_error([] { throw 1; });
} catch (...) {
  return 1;
}
