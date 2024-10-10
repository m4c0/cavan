module cavan;
import silog;

static bool take_activation(const cavan::token *& t) {
  lint_tag(t);
  return false;
}

void cavan::list_profiles(const cavan::token *& t, cavan::pom * res) {
  cavan::deps dm {};
  cavan::deps deps {};
  bool active = false;
  take(t, "profiles", [&] {
    take(t, "profile", [&] {
      if (match(*t, T_OPEN_TAG, "activation")) active = take_activation(t);
      else if (match(*t, T_OPEN_TAG, "dependencyManagement"))
        take_if(t, "dependencyManagement", [&] { dm = list_deps(t); });
      else if (match(*t, T_OPEN_TAG, "dependencies")) deps = list_deps(t);
      else lint_tag(t);
    });
  });

  if (active) {
    for (auto & [d, depth] : dm) res->deps_mgmt.push_back(d, depth);
    for (auto & [d, depth] : deps) res->deps.push_back(d, depth);
  }
}
