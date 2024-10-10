module;
extern "C" char * getenv(const char *);

module cavan;
import hai;
import hashley;
import jojo;
import silog;

static class {
  hai::chain<cavan::pom> m_buffer { 1024 };
  hashley::niamh m_idx { 7919 };

public:
  cavan::pom * take(cavan::pom p) {
    auto & idx = m_idx[(p.grp + ":" + p.art + ":" + p.ver).cstr()];
    if (idx == 0) {
      m_buffer.push_back(traits::move(p));
      idx = m_buffer.size();
    }
    return &m_buffer.seek(idx - 1);
  }
  cavan::pom * get(jute::view grp, jute::view art, jute::view ver) {
    auto idx = m_idx[(grp + ":" + art + ":" + ver).cstr()];
    return idx == 0 ? nullptr : &m_buffer.seek(idx - 1);
  }
} g_cache;

static cavan::props list_props(const cavan::token *& t) {
  cavan::props res { 32 };
  take_if(t, "properties", [&] {
    jute::view key = t->text;

    if (match(*t, cavan::T_TAG)) {
      res.push_back(cavan::prop { key, {} });
      return;
    }
    if (!match(*t, cavan::T_OPEN_TAG)) return;

    t++;

    if (!match(*t, cavan::T_TEXT)) return;
    jute::view val = t->text;
    t++;

    if (!match(*t, cavan::T_CLOSE_TAG, key)) return;

    res.push_back(cavan::prop { key, val });
  });
  return res;
}

static auto list_modules(const cavan::token *& t) {
  hai::chain<jute::view> res {};
  take_if(t, "modules", [&] {
    jute::view name;
    take_tag("module", t, &name);
    res.push_back(name);
  });
  return res;
}

static void parse_project(const cavan::token *& t, cavan::pom & res) {
  using namespace cavan;

  if (match(*t, T_OPEN_TAG, "groupId")) take_tag("groupId", t, &res.grp);
  else if (match(*t, T_OPEN_TAG, "artifactId")) take_tag("artifactId", t, &res.art);
  else if (match(*t, T_OPEN_TAG, "version")) take_tag("version", t, &res.ver);
  else if (match(*t, T_OPEN_TAG, "parent"))
    take_if(t, "parent", [&] {
      if (match(*t, T_OPEN_TAG, "groupId")) take_tag("groupId", t, &res.parent.grp);
      else if (match(*t, T_OPEN_TAG, "artifactId")) take_tag("artifactId", t, &res.parent.art);
      else if (match(*t, T_OPEN_TAG, "version")) take_tag("version", t, &res.parent.ver);
      else lint_tag(t);
    });
  else if (match(*t, T_OPEN_TAG, "dependencyManagement"))
    take_if(t, "dependencyManagement", [&] { res.deps_mgmt = list_deps(t); });
  else if (match(*t, T_OPEN_TAG, "dependencies")) res.deps = list_deps(t);
  else if (match(*t, T_OPEN_TAG, "properties")) res.props = list_props(t);
  else if (match(*t, T_OPEN_TAG, "modules")) res.modules = list_modules(t);
  else if (match(*t, T_OPEN_TAG, "profiles")) list_profiles(t, &res);
  else lint_tag(t);
}

static cavan::pom parse_pom(const cavan::tokens & ts) {
  using namespace cavan;

  auto * t = ts.begin();

  if (match(*t, T_DIRECTIVE, "xml")) t++;

  cavan::pom res {};
  take(t, "project", [&] { parse_project(t, res); });

  if (res.grp.size() == 0) res.grp = res.parent.grp;
  if (res.ver.size() == 0) res.ver = res.parent.ver;

  res.props.push_back({ "project.groupId", res.grp });
  res.props.push_back({ "project.version", res.ver });
  return res;
}

static cavan::pom * try_read(jute::view file) {
  auto xml = jojo::read_cstr(file);

  auto tokens = cavan::split_tokens(xml);
  cavan::lint_xml(tokens);

  auto pom = parse_pom(tokens);
  pom.xml = traits::move(xml);
  pom.filename = file.cstr();
  return g_cache.take(traits::move(pom));
}

cavan::pom * cavan::read_pom(jute::view file) try {
  jojo::on_error([](void *, jute::view msg) {
    silog::log(silog::error, "IO error: %s", msg.cstr().begin());
    struct io_error {};
    throw io_error {};
  });
  return try_read(file);
} catch (...) {
  cavan::whilst("reading POM from " + file);
}

cavan::pom * cavan::read_pom(jute::view grp, jute::view art, jute::view ver) try {
  if (grp == "" || art == "" || ver == "") fail("missing identifier");

  auto cached = g_cache.get(grp, art, ver);
  if (cached) return cached;

  auto home_env = getenv("HOME");
  if (!home_env) fail("missing HOME environment variable");

  auto grp_path = grp.cstr();
  for (auto & c : grp_path)
    if (c == '.') c = '/';

  auto home = jute::view::unsafe(home_env);
  auto pom_file = home + "/.m2/repository/" + grp_path + "/" + art + "/" + ver + "/" + art + "-" + ver + ".pom";

  return read_pom(pom_file.cstr());
} catch (...) {
  cavan::whilst("parsing POM of " + grp + ":" + art + ":" + ver);
}

static auto read_parent(cavan::pom * pom) try {
  jojo::on_error([](void *, jute::view msg) {
    struct io_error {};
    throw io_error {};
  });

  auto [dir, fn] = jute::view { pom->filename }.rsplit('/');
  auto [pdir, dn] = dir.rsplit('/');
  auto ppom = pdir + "/pom.xml";
  try {
    auto parent = try_read(ppom.cstr());
    if (parent->grp == pom->parent.grp && parent->art == pom->parent.art && parent->ver == pom->parent.ver)
      return parent;
  } catch (...) {
    // Ignore and try from repo
  }
  return cavan::read_pom(pom->parent.grp, pom->parent.art, pom->parent.ver);
} catch (...) {
  cavan::whilst("reading parent of " + pom->grp + ":" + pom->art + ":" + pom->ver);
}

void cavan::read_parent_chain(pom * p) try {
  while (p->parent.grp.size() > 0) {
    if (!p->ppom) p->ppom = read_parent(p);
    p = &*p->ppom;
  }
} catch (...) {
  cavan::whilst("reading chain of " + p->grp + ":" + p->art + ":" + p->ver);
}

static auto infer_pom_from_source(jute::view src) {
  while (src != "") {
    auto [l, r] = src.rsplit('/');
    if (r == "src") return l;
    src = l;
  }
  cavan::fail("file not in maven repo");
}
cavan::pom * cavan::read_pom_of_source(jute::view java_file) {
  return read_pom((infer_pom_from_source(java_file) + "/pom.xml").cstr());
}
