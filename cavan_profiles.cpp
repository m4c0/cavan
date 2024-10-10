module cavan;
import silog;

static bool take_property(const cavan::token *& t) {
  jute::view name;
  jute::view value;
  if (match(*t, cavan::T_OPEN_TAG, "name")) take_tag("name", t, &name);
  if (match(*t, cavan::T_OPEN_TAG, "value")) take_tag("value", t, &value);

  if (!name.size()) return false;

  bool reverse = name[0] == '!';
  if (reverse) name = name.subview(1).after;
  // Simple logic to assume no variable is defined
  return reverse;
}

static bool take_activation(const cavan::token *& t) {
  auto res = false;
  take_if(t, "property", [&] { res |= take_property(t); });
  return res;
}

void cavan::list_profiles(const cavan::token *& t, cavan::pom * res) {
  cavan::deps dm {};
  cavan::deps deps {};
  bool active = false;
  take(t, "profiles", [&] {
    take(t, "profile", [&] {
      take_if(t, "activation", [&] { active = take_activation(t); });
      take_if(t, "dependencyManagement", [&] { dm = list_deps(t); });
      take_if(t, "dependencies", [&] { deps = list_deps(t); });
    });
  });

  if (active) {
    for (auto & [d, depth] : dm) res->deps_mgmt.push_back(d, depth);
    for (auto & [d, depth] : deps) res->deps.push_back(d, depth);
  }
}
