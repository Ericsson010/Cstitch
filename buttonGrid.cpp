//
// Copyright 2010, 2011 Tom Klein.
//
// This file is part of cstitch.
//
// cstitch is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "buttonGrid.h"

#include <QtCore/qmath.h>

#include <QtWidgets/QApplication>
#include <QPainter>
#include <QPen>
#include <QMouseEvent>

#include "triC.h"

buttonGrid::buttonGrid(const QVector<QPixmap>& icons,
                       const QString& windowTitle,
                       QWidget* parent)
  : QWidget(parent), gridX_(-1), gridY_(-1) {

  createQPixmapGrid(icons, windowTitle);
}

buttonGrid::buttonGrid(const QVector<triC>& colors, int iconSize,
                       int rowWidth, const QString& windowTitle,
                       QWidget* parent)
  : QWidget(parent), gridX_(-1), gridY_(-1) {

  QVector<QPixmap> colorPixmaps;
  colorPixmaps.reserve(colors.size());
  for (int i = 0, size = colors.size(); i < size; ++i) {
    QPixmap pixmap(iconSize, iconSize);
    pixmap.fill(colors[i].qc());
    colorPixmaps.push_back(pixmap);
  }
  createQPixmapGrid(colorPixmaps, windowTitle, rowWidth);
}

buttonGrid::buttonGrid(const QVector<floss>& floss, int iconSize,
                       int rowWidth, const QString& windowTitle,
                       QWidget* parent)
  : QWidget(parent), gridX_(-1), gridY_(-1) {
  createFlossGrid(floss, windowTitle, iconSize,rowWidth);
}

void buttonGrid::createQPixmapGrid(const QVector<QPixmap>& icons,
                                   const QString& windowTitle,
                                   int rowWidth) {

  buttonWidht_ = icons.empty() ? 12 : icons[0].width() + 12;
  buttonHeight_ = buttonWidht_;
  const QPalette palette(QApplication::palette());
  const QColor windowColor(palette.color(QPalette::Window));
  const QColor darkColor(palette.color(QPalette::Dark));
  for (int i = 0, size = icons.size(); i < size; ++i) {
    QPixmap pixmap(buttonWidht_, buttonHeight_);
    pixmap.fill(windowColor);
    QPainter painter(&pixmap);
    painter.setPen(QPen(darkColor, 2));
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawPixmap(QPoint(6, 6), icons[i]);
    painter.drawRoundedRect(QRect(1, 1, buttonWidht_-2, buttonHeight_-2),
                            3.5, 3.5);

    icons_.push_back(pixmap);
  }
  const int numIcons = icons_.size();
  buttonsPerRow_ = rowWidth ? rowWidth : ceil(sqrt(numIcons));
  if (!rowWidth) {
    buttonsPerRow_ += buttonsPerRow_/4;
  }
  int gridRows = numIcons/buttonsPerRow_;
  if (numIcons % buttonsPerRow_ != 0) {
    ++gridRows;
  }
  setFixedSize(buttonsPerRow_ * buttonWidht_, gridRows * buttonHeight_);
  setWindowTitle(windowTitle);
}

void buttonGrid::createFlossGrid(const QVector<floss>& floss,
                                   const QString& windowTitle,
                                   int iconSize, int rowWidth) {
  int descriptionLength = 180;
  buttonWidht_ = iconSize + 12 + descriptionLength;
  buttonHeight_ = iconSize + 12;
  const QPalette palette(QApplication::palette());
  const QColor windowColor(palette.color(QPalette::Window));
  const QColor darkColor(palette.color(QPalette::Dark));
  for (int i = 0, size = floss.size(); i < size; ++i) {
    QPixmap pixmap(buttonWidht_, buttonHeight_);
    pixmap.fill(windowColor);
    QPainter painter(&pixmap);
    painter.setPen(QPen(darkColor, 2));
    painter.setRenderHint(QPainter::Antialiasing);
    QPixmap color(iconSize, iconSize);
    color.fill(floss[i].color().qc());
    painter.drawPixmap(QPoint(6, 6), color);
    painter.drawRoundedRect(QRect(1, 1, buttonWidht_-2, buttonHeight_-2),
                            3.5, 3.5);
    painter.setPen(QPen(Qt::black, 4));
    painter.drawText(QPoint(iconSize + 12, iconSize),
                     QString::number(floss[i].code()) + " - " + floss[i].name());

    icons_.push_back(pixmap);
  }
  const int numIcons = floss.size();
  buttonsPerRow_ = rowWidth ? rowWidth : ceil(sqrt(numIcons));
  if (!rowWidth) {
    buttonsPerRow_ += buttonsPerRow_/4;
  }
  int gridRows = numIcons/buttonsPerRow_;
  if (numIcons % buttonsPerRow_ != 0) {
    ++gridRows;
  }
  setFixedSize(buttonsPerRow_ * buttonWidht_, gridRows * buttonHeight_);
  setWindowTitle(windowTitle);

}

QSize buttonGrid::sizeHint() const {

  int numRows = icons_.size()/buttonsPerRow_;
  if (icons_.size()%buttonsPerRow_ != 0) {
    ++numRows;
  }
  int parrentHeight = this->parentWidget()->height();
  int height = (numRows*buttonHeight_ > parrentHeight/2) ? parrentHeight/4 : numRows*buttonHeight_;

  return QSize(buttonsPerRow_*buttonWidht_, height);
}

void buttonGrid::paintEvent(QPaintEvent* ) {
  int i = 0, j = 0;
  QPainter painter(this);
  for (int k = 0, size = icons_.size(); k < size; ++k) {
    painter.drawPixmap(QPoint(i*buttonWidht_, j*buttonHeight_),
                       icons_[k]);
    ++i;
    if (i == buttonsPerRow_) {
      i = 0;
      ++j;
    }
  }
  if (gridX_ >= 0) { // if a button has been chosen, highlight it
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(Qt::black, 1.2, Qt::DotLine));
    painter.drawRoundedRect(QRect(gridX_ * buttonWidht_ + 4, gridY_ * buttonHeight_ + 4,
                                  buttonWidht_ - 8, buttonHeight_ - 8), 0., 0.);
    painter.restore();
  }
}

void buttonGrid::mousePressEvent(QMouseEvent* event) {

  if (event->button() == Qt::LeftButton) {
    gridX_ = event->x()/buttonWidht_;
    gridY_ = event->y()/buttonHeight_;
    const int index = gridY_ * buttonsPerRow() + gridX_;
    if (index >= icons_.size() || index < 0) {
      return;
    }
    emit buttonSelected(index);
    update();
  }
}

colorButtonGrid::colorButtonGrid(const QVector<triC>& colors, int iconSize,
                                 const QString& windowTitle, int rowWidth,
                                 QWidget* parent)
  : buttonGrid(colors, iconSize, rowWidth, windowTitle, parent),
    colors_(colors) {


}

colorButtonGrid::colorButtonGrid(const QVector<floss>& floss,
                                 const QVector<triC>& colors, int iconSize,
                                 const QString& windowTitle, int rowWidth,
                                 QWidget* parent)
  : buttonGrid(floss, iconSize, rowWidth, windowTitle, parent),
    colors_(colors) {
}

void colorButtonGrid::mousePressEvent(QMouseEvent* event) {

  if (event->button() == Qt::LeftButton) {
    int gridX = event->x()/buttonWidht();
    int gridY = event->y()/buttonHeight();
    const int index = gridY * buttonsPerRow() + gridX;

    // click on an empty grid tile does nothing
    if (index >= colors_.size() || index < 0) {
      return;
    }
    setButtonClickedX(gridX);
    setButtonClickedY(gridY);
    update();
    emit buttonSelected(colors_[index].qrgb(), gridX, gridY);
  }
}

void colorButtonGrid::focusButton(int x, int y) {

  const int buttonCount = y * buttonsPerRow() + x;
  if (buttonCount >= colors_.size() || buttonCount < 0) {
    return;
  }
  setButtonClickedX(x);
  setButtonClickedY(y);
  update();
  emit buttonSelected(colors_[buttonCount].qrgb(), x, y);
}

QSize colorButtonGrid::sizeHint() const {

  return buttonGrid::sizeHint();
}
