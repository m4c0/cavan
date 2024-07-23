#pragma leco tool

import cavan;
import gopt;
import hai;
import jute;
import traits;
import yoyo;
import silog;

using namespace cavan;
using namespace jute::literals;

static void usage() {
  // TODO: document usage
  silog::log(silog::error, "invalid usage");
  throw 1;
}

[[nodiscard]] mno::req<void> take_tag(jute::view exp_id, const token *&t,
                                      hai::cstr *out) {
  t++;
  if (!match(*t, T_TEXT))
    return mno::req<void>::failed("expecting text inside tag");

  *out = jute::view{t->id}.cstr();

  t++;
  if (!match(*t, T_CLOSE_TAG, exp_id))
    return mno::req<void>::failed("missing close tag");

  return mno::req{};
}

mno::req<dep> take_dep(const token *&t) {
  dep d{};

  for (; t->type != T_END; t++) {
    mno::req<void> res{};
    if (match(*t, T_OPEN_TAG, "groupId")) {
      res = take_tag("groupId", t, &d.grp);
    } else if (match(*t, T_OPEN_TAG, "artifactId")) {
      res = take_tag("artifactId", t, &d.art);
    } else if (match(*t, T_OPEN_TAG, "version")) {
      res = take_tag("version", t, &d.ver);
    } else if (match(*t, T_OPEN_TAG, "scope")) {
      res = take_tag("scope", t, &d.scp);
    } else if (match(*t, T_CLOSE_TAG, "dependency")) {
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
    if (match(*t, T_OPEN_TAG, "dependencies"))
      break;
  }
  if (match(*t, T_END))
    return mno::req<deps>::failed("missing <dependencies>");

  mno::req<deps> res{deps{128}};
  for (; t->type != T_END && res.is_valid(); t++) {
    if (match(*t, T_CLOSE_TAG, "dependencies"))
      return res;

    if (!match(*t, T_OPEN_TAG, "dependency"))
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

  input.fmap(cavan::read_tokens)
      .fpeek(cavan::lint_xml)
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
