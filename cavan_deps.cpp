module cavan;
import :fail;

using namespace cavan;

void cavan::take_tag(jute::view exp_id, const token *& t, hai::cstr * out) {
  t++;
  if (!match(*t, T_TEXT)) fail("expecting text inside tag");

  *out = jute::view { t->id }.cstr();

  t++;
  if (!match(*t, T_CLOSE_TAG, exp_id)) fail("missing close tag for " + exp_id);
}

[[nodiscard]] static mno::req<void> take_exclusions(const token *&t, hashley::rowan &exc) {
  hai::cstr grp{};
  hai::cstr art{};
  mno::req<void> res{};
  for (; !match(*t, T_END); t++) {
    if (match(*t, T_OPEN_TAG, "groupId")) {
      take_tag("groupId", t, &grp);
    } else if (match(*t, T_OPEN_TAG, "artifactId")) {
      take_tag("artifactId", t, &art);
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
      take_tag("groupId", t, &d.grp);
    } else if (match(*t, T_OPEN_TAG, "artifactId")) {
      take_tag("artifactId", t, &d.art);
    } else if (match(*t, T_OPEN_TAG, "version")) {
      take_tag("version", t, &d.ver);
    } else if (match(*t, T_OPEN_TAG, "scope")) {
      take_tag("scope", t, &d.scp);
    } else if (match(*t, T_OPEN_TAG, "optional")) {
      hai::cstr tmp{100};
      take_tag("optional", t, &tmp);
      res = res.map([&] { d.opt = "true"_s == tmp; });
    } else if (match(*t, T_OPEN_TAG, "classifier")) {
      take_tag("classifier", t, &d.cls);
    } else if (match(*t, T_OPEN_TAG, "type")) {
      take_tag("type", t, &d.typ);
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

mno::req<deps> cavan::list_deps(const token *&t) {
  deps res{128};
  return take_if(t, "dependencies",
                 [&] {
                   if (!match(*t, T_OPEN_TAG, "dependency"))
                     return mno::req{};

                   return take_dep(++t).map(
                       [&](auto &d) { res.push_back_doubling(d); });
                 })
      .map([&] { return traits::move(res); });
  ;
}
