module cavan;

namespace cavan {
[[nodiscard]] static mno::req<void> take_tag(jute::view exp_id, const token *&t,
                                             hai::cstr *out) {
  t++;
  if (!match(*t, T_TEXT))
    return mno::req<void>::failed("expecting text inside tag");

  *out = jute::view{t->id}.cstr();

  t++;
  if (!match(*t, T_CLOSE_TAG, exp_id))
    return mno::req<void>::failed("missing close tag for " + exp_id);

  return mno::req{};
}

[[nodiscard]] static mno::req<void> take_exclusions(const token *&t,
                                                    hashley::rowan &exc) {
  hai::cstr grp{};
  hai::cstr art{};
  mno::req<void> res{};
  for (; !match(*t, T_END); t++) {
    if (match(*t, T_OPEN_TAG, "groupId")) {
      res = take_tag("groupId", t, &grp);
    } else if (match(*t, T_OPEN_TAG, "artifactId")) {
      res = take_tag("artifactId", t, &art);
    } else if (match(*t, T_CLOSE_TAG, "exclusion")) {
      auto key = ""_hs + grp + ":" + art + "\0";
      exc[(*key).begin()] = 1;
    } else if (match(*t, T_CLOSE_TAG, "exclusions")) {
      return {};
    }
    if (!res.is_valid())
      return res.trace("parsing exclusions");
  }
  return res;
}

[[nodiscard]] static mno::req<dep> take_dep(const token *&t) {
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
    } else if (match(*t, T_OPEN_TAG, "optional")) {
      hai::cstr tmp{100};
      res = take_tag("optional", t, &tmp).map([&] { d.opt = "true"_s == tmp; });
    } else if (match(*t, T_OPEN_TAG, "exclusions")) {
      res = take_exclusions(t, d.exc);
    } else if (match(*t, T_CLOSE_TAG, "dependency")) {
      break;
    } else {
      res = mno::req<void>::failed("unknown stuff found inside dependencies");
    }
    if (!res.is_valid())
      return res.map([] { return dep{}; }).trace("parsing dependency");
  }

  // silog::log(silog::debug, "dependency: %s:%s:%s:%s", grp, art, ver, scp);
  return mno::req{traits::move(d)};
}

mno::req<deps> list_deps(const tokens &ts) {
  auto *t = ts.begin();

  for (; t->type != T_END; t++) {
    if (match(*t, T_OPEN_TAG, "plugin")) {
      while (!match(*t, T_CLOSE_TAG, "plugin"))
        t++;
      continue;
    }

    if (match(*t, T_OPEN_TAG, "dependencies"))
      break;
  }
  // No <dependencies>
  if (match(*t, T_END))
    return mno::req<deps>{};

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
  return res.is_valid() ? mno::req<deps>::failed("missing </dependencies>")
                        : traits::move(res);
}
} // namespace cavan
