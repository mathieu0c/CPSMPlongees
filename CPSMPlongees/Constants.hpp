#pragma once

#include <CPSMGlobals.hpp>
#include <QColor>
#include <QDate>
#include <SimpleUpdater.hpp>

namespace consts {

namespace colors {
const QColor kBackgroundInvisible{"#00000000"};

const QColor kBackgroundGreen{"#eaffe8"};
const QColor kBackgroundRed{"#ffe3e3"};
const QColor kCriticalRed{"#ff0000"};

const QColor kBackgroundYellow{"#ffffe0"};
const QColor kBackgroundBlue{"#e3f2ff"};
const QColor kBackgroundSunsetOrange{"#FECB94"};
const QColor kBackgroundNightPurple{"#C7CAFF"};

// const QColor kBackgroundPurple{"#d39edb"};
// const QColor kBackgroundCyan{"#9eded6"};
const QColor kBackgroundPurple{"#f2e1f5"};
const QColor kBackgroundCyan{"#ebfaf8"};

QColor GetColorForDivingType(int diving_type_id, int default_diving_type_id);

}  // namespace colors

}  // namespace consts
