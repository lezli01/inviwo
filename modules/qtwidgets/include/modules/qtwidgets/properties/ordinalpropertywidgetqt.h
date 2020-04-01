/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2020 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#pragma once

#include <modules/qtwidgets/qtwidgetsmoduledefine.h>
#include <modules/qtwidgets/inviwoqtutils.h>
#include <modules/qtwidgets/ordinalbasewidget.h>
#include <modules/qtwidgets/editablelabelqt.h>
#include <modules/qtwidgets/sliderwidgetqt.h>
#include <modules/qtwidgets/ordinaleditorwidget.h>
#include <modules/qtwidgets/properties/propertysettingswidgetqt.h>
#include <modules/qtwidgets/properties/propertywidgetqt.h>
#include <modules/qtwidgets/properties/ordinalspinboxwidget.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/properties/propertyowner.h>

#include <cmath>

#include <warn/push>
#include <warn/ignore/all>
#include <QMenu>
#include <warn/pop>

namespace inviwo {

namespace util {

template <typename T>
T spherical(T val) {
    const dvec3 dval{val};
    const dvec3 res{glm::length(dval),
                    std::atan2(std::sqrt(dval[0] * dval[0] + dval[1] * dval[1]), dval[2]),
                    std::atan2(dval[1], dval[0])};
    return static_cast<T>(res);
}

template <typename T>
T euclidean(T val) {
    const dvec3 dval{val};
    const dvec3 res{dval[0] * std::sin(dval[1]) * std::cos(dval[2]),
                    dval[0] * std::sin(dval[1]) * std::sin(dval[2]), dval[0] * std::cos(dval[1])};
    return static_cast<T>(res);
}

}  // namespace util

enum class OrdinalPropertyWidgetQtSematics { Default, Spherical, SpinBox, SphericalSpinBox, Text };

template <typename T, OrdinalPropertyWidgetQtSematics Sem>
class OrdinalPropertyWidgetQt final : public PropertyWidgetQt {
public:
    using BT = typename util::value_type<T>::type;

    OrdinalPropertyWidgetQt(OrdinalProperty<T>* property);
    virtual ~OrdinalPropertyWidgetQt() = default;
    virtual void updateFromProperty() override;
    virtual std::unique_ptr<QMenu> getContextMenu() override;

private:
    // Connected to OrdinalEditorWidget::valueChanged()
    void setPropertyValue(size_t);
    void showSettings();

    OrdinalProperty<T>* ordinal_;
    EditableLabelQt* label_;
    OrdinalPropertySettingsWidgetQt<T>* settingsWidget_;

    std::vector<OrdinalBaseWidget<BT>*> editors_;
};

template <typename T, OrdinalPropertyWidgetQtSematics Sem>
OrdinalPropertyWidgetQt<T, Sem>::OrdinalPropertyWidgetQt(OrdinalProperty<T>* property)
    : PropertyWidgetQt(property)
    , ordinal_(property)
    , label_{new EditableLabelQt(this, property)}
    , settingsWidget_(nullptr) {

    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->setSpacing(getSpacing());
    hLayout->addWidget(label_);

    auto centralWidget = new QWidget();
    auto policy = centralWidget->sizePolicy();
    policy.setHorizontalStretch(3);
    centralWidget->setSizePolicy(policy);

    auto gridLayout = new QGridLayout();
    centralWidget->setLayout(gridLayout);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->setSpacing(0);

    auto factory = [this](size_t row, size_t col) {
        auto editor = []() {
            if constexpr (Sem == OrdinalPropertyWidgetQtSematics::SpinBox) {
                return new OrdinalSpinBoxWidget<BT>();
            } else if constexpr (Sem == OrdinalPropertyWidgetQtSematics::SphericalSpinBox) {
                return new OrdinalSpinBoxWidget<BT>();
            } else if constexpr (Sem == OrdinalPropertyWidgetQtSematics::Text) {
                return new OrdinalEditorWidget<BT>();
            } else if constexpr (Sem == OrdinalPropertyWidgetQtSematics::Spherical) {
                return new SliderWidgetQt<BT>();
            } else {
                return new SliderWidgetQt<BT>();
            }
        }();

        editors_.push_back(editor);

        if constexpr (Sem == OrdinalPropertyWidgetQtSematics::SphericalSpinBox ||
                      Sem == OrdinalPropertyWidgetQtSematics::Spherical) {
            if (col > 0) editor->setWrapping(true);
        }

        connect(editor, &std::remove_reference_t<decltype(*editor)>::valueChanged, this,
                [this, index = col + row * util::extent<T, 0>::value]() {
                    this->setPropertyValue(index);
                });

        auto sp = editor->sizePolicy();
        sp.setHorizontalPolicy(QSizePolicy::Expanding);
        editor->setSizePolicy(sp);

        if constexpr (Sem == OrdinalPropertyWidgetQtSematics::SphericalSpinBox ||
                      Sem == OrdinalPropertyWidgetQtSematics::Spherical) {
            if (col > 0) editor->setWrapping(true);

            constexpr std::array<const char*, 3> sphericalLabels{"r", "<html>&theta;</html>",
                                                                 "<html>&phi;</html>"};
            auto widget = new QWidget(this);
            auto layout = new QHBoxLayout();
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(7);
            widget->setLayout(layout);
            layout->addWidget(new QLabel(sphericalLabels[col], this));
            layout->addWidget(editor);

            widget->setFocusPolicy(editor->focusPolicy());
            widget->setFocusProxy(editor);
            return widget;
        } else {
            return editor;
        }
    };

    for (size_t row = 0; row < util::extent<T, 1>::value; row++) {
        for (size_t col = 0; col < util::extent<T, 0>::value; col++) {
            auto editor = factory(row, col);

            auto layoutCol = col;
            auto layoutRow = row;

            // vectors should be drawn in row major while matrices are column major
            if constexpr (util::extent<T, 1>::value > 1 &&
                          Sem != OrdinalPropertyWidgetQtSematics::Default) {
                std::swap(layoutCol, layoutRow);
            } else if constexpr (Sem == OrdinalPropertyWidgetQtSematics::Default) {
                layoutCol = 1;
                layoutRow = col + util::extent<T, 1>::value * row;
            }
            gridLayout->addWidget(editor, static_cast<int>(layoutRow), static_cast<int>(layoutCol));
        }
    }

    if ((gridLayout->count() > 0) && gridLayout->itemAt(0)->widget()) {
        setFocusPolicy(gridLayout->itemAt(0)->widget()->focusPolicy());
        setFocusProxy(gridLayout->itemAt(0)->widget());
    }

    hLayout->addWidget(centralWidget);

    centralWidget->setMinimumHeight(centralWidget->sizeHint().height());
    auto sp = centralWidget->sizePolicy();
    sp.setVerticalPolicy(QSizePolicy::Fixed);
    centralWidget->setSizePolicy(sp);

    setLayout(hLayout);

    setFixedHeight(sizeHint().height());
    sp = sizePolicy();
    sp.setVerticalPolicy(QSizePolicy::Fixed);
    setSizePolicy(sp);

    updateFromProperty();
}

template <typename T, OrdinalPropertyWidgetQtSematics Sem>
void OrdinalPropertyWidgetQt<T, Sem>::updateFromProperty() {
    T min = ordinal_->getMinValue();
    T max = ordinal_->getMaxValue();
    T inc = ordinal_->getIncrement();
    T val = ordinal_->get();

    if constexpr (Sem == OrdinalPropertyWidgetQtSematics::SphericalSpinBox ||
                  Sem == OrdinalPropertyWidgetQtSematics::Spherical) {
        val = util::spherical(val);
        min = T{std::numeric_limits<BT>::epsilon(), 0, -M_PI};
        max = T{3 * glm::length(max), M_PI, M_PI};
        inc = T{glm::length(inc), M_PI / 100.0, 2 * M_PI / 100.0};
    }

    const size_t nelem = ordinal_->getDim().x * ordinal_->getDim().y;
    for (size_t i = 0; i < nelem; i++) {
        editors_[i]->setMinValue(util::glmcomp(min, i), ordinal_->getMinConstraintBehaviour());
        editors_[i]->setMaxValue(util::glmcomp(max, i), ordinal_->getMaxConstraintBehaviour());
        editors_[i]->setIncrement(util::glmcomp(inc, i));
        editors_[i]->initValue(util::glmcomp(val, i));
    }
}

template <typename T, OrdinalPropertyWidgetQtSematics Sem>
void OrdinalPropertyWidgetQt<T, Sem>::setPropertyValue(size_t editorId) {
    T val = ordinal_->get();

    if constexpr (Sem == OrdinalPropertyWidgetQtSematics::SphericalSpinBox ||
                  Sem == OrdinalPropertyWidgetQtSematics::Spherical) {
        val = util::spherical(val);
    }

    util::glmcomp(val, editorId) = editors_[editorId]->getValue();

    if constexpr (Sem == OrdinalPropertyWidgetQtSematics::SphericalSpinBox ||
                  Sem == OrdinalPropertyWidgetQtSematics::Spherical) {
        val = util::euclidean(val);
    }

    ordinal_->setInitiatingWidget(this);
    ordinal_->set(val);
    ordinal_->clearInitiatingWidget();
}

template <typename T, OrdinalPropertyWidgetQtSematics Sem>
void OrdinalPropertyWidgetQt<T, Sem>::showSettings() {
    if (!settingsWidget_) {
        settingsWidget_ = new OrdinalPropertySettingsWidgetQt<T>(ordinal_, this);
    }
    settingsWidget_->showWidget();
}

template <typename T, OrdinalPropertyWidgetQtSematics Sem>
std::unique_ptr<QMenu> OrdinalPropertyWidgetQt<T, Sem>::getContextMenu() {
    auto menu = PropertyWidgetQt::getContextMenu();

    auto settingsAction = menu->addAction(tr("&Property settings..."));
    settingsAction->setToolTip(
        tr("&Open the property settings dialog to adjust min, max, and increment values"));

    connect(settingsAction, &QAction::triggered, this,
            &OrdinalPropertyWidgetQt<T, Sem>::showSettings);

    settingsAction->setEnabled(!property_->getReadOnly());
    settingsAction->setVisible(getApplicationUsageMode() == UsageMode::Development);

    return menu;
}

}  // namespace inviwo
