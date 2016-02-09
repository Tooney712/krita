/*
 *  Copyright (c) 2008,2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "kis_sprayop_option.h"
#include <klocalizedstring.h>

#include "ui_wdgsprayoptions.h"

class KisSprayOpOptionsWidget: public QWidget, public Ui::WdgSprayOptions
{
public:
    KisSprayOpOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);
    }
};

KisSprayOpOption::KisSprayOpOption()
    : KisPaintOpOption(KisPaintOpOption::GENERAL, false)
{
    setObjectName("KisSprayOpOption");

    m_checkable = false;
    m_options = new KisSprayOpOptionsWidget();

    m_options->diameterSpinBox->setRange(1, 1000, 0);
    m_options->diameterSpinBox->setValue(100);
    m_options->diameterSpinBox->setExponentRatio(1.5);
    m_options->diameterSpinBox->setSuffix(i18n(" px"));

    m_options->aspectSPBox->setRange(0.0, 2.0, 2);
    m_options->aspectSPBox->setValue(1.0);

    m_options->rotationSPBox->setRange(0.0, 360.0, 0);
    m_options->rotationSPBox->setValue(0.0);
    m_options->rotationSPBox->setSuffix(QChar(Qt::Key_degree));

    m_options->scaleSpin->setRange(0.0, 10.0, 2);
    m_options->scaleSpin->setValue(1.0);

    m_options->spacingSpin->setRange(0.0, 5.0, 2);
    m_options->spacingSpin->setValue(0.5);

    m_options->coverageSpin->setRange(0.0, 100.0, 1);
    m_options->coverageSpin->setValue(0.1);
    m_options->coverageSpin->setSuffix("%");


    m_options->particlesSpinBox->setRange(1.0, 1000.0, 0);
    m_options->particlesSpinBox->setValue(12);
    m_options->particlesSpinBox->setExponentRatio(3.0);

    m_options->jitterMovementSpin->setRange(0.0,5.0, 1);
    m_options->jitterMovementSpin->setValue(1.0);


    connect(m_options->diameterSpinBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->coverageSpin, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->jitterMovementSpin, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->spacingSpin, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->scaleSpin, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->particlesSpinBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->countRadioButton, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->densityRadioButton, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->gaussianBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->aspectSPBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->rotationSPBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->jitterMoveBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));

    connect(m_options->countRadioButton, SIGNAL(toggled(bool)), m_options->particlesSpinBox, SLOT(setEnabled(bool)));
    connect(m_options->densityRadioButton, SIGNAL(toggled(bool)), m_options->coverageSpin, SLOT(setEnabled(bool)));
    connect(m_options->jitterMoveBox, SIGNAL(toggled(bool)), m_options->jitterMovementSpin, SLOT(setEnabled(bool)));

    setConfigurationPage(m_options);
}

KisSprayOpOption::~KisSprayOpOption()
{
    delete m_options;
}

void KisSprayOpOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    setting->setProperty(SPRAY_DIAMETER, m_options->diameterSpinBox->value());
    setting->setProperty(SPRAY_ASPECT, m_options->aspectSPBox->value());
    setting->setProperty(SPRAY_COVERAGE, m_options->coverageSpin->value());
    setting->setProperty(SPRAY_SCALE, m_options->scaleSpin->value());
    setting->setProperty(SPRAY_ROTATION, m_options->rotationSPBox->value());
    setting->setProperty(SPRAY_PARTICLE_COUNT, m_options->particlesSpinBox->value());
    setting->setProperty(SPRAY_JITTER_MOVE_AMOUNT, m_options->jitterMovementSpin->value());
    setting->setProperty(SPRAY_JITTER_MOVEMENT, m_options->jitterMoveBox->isChecked());
    setting->setProperty(SPRAY_SPACING, m_options->spacingSpin->value());
    setting->setProperty(SPRAY_GAUSS_DISTRIBUTION, m_options->gaussianBox->isChecked());
    setting->setProperty(SPRAY_USE_DENSITY, m_options->densityRadioButton->isChecked());
}

void KisSprayOpOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    m_options->diameterSpinBox->setValue(setting->getInt(SPRAY_DIAMETER));
    m_options->aspectSPBox->setValue(setting->getDouble(SPRAY_ASPECT));
    m_options->coverageSpin->setValue(setting->getDouble(SPRAY_COVERAGE));
    m_options->scaleSpin->setValue(setting->getDouble(SPRAY_SCALE));
    m_options->rotationSPBox->setValue(setting->getDouble(SPRAY_ROTATION));
    m_options->particlesSpinBox->setValue(setting->getDouble(SPRAY_PARTICLE_COUNT));
    m_options->jitterMovementSpin->setValue(setting->getDouble(SPRAY_JITTER_MOVE_AMOUNT));
    m_options->jitterMoveBox->setChecked(setting->getBool(SPRAY_JITTER_MOVEMENT));
    m_options->spacingSpin->setValue(setting->getDouble(SPRAY_SPACING));
    m_options->gaussianBox->setChecked(setting->getBool(SPRAY_GAUSS_DISTRIBUTION));
    //TODO: come on, do this nicer! e.g. button group or something
    bool useDensity = setting->getBool(SPRAY_USE_DENSITY);
    m_options->densityRadioButton->setChecked(useDensity);
    m_options->countRadioButton->setChecked(!useDensity);
}


void KisSprayOpOption::setDiameter(int diameter) const
{
    m_options->diameterSpinBox->setValue(diameter);
}

int KisSprayOpOption::diameter() const
{
    return m_options->diameterSpinBox->value();
}

qreal KisSprayOpOption::brushAspect() const
{
    return m_options->aspectSPBox->value();
}
