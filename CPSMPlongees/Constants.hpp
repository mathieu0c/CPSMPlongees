#pragma once

#include <QColor>
#include <SimpleUpdater.hpp>

namespace consts {

constexpr auto PROJECT_GITHUB_RELEASE{PROJECT_GITHUB_RELEASE_DEFINE};
constexpr updt::Version kCurrentVersion{PROJECT_V_MAJOR, PROJECT_V_MINOR, PROJECT_V_PATCH};

constexpr auto PUBLIC_VERIFIER_KEY_FILE{"Assets/CPSMPlongees.public"};
constexpr auto POST_UPDATE_CMD{"CPSMPlongees.exe"};
constexpr auto UPDATED_TAG_FILENAME{"UPDATED.TAG"};

constexpr auto kDateUserFormat{"dd/MM/yyyy"};

namespace colors {
const QColor kBackgroundGreen{"#eaffe8"};
const QColor kBackgroundRed{"#ffe3e3"};
}  // namespace colors

}  // namespace consts
