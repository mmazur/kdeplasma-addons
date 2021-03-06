/*
 * Copyright 2018  Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "util.h"

// KF
#include <KIconLoader>
#include <KUnitConversion/Value>
#include <KLocalizedString>
// Qt
#include <QPixmap>
#include <QLocale>
// Std
#include <cmath>


template <typename T>
T clampValue(T value, int decimals)
{
    const T mul = std::pow(static_cast<T>(10), decimals);
    return int(value * mul) / mul;
}

Util::Util(QObject *parent)
    : QObject(parent)
{
}


QString Util::existingWeatherIconName(const QString &iconName) const
{
    const bool isValid = !iconName.isEmpty() &&
           !KIconLoader::global()->loadIcon(iconName, KIconLoader::Desktop, 0,
                                            KIconLoader::DefaultState, QStringList(), nullptr, true).isNull();
    return isValid ? iconName : QStringLiteral("weather-not-available");
}

QString Util::temperatureToDisplayString(int displayUnitType, double value,
                                         int valueUnitType, bool rounded, bool degreesOnly) const
{
    KUnitConversion::Value v(value, static_cast<KUnitConversion::UnitId>(valueUnitType));
    v = v.convertTo(static_cast<KUnitConversion::UnitId>(displayUnitType));

    const QString unit = degreesOnly ? i18nc("Degree, unit symbol", "°") : v.unit().symbol();

    if (rounded) {
        int tempNumber = qRound(v.number());
        return i18nc("temperature unitsymbol", "%1 %2", tempNumber, unit);
    }

    const QString formattedTemp = QLocale().toString(clampValue(v.number(), 1), 'f', 1);
    return i18nc("temperature unitsymbol", "%1 %2", formattedTemp, unit);
}

QString Util::valueToDisplayString(int displayUnitType, double value,
                                   int valueUnitType, int precision) const
{
    KUnitConversion::Value v(value, static_cast<KUnitConversion::UnitId>(valueUnitType));
    v = v.convertTo(static_cast<KUnitConversion::UnitId>(displayUnitType));

    // TODO: fix KUnitConversion to do locale encoded values and use that
    const QString formattedValue = QLocale().toString(clampValue(v.number(), precision), 'f', precision);
    return i18nc("value unitsymbol", "%1 %2", formattedValue, v.unit().symbol());
}

QString Util::percentToDisplayString(double value) const
{
    const QString formattedPercentValue = QLocale().toString(clampValue(value, 0), 'f', 0);
    return i18nc("value percentsymbol", "%1 %", formattedPercentValue);
}

