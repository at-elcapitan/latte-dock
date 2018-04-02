/*
*  Copyright 2016  Smith AR <audoban@openmailbox.org>
*                  Michail Vourlakos <mvourlakos@gmail.com>
*
*  This file is part of Latte-Dock
*
*  Latte-Dock is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  Latte-Dock is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "quickwindowsystem.h"
#include "../app/config-latte.h"

#include <QDebug>
#include <QProcess>

#include <plasma/version.h>

#if HAVE_X11
    #include <KWindowSystem>
#endif

namespace Latte {

QuickWindowSystem::QuickWindowSystem(QObject *parent)
    : QObject(parent)
{
    if (KWindowSystem::isPlatformWayland()) {
        //! TODO: Wayland compositing active
        m_compositing = true;
    } else {
        connect(KWindowSystem::self(), &KWindowSystem::compositingChanged
        , this, [&](bool enabled) {
            if (m_compositing == enabled)
                return;

            m_compositing = enabled;
            emit compositingChanged();
        });

        m_compositing = KWindowSystem::compositingActive();
    }

    loadPlasmaDesktopVersion();
}

QuickWindowSystem::~QuickWindowSystem()
{
    qDebug() << staticMetaObject.className() << "destructed";
}

bool QuickWindowSystem::compositingActive() const
{
    return m_compositing;
}

bool QuickWindowSystem::isPlatformWayland() const
{
    return KWindowSystem::isPlatformWayland();
}

uint QuickWindowSystem::frameworksVersion() const
{
    return Plasma::version();
}

uint QuickWindowSystem::plasmaDesktopVersion() const
{
    return m_plasmaDesktopVersion;
}

uint QuickWindowSystem::makeVersion(uint major, uint minor, uint release) const
{
    return (((major) << 16) | ((minor) << 8) | (release));
}

void QuickWindowSystem::loadPlasmaDesktopVersion()
{
    //! Indentify Plasma Desktop version

    QProcess process;
    process.start("plasmashell", QStringList() << "-v");
    process.waitForFinished();
    QString output(process.readAllStandardOutput());

    QStringList stringSplit = output.split(" ");

    if (stringSplit.count() >= 2) {
        qDebug() << " /////////////////////////";
        QString cleanVersionString = stringSplit[1].remove("\n");
        QStringList plasmaDesktopVersionParts = cleanVersionString.split(".");

        if (plasmaDesktopVersionParts.count() == 3) {
            uint maj = plasmaDesktopVersionParts[0].toUInt();
            uint min = plasmaDesktopVersionParts[1].toUInt();
            uint rel = plasmaDesktopVersionParts[2].toUInt();

            if (maj > 0) {
                m_plasmaDesktopVersion = makeVersion(maj, min, rel);

                QString message("Plasma Desktop version:  " + QString::number(maj) + "."
                                + QString::number(min) + "." + QString::number(rel)
                                + " (" + QString::number(m_plasmaDesktopVersion) + ")");
                qDebug() << message;
            }
        }

        qDebug() << " /////////////////////////";
    }
}

} //end of namespace
