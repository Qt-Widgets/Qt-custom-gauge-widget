/***************************************************************************
**                                                                        **
**  QcGauge, for instrumentation, and real time data measurement          **
**  visualization widget for Qt.                                          **
**  Copyright (C) 2015 Hadj Tahar Berrima                                 **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU Lesser General Public License as        **
**  published by the Free Software Foundation, either version 3 of the    **
**  License, or (at your option) any later version.                       **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU Lesser General Public License for more details.                   **
**                                                                        **
**  You should have received a copy of the GNU Lesser General Public      **
**  License along with this program.                                      **
**  If not, see http://www.gnu.org/licenses/.                             **
**                                                                        **
****************************************************************************
**           Author:  Hadj Tahar Berrima                                  **
**           Website: http://pytricity.com/                               **
**           Contact: berrima_tahar@yahoo.com                             **
**           Date:    1 dec 2014                                          **
**           Version:  1.0                                                **
****************************************************************************/



#include "qcgaugewidget.h"

#include <QPainter>
#include <QtMath>
#include <QResizeEvent>
#include <QGraphicsBlurEffect>
#include <QLabel>

#include <qtlabb/common/qtlabb_diag.h>
#include <qtlabb/common/StreamHelpers.h>

/**
 * This is private QT API
 */
QT_BEGIN_NAMESPACE
  extern Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage,
	  qreal radius, bool quality, bool alphaOnly, int transposed = 0 );
QT_END_NAMESPACE

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

QcGaugeWidget::QcGaugeWidget(QWidget *parent) :
    QWidget(parent),
    mUpdateBufferImages(true),
    mBorderPen(Qt::NoPen)
{

}


QcGaugeWidget::~QcGaugeWidget()
{
	qDeleteAll(mItems);
}

QcBackgroundItem *QcGaugeWidget::addBackground(double position)
{
    QcBackgroundItem * item = new QcBackgroundItem(this);
    item->setPosition(position);
    mItems.append(item);
    return item;
}

QcDegreesItem *QcGaugeWidget::addDegrees(double position)
{
    QcDegreesItem * item = new QcDegreesItem(this);
    item->setPosition(position);

    mItems.append(item);
    return item;
}


QcValuesItem *QcGaugeWidget::addValues(double position)
{
    QcValuesItem * item = new QcValuesItem(this);
    item->setPosition(position);
    mItems.append(item);
    return item;
}

QcArcItem *QcGaugeWidget::addArc(double position)
{
    QcArcItem * item = new QcArcItem(this);
    item->setPosition(position);
    mItems.append(item);
    return item;
}

QcColorBand *QcGaugeWidget::addColorBand(double position)
{
    QcColorBand * item = new QcColorBand(this);
    item->setPosition(position);
    mItems.append(item);
    return item;
}

QcNeedleItem *QcGaugeWidget::addNeedle(double position)
{
    QcNeedleItem * item = new QcNeedleItem(this);
    item->setPosition(position);
    mItems.append(item);
    return item;
}

QcLabelItem *QcGaugeWidget::addLabel(double position)
{
    QcLabelItem * item = new QcLabelItem(this);
    item->setPosition(position);
    mItems.append(item);
    return item;
}

QcGlassItem *QcGaugeWidget::addGlass(double position)
{
    QcGlassItem * item = new QcGlassItem(this);
    item->setPosition(position);
    mItems.append(item);
    return item;
}

QcAltitudeMeter *QcGaugeWidget::addAltitudeMeter(double position)
{
    QcAltitudeMeter * item = new QcAltitudeMeter(this);
    item->setPosition(position);
    mItems.append(item);
    return item;
}

void QcGaugeWidget::addItem(QcItem *item, double position)
{
    item->setPosition(position);
    mItems.append(item);
}

bool QcGaugeWidget::removeItem(QcItem *item)
{
	QList<QcItem*>::iterator it;
	it = mItems.begin();
	for (; it != mItems.end(); ++it)
	{
		if (*it == item)
		{
			mItems.erase(it);
			return true;
		}
	}

	return false;
}

QList<QcItem *> QcGaugeWidget::items()
{
    return mItems;
}

int QcGaugeWidget::heightForWidth(int w) const
{
	return w;
}


int QcGaugeWidget::diameter() const
{
	return qMin(rect().width(), rect().height());
}


QImage QcGaugeWidget::createBufferImage() const
{
	int Diameter = diameter();
	return QImage(QSize(Diameter, Diameter), QImage::Format_ARGB32_Premultiplied);
}


void QcGaugeWidget::updateBufferImages()
{
	mNeedleItemIndex = -1;
	mBackgroundBuffer = createBufferImage();
	mBackgroundBuffer.fill(qRgba(0, 0, 0, 0));
	mForegeroundBuffer = createBufferImage();
	mForegeroundBuffer.fill(qRgba(0, 0, 0, 0));

	int i = 0;
	QPainter BackgroundPainter(&mBackgroundBuffer);
	BackgroundPainter.setRenderHints(QPainter::Antialiasing);
	for (; i < mItems.size(); ++i)
	{
		QcNeedleItem* NeedleItem = dynamic_cast<QcNeedleItem*>(mItems.at(i));
		if (NeedleItem)
		{
			mNeedleItemIndex = i;
			i++;
			break;
		}

		mItems.at(i)->draw(&BackgroundPainter);
	}

	QPainter ForegroundPainter(&mForegeroundBuffer);
	ForegroundPainter.setRenderHints(QPainter::Antialiasing);
	for (; i < mItems.size(); ++i)
	{
		mItems.at(i)->draw(&ForegroundPainter);
	}
	mUpdateBufferImages = false;
}


void QcGaugeWidget::invalidateBufferImages()
{
	mUpdateBufferImages = true;
}


QImage QcGaugeWidget::blurShadowImage(QImage& Source) const
{
	double Radius = diameter() / 2.0;
	double BlurRadius = Radius / 20;
	int ImageBorderSize = BlurRadius / 2;
	QImage ExtendedSource(Source.size() + QSize(ImageBorderSize * 2, ImageBorderSize * 2), QImage::Format_ARGB32_Premultiplied);
	ExtendedSource.fill(Qt::transparent);
	{
		QPainter Painter(&ExtendedSource);
		Painter.drawImage(QPoint(ImageBorderSize, ImageBorderSize), Source);
	}

	QImage BlurredImage(ExtendedSource.size(), QImage::Format_ARGB32_Premultiplied);
    BlurredImage.fill(Qt::transparent);
    QPainter Painter(&BlurredImage);
    qt_blurImage(&Painter, ExtendedSource, BlurRadius, false, false );//blur radius: 2px
    return BlurredImage;
}


void QcGaugeWidget::setBorderPen(const QPen& Pen)
{
	mBorderPen = Pen;
}


QBrush QcGaugeWidget::shadowBrush() const
{
	return QBrush(QColor(0, 0, 0, 160));
}

QPointF QcGaugeWidget::shadowOffset() const
{
	double WidgetRadius = diameter() / 2.0;
	QPointF ShadowOffset(0.02, 0.03);
	return ShadowOffset * WidgetRadius;
}


void QcGaugeWidget::paintEvent(QPaintEvent* PaintEvent)
{
	if (mUpdateBufferImages)
	{
		updateBufferImages();
	}

	QWidget::paintEvent(PaintEvent);
	QPainter painter(this);
	int Radius = diameter() / 2;
	painter.translate(rect().center().x() - Radius, rect().center().y() - Radius);
	painter.setRenderHint(QPainter::Antialiasing);
    painter.drawImage(QPointF(0, 0), mBackgroundBuffer);
    if (mNeedleItemIndex)
    {
    	mItems.at(mNeedleItemIndex)->draw(&painter);
    }
    painter.drawImage(QPointF(0, 0), mForegeroundBuffer);
    if (mBorderPen.style() != Qt::NoPen)
    {
    	painter.setBrush(Qt::NoBrush);
    	painter.setPen(mBorderPen);
    	float PenWidth = mBorderPen.width() / 2.0 + 1;
    	QRectF EllipseRect = QRectF(contentsRect()).adjusted(PenWidth, PenWidth, -PenWidth - 1, -PenWidth - 1);
    	painter.drawEllipse(EllipseRect);
    }
}


void QcGaugeWidget::resizeEvent(QResizeEvent* event)
{
	mUpdateBufferImages = true;
	QWidget::resizeEvent(event);
	emit sizeChanged(event->size());
}
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

QcItem::QcItem(QcGaugeWidget* ParentWidget)
	: mGaugeWidget(ParentWidget),
	  mPosition(50)
{

}


int QcItem::type()
{
    return 50;
}

void QcItem::update()
{
    mGaugeWidget->update();
}

double QcItem::position() const
{
    return mPosition;
}

QRectF QcItem::widgetRect() const
{
    QRectF Rect(mGaugeWidget->contentsRect().topLeft(), mGaugeWidget->contentsRect().bottomRight());
    int Diameter = qMin(Rect.width(), Rect.height());
    Rect.setSize(QSize(Diameter, Diameter));
    return Rect;
}


QRectF QcItem::itemRect() const
{
	return adjustRect(position());
}


void QcItem::setPosition(double position)
{
    if(position>100)
        mPosition = 100;
    else if(position<0)
        mPosition = 0;
    else
        mPosition = position;
    update();
}


QRectF QcItem::adjustRect(double percentage) const
{
	QRectF Rect = widgetRect().adjusted(2, 2, -2, -2);
    double r = getRadius(Rect);
    double offset =   r - (percentage * r) / 100.0;
    QRectF Result = Rect.adjusted(offset, offset, -offset, -offset);
    return Result;
}

double QcItem::getRadius(const QRectF &tmpRect)
{
    double Diameter = qMin(tmpRect.width(), tmpRect.height());
    return Diameter / 2.0;
}


QPointF QcItem::getPoint(double deg,const QRectF &tmpRect)
{
    double r = getRadius(tmpRect);
    double xx=cos(qDegreesToRadians(deg))*r;
    double yy=sin(qDegreesToRadians(deg))*r;
    QPointF pt;
    xx=tmpRect.center().x()-xx;
    yy=tmpRect.center().y()-yy;
    pt.setX(xx);
    pt.setY(yy);
    return pt;
}



double QcItem::getAngle(const QPointF&pt, const QRectF &tmpRect)
{
    double xx = tmpRect.center().x()-pt.x();
    double yy = tmpRect.center().y()-pt.y();
    return qRadiansToDegrees( atan2(yy,xx));
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

QcScaleItem::QcScaleItem(QcGaugeWidget* ParentWidget) :
    QcItem(ParentWidget),
    mMinValue(0),
    mMaxValue(100),
    mMinDegree(-45),
    mMaxDegree(225)
{

}

void QcScaleItem::setRange(double minValue, double maxValue)
{
    if(!(minValue<maxValue))
        throw( InvalidValueRange);
    mMinValue = minValue;
    mMaxValue = maxValue;
    mGaugeWidget->invalidateBufferImages();
    update();
}

void QcScaleItem::setDegreeRange(double minDegree, double maxDegree)
{
    if(!(minDegree<maxDegree))
        throw( InvalidValueRange);
    mMinDegree = minDegree;
    mMaxDegree = maxDegree;
}

double QcScaleItem::getDegFromValue(double v) const
{
    double a = (mMaxDegree-mMinDegree)/(mMaxValue-mMinValue);
    double b = -a*mMinValue+mMinDegree;
    return a*v+b;
}


void QcScaleItem::setMinimumValue(double minValue)
{
    if(minValue>mMaxValue)
        throw (InvalidValueRange);
    mMinValue = minValue;
    mGaugeWidget->invalidateBufferImages();
    update();
}


void QcScaleItem::setMaximumValue(double maxValue)
{
    if(maxValue<mMinValue )
        throw (InvalidValueRange);
    mMaxValue = maxValue;
    mGaugeWidget->invalidateBufferImages();
    update();
}

void QcScaleItem::setMinimumDegree(double minDegree)
{
    if(minDegree>mMaxDegree)
        throw (InvalidDegreeRange);
    mMinDegree = minDegree;
    update();
}
void QcScaleItem::setMaximumDegree(double maxDegree)
{
    if(maxDegree<mMinDegree)
        throw (InvalidDegreeRange);
    mMaxDegree = maxDegree;
    update();
}


double QcScaleItem::maximumDegree() const
{
	return mMaxDegree;
}


double QcScaleItem::minimumDegree() const
{
	return mMinDegree;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

QcBackgroundItem::QcBackgroundItem(QcGaugeWidget* ParentWidget) :
    QcItem(ParentWidget),
    mBrush(Qt::darkGray)
{
    setPosition(88);
    mPen = Qt::NoPen;
    setPosition(88);

    addColor(0.4,Qt::darkGray);
    addColor(0.8,Qt::black);
}


QcBackgroundItem::~QcBackgroundItem()
{

}


void QcBackgroundItem::draw(QPainter* painter)
{
    QRectF tmpRect = widgetRect();

    if (mDropShadow)
    {
		QPointF ShadowOffset = mGaugeWidget->shadowOffset();
		painter->save();
		painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
		painter->translate(-(mDropShadowImage.width() / 2.0) + ShadowOffset.x(),
			-(mDropShadowImage.height() / 2.0) +  ShadowOffset.y());
		painter->drawImage(itemRect().center(), mDropShadowImage);
		painter->restore();
    }

    if (!mColors.empty())
    {
		QLinearGradient linearGrad(tmpRect.topLeft(), tmpRect.bottomRight());
		for(int i = 0;i<mColors.size();i++){
			linearGrad.setColorAt(mColors[i].first,mColors[i].second);
		}
		painter->setBrush(linearGrad);
    }
    else
    {
    	painter->setBrush(mBrush);
    }

    painter->setPen(mPen);
    painter->drawEllipse(itemRect());
}

void QcBackgroundItem::addColor(double position, const QColor &color)
{
    if(position<0||position>1)
        return;
      QPair<double,QColor> pair;
      pair.first = position;
      pair.second = color;
      mColors.append(pair);
      update();
}

void QcBackgroundItem::clearColors()
{
    mColors.clear();
}


void QcBackgroundItem::setDropShadow(bool DropShadow)
{
	mDropShadow = DropShadow;
	if (DropShadow)
	{
		updateDropShadowImage();
		connect(mGaugeWidget, SIGNAL(sizeChanged(const QSize&)), this,
			SLOT(onWidgetSizeChanged(const QSize&)));
	}
	else
	{
		mDropShadowImage = QImage();
		disconnect(mGaugeWidget, SIGNAL(sizeChanged(const QSize&)), this,
			SLOT(onWidgetSizeChanged(const QSize&)));
	}
}


bool QcBackgroundItem::dropShadow() const
{
	return mDropShadow;
}


void QcBackgroundItem::onWidgetSizeChanged(const QSize& Size)
{
	updateDropShadowImage();
}


void QcBackgroundItem::updateDropShadowImage()
{
	if (!mDropShadow)
	{
		return;
	}

	QRectF tmpRect = itemRect();
	tmpRect.moveTopLeft(QPointF(0, 0));
	QImage ShadowImage(tmpRect.size().toSize(), QImage::Format_ARGB32_Premultiplied);
    ShadowImage.fill(Qt::transparent);
    {
    	QPainter Painter(&ShadowImage);
		Painter.setRenderHint(QPainter::Antialiasing);
		Painter.setBrush(QColor(0, 0, 0, 160));
		Painter.setPen(Qt::NoPen);
		Painter.drawEllipse(tmpRect);
    }
    mDropShadowImage = mGaugeWidget->blurShadowImage(ShadowImage);
}


void QcBackgroundItem::setBrush(const QBrush& Brush)
{
	mBrush = Brush;
	update();
}

const QBrush& QcBackgroundItem::brush() const
{
	return mBrush;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

QcGlassItem::QcGlassItem(QcGaugeWidget* ParentWidget) :
    QcItem(ParentWidget),
    mGlassType(CurvedGlass2)
{
    setPosition(88);
}

void QcGlassItem::draw(QPainter *painter)
{
    QColor clr1(Qt::white);
    QLinearGradient linearGrad1;
    linearGrad1.setCoordinateMode(QGradient::ObjectBoundingMode);
    linearGrad1.setStart(0,0);
    linearGrad1.setFinalStop(0,1);
    clr1.setAlphaF(0.7);
    linearGrad1.setColorAt(0, clr1);
    clr1.setAlphaF(0.35);
    linearGrad1.setColorAt(0.4, clr1);
    clr1.setAlphaF(0.15);
    linearGrad1.setColorAt(1, clr1);

	QPainterPath Path;
	QBrush Brush(linearGrad1);
	switch (mGlassType)
	{
		case StronglyCurvedGlass: createStronglyCurvedGlass(Path, Brush); break;
		case CurvedGlass1: createCurvedGlass1(Path, Brush); break;
		case CurvedGlass2: createCurvedGlass2(Path, Brush); break;
		case FlatGlass: createFlatGlass(Path, Brush); break;
		case FlatGlass2: createFlatGlass2(Path, Brush); break;
		default:
			createStronglyCurvedGlass(Path, Brush); break;
	}
    painter->setBrush(Brush);
    painter->setPen(Qt::NoPen);
    painter->drawPath(Path);
}


void QcGlassItem::setGlassType(GlassType glassType)
{
	mGlassType = glassType;
}


void QcGlassItem::createStronglyCurvedGlass(QPainterPath& Path,
	QBrush& Brush)
{
    static const double Angle = 2;

	QRectF tmpRect1 = itemRect();
    QPointF Start = getPoint(Angle, tmpRect1);
    QPointF End = getPoint(180 - Angle, tmpRect1);
    Path.moveTo(End);
    Path.arcTo(tmpRect1, Angle, 180 - 2 * Angle);
	QRectF tmpRect2;
	tmpRect2.setWidth(qAbs(End.x() - Start.x()));
	tmpRect2.setHeight(tmpRect1.height() / 5);
	tmpRect2.moveCenter(QPointF(tmpRect1.center().x(), Start.y()));
	Path.arcTo(tmpRect2, 180, 180);
}


void QcGlassItem::createCurvedGlass1(QPainterPath& Path,
	QBrush& Brush)
{
    static const double Angle = 0;

	QRectF tmpRect1 = itemRect();
    QPointF Start = getPoint(Angle, tmpRect1);
    QPointF End = getPoint(180 - Angle, tmpRect1);
    Path.moveTo(End);
    Path.arcTo(tmpRect1, Angle, 180 - 2 * Angle);
    Path.quadTo(QPointF(tmpRect1.center().x(), Start.y() + tmpRect1.height() / 7), End);
}


void QcGlassItem::createCurvedGlass2(QPainterPath& Path,
	QBrush& Brush)
{
    static const double Angle = 5;

	QRectF tmpRect1 = itemRect();
    QPointF Start = getPoint(Angle, tmpRect1);
    QPointF End = getPoint(180 - Angle, tmpRect1);
    Path.moveTo(End);
    Path.arcTo(tmpRect1, Angle, 180 - 2 * Angle);
    Path.quadTo(QPointF(tmpRect1.center().x(), Start.y() - tmpRect1.height() / 7), End);
}


void QcGlassItem::createFlatGlass(QPainterPath& Path,
	QBrush& Brush)
{
	QRectF tmpRect1 = itemRect();
    QColor clr1 = Qt::white;
    QColor clr2 = Qt::white;
    clr1.setAlphaF(0.6);
    clr2.setAlphaF(0.1);

    QLinearGradient linearGrad1(QPointF(0, tmpRect1.top()), QPointF(0, tmpRect1.bottom()));
    linearGrad1.setColorAt(0, clr1);
    linearGrad1.setColorAt(1, clr2);
    Brush = QBrush(linearGrad1);

    QPointF End = getPoint(100, tmpRect1);
    Path.moveTo(End);
    Path.arcTo(tmpRect1, 80, 160);
}


void QcGlassItem::createFlatGlass2(QPainterPath& Path, QBrush& Brush)
{
    static const double Angle = 10;
    QRectF tmpRect1 = itemRect();

    QColor clr1(Qt::white);
    QLinearGradient linearGrad1;
    linearGrad1.setCoordinateMode(QGradient::ObjectBoundingMode);
    linearGrad1.setStart(0,0);
    linearGrad1.setFinalStop(0, 0.7);
    clr1.setAlphaF(0.7);
    linearGrad1.setColorAt(0, clr1);
    clr1.setAlphaF(0.3);
    linearGrad1.setColorAt(0.4, clr1);
    clr1.setAlphaF(0.2);
    linearGrad1.setColorAt(1, clr1);
    Brush = QBrush(linearGrad1);

    QPointF Start = getPoint(Angle, tmpRect1);
    QPointF End = getPoint(180 - Angle, tmpRect1);
    Path.moveTo(End);
    Path.arcTo(tmpRect1, Angle, 180 - 2 * Angle);
    Path.cubicTo(QPointF(tmpRect1.center().x(), tmpRect1.center().y() + tmpRect1.height() / 12 ),
    	QPointF(tmpRect1.center().x() + tmpRect1.width() / 8, End.y() - tmpRect1.height() / 4),
    	QPointF(End.x(), Start.y()));
}


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

QcLabelItem::QcLabelItem(QcGaugeWidget* ParentWidget) :
    QcItem(ParentWidget),
    mAngle(270),
    mText("%"),
    mColor(Qt::black),
    mScaleFactor(1)
{
    setPosition(50);
}

void QcLabelItem::draw(QPainter *painter)
{
    QRectF tmpRect = itemRect();
    double r = getRadius(widgetRect());
    mFont.setPointSizeF(r / 10.0 * mScaleFactor);
    painter->setFont(mFont);
    painter->setPen(QPen(mColor));

    QPointF txtCenter = getPoint(mAngle,tmpRect);
    QFontMetrics fMetrics = painter->fontMetrics();
    QSize sz = fMetrics.size( Qt::TextSingleLine, mText );
    QRectF txtRect(QPointF(0,0), sz );
    txtRect.moveCenter(txtCenter);

    painter->drawText( txtRect, Qt::TextSingleLine, mText);

}

void QcLabelItem::setAngle(double a)
{
    mAngle = a;
    update();
}

double QcLabelItem::angle() const
{
    return mAngle;
}

void QcLabelItem::setText(const QString &text, bool repaint)
{
    mText = text;
    if(repaint)
        update();
}

const QString& QcLabelItem::text() const
{
    return mText;
}

void QcLabelItem::setColor(const QColor &color)
{
    mColor = color;
    update();
}

const QColor& QcLabelItem::color() const
{
    return mColor;
}

void QcLabelItem::setFont(const QFont& font)
{
	mFont = font;
}


const QFont& QcLabelItem::font() const
{
	return mFont;
}


void QcLabelItem::setScaleFactor(float Factor)
{
	mScaleFactor = Factor;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

QcArcItem::QcArcItem(QcGaugeWidget* ParentWidget) :
    QcScaleItem(ParentWidget),
    mColor(Qt::black)
{
    setPosition(80);
}

void QcArcItem::draw(QPainter *painter)
{
    QRectF tmpRect= itemRect();
    double r = getRadius(tmpRect);

    QPen pen;
    pen.setColor(mColor);
    pen.setWidthF(qMax(r/40.0, 1.0));
    painter->setPen(pen);
    painter->drawArc(tmpRect,-16*(mMinDegree+180),-16*(mMaxDegree-mMinDegree));
}

void QcArcItem::setColor(const QColor &color)
{
    mColor = color;
}
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

QcColorBand::QcColorBand(QcGaugeWidget* ParentWidget) :
    QcScaleItem(ParentWidget),
    mBandStartValue(0)
{
    QColor tmpColor;
    tmpColor.setAlphaF(0.1);
    QPair<QColor,double> pair;

    pair.first = Qt::green;
    pair.second = 10;
    mBandColors.append(pair);

    pair.first = Qt::darkGreen;
    pair.second = 50;
    mBandColors.append(pair);

    pair.first = Qt::red;
    pair.second = 100;
    mBandColors.append(pair);

    setPosition(50);
}

QPainterPath QcColorBand::createSubBand(double from, double sweep)
{
    QRectF tmpRect = itemRect();
    QPainterPath path;
    path.arcMoveTo(tmpRect,180+from);
    path.arcTo(tmpRect,180+from,-sweep);
    return path;
}

void QcColorBand::draw(QPainter *painter)
{
    double r = getRadius(widgetRect());
    QPen pen;
    pen.setCapStyle(Qt::FlatCap);
    pen.setWidthF(r/20.0 * mPenWidthScaleFactor);
    painter->setBrush(Qt::NoBrush);
    double offset = getDegFromValue(mBandStartValue);
    for(int i = 0;i<mBandColors.size();i++){
        QColor clr = mBandColors[i].first;
        double sweep;
        if(i==0)
            sweep = getDegFromValue(mBandColors[i].second)-getDegFromValue(mMinValue);
        else
            sweep = getDegFromValue(mBandColors[i].second)-getDegFromValue(mBandColors[i-1].second);
        QPainterPath path = createSubBand(-offset,sweep);
        offset += sweep;
        pen.setColor(clr);
        painter->setPen(pen);
        painter->drawPath(path);
    }
}
void QcColorBand::setColors(const QList<QPair<QColor, double> > &colors)
{
    mBandColors = colors;
    update();
}

void QcColorBand::setPenWidthScaleFactor(float Factor)
{
	mPenWidthScaleFactor = Factor;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

QcDegreesItem::QcDegreesItem(QcGaugeWidget* ParentWidget) :
    QcScaleItem(ParentWidget),
    mStep(10),
    mColor(Qt::black),
    mSubDegree(false)
{
    setPosition(90);
}


void QcDegreesItem::draw(QPainter *painter)
{
    QRectF tmpRect = itemRect();

    painter->setPen(mColor);
    double r = getRadius(tmpRect);
    for(double val = mMinValue;val<=mMaxValue;val+=mStep){
        double deg = getDegFromValue(val);
        QPointF pt = getPoint(deg,tmpRect);
        QPainterPath path;
        path.moveTo(pt);
        path.lineTo(tmpRect.center());
        pt = path.pointAtPercent(0.03);
        QPointF newPt = path.pointAtPercent(mSubDegree ? 0.09 : 0.13);

        QPen pen;
        pen.setColor(mColor);
        pen.setWidthF(mSubDegree ? qMax(r/75.0, 0.5) : qMax(r/25.0, 0.75));
        painter->setPen(pen);
        painter->drawLine(pt,newPt);

    }
}

void QcDegreesItem::setStep(double step)
{
    mStep = step;
    update();
}

void QcDegreesItem::setColor(const QColor& color)
{
    mColor = color;
    update();
}

void QcDegreesItem::setSubDegree(bool b)
{
    mSubDegree = b;
    update();
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

QcNeedleItem::QcNeedleItem(QcGaugeWidget* ParentWidget) :
	QObject(0),
    QcScaleItem(ParentWidget),
    mCurrentValue(0),
    mNeedleType(FeatherNeedle),
    mLabel(0),
    mBrush(Qt::black),
    mDropShadow(false),
    mThicknessFactor(1),
    mDecimals(1)
{
	connect(ParentWidget, SIGNAL(sizeChanged(const QSize&)), this,
		SLOT(onWidgetSizeChanged(const QSize&)));
}

void QcNeedleItem::draw(QPainter *painter)
{
	if (mLabel)
	{
		mLabel->draw(painter);
	}

    QRectF tmpRect = itemRect();
    painter->save();
    painter->translate(tmpRect.center());
    double deg = getDegFromValue( mCurrentValue);
    painter->setPen(Qt::NoPen);

    double Radius = getRadius(tmpRect);
    QPolygonF NeedlePoly = createNeedlePoly(Radius);
    QBrush Brush(mBrush);

    // draw the shadow
    if (mDropShadow)
    {
		int yOffset = round((mDropShadowImage.height() - NeedlePoly.boundingRect().height()) / 2.0 - NeedlePoly.boundingRect().top());
		painter->save();
		painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
		painter->translate(mGaugeWidget->shadowOffset());
		painter->rotate(deg + 90.0);
		painter->translate(QPointF(0, -yOffset));
		painter->drawImage(QPointF(-(mDropShadowImage.width() / 2), 0), mDropShadowImage);
		painter->restore();
    }

    // Draw the needle
    painter->setBrush(Brush);
    painter->rotate(deg + 90.0);
    painter->drawConvexPolygon(NeedlePoly);
    painter->restore();
}


void QcNeedleItem::setDropShadow(bool DropShadow)
{
	mDropShadow = DropShadow;
	if (!DropShadow)
	{
		mDropShadowImage = QImage();
	}
	else
	{
		updateDropShadowImage();
	}
}


bool QcNeedleItem::dropShadow() const
{
	return mDropShadow;
}


void QcNeedleItem::updateDropShadowImage()
{
	if (!mDropShadow)
	{
		return;
	}

    QRectF tmpRect = itemRect();
	double Radius = getRadius(tmpRect);
	QPolygonF NeedlePoly = createNeedlePoly(Radius);
    QImage ShadowImage(NeedlePoly.boundingRect().size().toSize(), QImage::Format_ARGB32_Premultiplied);
    ShadowImage.fill(Qt::transparent);
    {
    	QPainter Painter(&ShadowImage);
		Painter.setRenderHint(QPainter::Antialiasing);
		Painter.setBrush(mGaugeWidget->shadowBrush());
		Painter.setPen(Qt::NoPen);
		Painter.translate(NeedlePoly.boundingRect().width() / 2, - NeedlePoly.boundingRect().top());
		Painter.drawConvexPolygon(NeedlePoly);
    }
    mDropShadowImage = mGaugeWidget->blurShadowImage(ShadowImage);
}


void QcNeedleItem::onWidgetSizeChanged(const QSize& Size)
{
	updateDropShadowImage();
}


void QcNeedleItem::setDecimals(int Decimals)
{
	mDecimals = Decimals;
}


int QcNeedleItem::decimals() const
{
	return mDecimals;
}


void QcNeedleItem::setValue(double value)
{
    if(value<mMinValue)
        mCurrentValue = mMinValue;
    else if(value>mMaxValue)
        mCurrentValue = mMaxValue;
    else
        mCurrentValue = value;
    if(mLabel!=0)
        mLabel->setText(QString::number(mCurrentValue, 'f', mDecimals),false);
    update();
}

double QcNeedleItem::value() const
{
    return mCurrentValue;
}


void QcNeedleItem::setValueRange(double minValue,double maxValue)
{
	QcScaleItem::setRange(minValue, maxValue);
}

void QcNeedleItem::setMinimumValue(double minValue)
{
	QcScaleItem::setMinimumValue(minValue);
}


void QcNeedleItem::setMaximumValue(double maxValue)
{
	QcScaleItem::setMaximumValue(maxValue);
}

void QcNeedleItem::setColor(const QColor &color)
{
    mBrush.setColor(color);
    update();
}

const QColor& QcNeedleItem::color() const
{
    return mBrush.color();
}


void QcNeedleItem::setBrush(const QBrush& Brush)
{
	mBrush = Brush;
}

const QBrush& QcNeedleItem::brush() const
{
	return mBrush;
}

void QcNeedleItem::setLabel(QcLabelItem *label)
{
    mLabel = label;
    mGaugeWidget->removeItem(label);
    update();
}

QcLabelItem *QcNeedleItem::label() const
{
    return mLabel;
}


void QcNeedleItem::setNeedle(QcNeedleItem::NeedleType needleType)
{
    mNeedleType = needleType;
    if (CompassNeedle == needleType)
    {
    	QLinearGradient grad;
    	grad.setCoordinateMode(QGradient::ObjectBoundingMode);
    	grad.setFinalStop(QPointF(0, 1));
    	grad.setColorAt(0.0,Qt::red);
        grad.setColorAt(0.49,Qt::red);
        grad.setColorAt(0.5,Qt::blue);
        grad.setColorAt(1,Qt::blue);
        mBrush = QBrush(grad);
    }
    update();
}


void QcNeedleItem::setCustomNeedle(const QVector<QPointF>& NeedlePoly)
{
	mNeedleType = CustomNeedle;
	mCustomNeedlePoly = NeedlePoly;
    update();
}


void QcNeedleItem::setThicknessFactor(float Value)
{
	mThicknessFactor = Value;
}


QPolygonF QcNeedleItem::createDiamonNeedle(double r) const
{
    QVector<QPointF> tmpPoints;
    tmpPoints.append(QPointF(0.0, 0.0));
    tmpPoints.append(QPointF(-r/20.0 * mThicknessFactor, r/20.0));
    tmpPoints.append(QPointF(0.0, r));
    tmpPoints.append(QPointF(r/20.0 * mThicknessFactor, r/20.0));
    return tmpPoints;
}

QPolygonF QcNeedleItem::createTriangleNeedle(double r) const
{
    QVector<QPointF> tmpPoints;
    tmpPoints.append(QPointF(0.0, r));
    tmpPoints.append(QPointF(-r/40 * mThicknessFactor, 0.0));
    tmpPoints.append(QPointF(r/40 * mThicknessFactor,0.0));
    return tmpPoints;
}

QPolygonF QcNeedleItem::createFeatherNeedle(double r) const
{
    QVector<QPointF> tmpPoints;
    tmpPoints.append(QPointF(0.0, r));
    tmpPoints.append(QPointF(-r/40.0 * mThicknessFactor, 0.0));
    tmpPoints.append(QPointF(-r/15.0 * mThicknessFactor, -r/5.0));
    tmpPoints.append(QPointF(r/15.0 * mThicknessFactor, -r/5));
    tmpPoints.append(QPointF(r/40.0 * mThicknessFactor,0.0));
    return tmpPoints;
}

QPolygonF QcNeedleItem::createAttitudeNeedle(double r) const
{
    QVector<QPointF> tmpPoints;
    tmpPoints.append(QPointF(0.0, r));
    tmpPoints.append(QPointF(-r/20.0, 0.85*r));
    tmpPoints.append(QPointF(r/20.0,0.85*r));
    return tmpPoints;
}

QPolygonF QcNeedleItem::createCompassNeedle(double r) const
{
    QVector<QPointF> tmpPoints;
    tmpPoints.append(QPointF(0.0, r));
    tmpPoints.append(QPointF(-r/15.0, 0.0));
    tmpPoints.append(QPointF(0.0, -r));
    tmpPoints.append(QPointF(r/15.0,0.0));
    return tmpPoints;
}


QPolygonF QcNeedleItem::createCustomNeedle(double r) const
{
	QVector<QPointF> tmpPoints(mCustomNeedlePoly);
	for (int i = 0; i < tmpPoints.size(); ++i)
	{
		tmpPoints[i] *= r;
	}

	return tmpPoints;
}


QPolygonF QcNeedleItem::createNeedlePoly(double Radius) const
{
    switch (mNeedleType)
    {
    case QcNeedleItem::FeatherNeedle: return createFeatherNeedle(Radius);
    case QcNeedleItem::DiamonNeedle: return createDiamonNeedle(Radius);
    case QcNeedleItem::TriangleNeedle: return createTriangleNeedle(Radius);
    case QcNeedleItem::AltitudeMeterNeedle: return createAttitudeNeedle(Radius);
    case QcNeedleItem::CompassNeedle: return createCompassNeedle(Radius);
    case QcNeedleItem::CustomNeedle: return createCustomNeedle(Radius);
    default:
        return createTriangleNeedle(Radius);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

QcValuesItem::QcValuesItem(QcGaugeWidget* ParentWidget) :
    QcScaleItem(ParentWidget),
    mStep(10),
    mColor(Qt::black),
    mScaleFactor(1),
    mDecimals(1)
{
    setPosition(70);
}


void QcValuesItem::draw(QPainter*painter)
{
    QRectF tmpRect = widgetRect();
    double r = getRadius(adjustRect(99));
    mFont.setPointSizeF(0.08 * r * mScaleFactor);

    painter->setFont(mFont);
    painter->setPen(mColor);
    for(double val = mMinValue;val<=mMaxValue;val+=mStep){
        double deg = getDegFromValue(val);
        QPointF pt = getPoint(deg,tmpRect);
        QPainterPath path;
        path.moveTo(pt);
        path.lineTo(    tmpRect.center());
        QString strVal = QString::number(val, 'f', mDecimals);
        QFontMetrics fMetrics = painter->fontMetrics();
        QSize sz = fMetrics.size( Qt::TextSingleLine, strVal );
        QRectF txtRect(QPointF(0,0), sz );
        QPointF textCenter = path.pointAtPercent(1.0-position()/100.0);
        txtRect.moveCenter(textCenter);

        painter->drawText( txtRect, Qt::TextSingleLine, strVal );
    }
}


void QcValuesItem::setDecimals(int Value)
{
	mDecimals = Value;
}

void QcValuesItem::setStep(double step)
{
    mStep = step;
}


void QcValuesItem::setColor(const QColor& color)
{
    mColor = color;
}

void QcValuesItem::setFont(const QFont& font)
{
	mFont = font;
	update();
}


const QFont& QcValuesItem::font() const
{
	return mFont;
}

void QcValuesItem::setScaleFactor(float ScaleFactor)
{
	mScaleFactor = ScaleFactor;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

QcAltitudeMeter::QcAltitudeMeter(QcGaugeWidget* ParentWidget) :
    QcItem(ParentWidget),
    mRoll(0),
    mPitch(0)
{

}

void QcAltitudeMeter::setCurrentPitch(double pitch)
{
    mPitch=-pitch;
    update();
}

void QcAltitudeMeter::setCurrentRoll(double roll)
{
    mRoll = roll;
    update();
}

QPointF QcAltitudeMeter::intersection(double r, const QPointF &pitchPoint, const QPointF &pt) const
{
    // refrence it to zero

    Q_UNUSED(r)
    double a = (pitchPoint.y()-pt.y())/(pitchPoint.x()-pt.x());
    double b = pt.y()-a*pt.x();
    return QPointF(0,a*0+b);
}

double QcAltitudeMeter::startAngle(const QRectF& tmpRect) const
{
    double r = getRadius(tmpRect);
    QPointF pt1 = getPoint(mRoll,tmpRect);
    pt1.setY(pt1.y()-mPitchOffset);
    QPointF pitchPoint = QPointF(tmpRect.center().x(),tmpRect.center().y()-mPitchOffset);


    ///////////////////////////////////////
    QPainterPath path1;
    path1.moveTo(pitchPoint);
    path1.lineTo(intersection(r,pitchPoint,pt1)+QPointF(0,5));
    path1.lineTo(intersection(r,pitchPoint,pt1)+QPointF(0,-5));

    QPainterPath path2;
    path2.addEllipse(tmpRect);

    QPointF p = path1.intersected(path2).pointAtPercent(.5);
    return getAngle(p,tmpRect);
}

void QcAltitudeMeter::draw(QPainter *painter)
{
    QRectF tmpRect = itemRect();
    double r = getRadius(tmpRect);
    if(mPitch<0)
        mPitchOffset = 0.0135*r*mPitch;
    else
        mPitchOffset = 0.015*r*mPitch;

    painter->setPen(Qt::NoPen);
    drawUpperEllipse(painter,tmpRect);
    drawLowerEllipse(painter,tmpRect);

    // Steps

    drawPitchSteps(painter,tmpRect);
    drawHandle(painter);

    drawDegrees(painter);
}

void QcAltitudeMeter::drawDegrees(QPainter *painter)
{
    QRectF tmpRect = itemRect();
    double r = getRadius(tmpRect);
    QPen pen;

    pen.setColor(Qt::white);
    painter->setPen(pen);
    for(int deg = 60;deg<=120;deg+=10){
        if(deg == 90)
            continue;
        drawDegree(painter,tmpRect,deg);
    }

    pen.setWidthF(r/30.0);
    painter->setPen(pen);
    drawDegree(painter,tmpRect,0);
    drawDegree(painter,tmpRect,90);
    drawDegree(painter,tmpRect,180);
    drawDegree(painter,tmpRect,30);
    drawDegree(painter,tmpRect,150);
}


void QcAltitudeMeter::drawDegree(QPainter * painter, const QRectF& tmpRect,double deg)
{
    QPointF pt1 = getPoint(deg,tmpRect);
    QPointF pt2 = tmpRect.center();
    QPainterPath path;
    path.moveTo(pt1);
    path.lineTo(pt2);
    QPointF pt = path.pointAtPercent(0.1);
    painter->drawLine(pt1,pt);
}


void QcAltitudeMeter::drawUpperEllipse(QPainter *painter, const QRectF &tmpRect)
{

    QLinearGradient radialGrad1(tmpRect.topLeft(),tmpRect.bottomRight());
    QColor clr1 = Qt::blue;
    clr1.setAlphaF(0.5);
    QColor clr2 = Qt::darkBlue;
    clr2.setAlphaF(0.5);
    radialGrad1.setColorAt(0, clr1);
    radialGrad1.setColorAt(.8, clr2);


    double offset = startAngle(tmpRect);
    double startAngle = 180-offset;
    double endAngle = offset-2*mRoll;
    double span =endAngle-startAngle;

    painter->setBrush(radialGrad1);
    painter->drawChord(tmpRect,16*startAngle,16*span);

}


void QcAltitudeMeter::drawLowerEllipse(QPainter *painter, const QRectF &tmpRect)
{
    QLinearGradient radialGrad2(tmpRect.topLeft(),tmpRect.bottomRight());
    QColor clr1 = QColor(139,119,118);
    QColor clr2 = QColor(139,119,101);
    radialGrad2.setColorAt(0, clr1);
    radialGrad2.setColorAt(.8, clr2);

    double offset = startAngle(tmpRect);
    double startAngle = 180+offset;
    double endAngle = offset-2*mRoll;
    double span =endAngle+startAngle;

    painter->setPen(Qt::NoPen);
    painter->setBrush(radialGrad2);
    painter->drawChord(tmpRect,-16*startAngle,16*span);

}

void QcAltitudeMeter::drawPitchSteps(QPainter *painter, const QRectF &tmpRect)
{
    double r = getRadius(tmpRect);
    QPointF center = tmpRect.center();
    painter->save();
    painter->translate(center.x(),center.y()-mPitchOffset);
    painter->rotate(mRoll);
    QPen pen;
    pen.setColor(Qt::white);
    pen.setWidthF(r/40.0);

    painter->setPen(pen);
    for (int i = -30;i<=30;i+=10){
        QPointF pt1;
        pt1.setX(-0.01*r*abs(i));
        pt1.setY(r/70.0*i);
        QPointF pt2;
        pt2.setX(0.01*r*abs(i));
        pt2.setY(r/70.0*i);
        painter->drawLine(pt1,pt2);

        if(i==0)
            continue;

        // draw value
        QFont font("Meiryo UI",0, QFont::Bold);
        font.setPointSizeF(0.08*r);
        painter->setFont(font);
        QString strVal = QString::number(abs(i));
        QFontMetrics fMetrics = painter->fontMetrics();
        QSize sz = fMetrics.size( Qt::TextSingleLine, strVal );
        QRectF leftTxtRect(QPointF(0,0), sz );
        QRectF rightTxtRect(QPointF(0,0), sz );
        leftTxtRect.moveCenter(pt1-QPointF(0.1*r,0));
        rightTxtRect.moveCenter(pt2+QPointF(0.1*r,0));
        painter->drawText( leftTxtRect, Qt::TextSingleLine, strVal );
        painter->drawText( rightTxtRect, Qt::TextSingleLine, strVal );
    }
    painter->restore();
}

void QcAltitudeMeter::drawHandle(QPainter *painter)
{
    QRectF tmpRct = adjustRect(15);
    double r = getRadius(tmpRct);
    QPen pen;
    pen.setColor(Qt::gray);
    pen.setWidthF(0.25*r);
    painter->setPen(pen);
    painter->drawArc(tmpRct,0,-16*180);

    QPointF center = tmpRct.center();
    QPointF leftPt1 = center;
    QPointF leftPt2 = center;
    QPointF rightPt1 = center;
    QPointF rightPt2 = center;
    leftPt1.setX(center.x()-2*r);
    leftPt2.setX(center.x()-r);
    rightPt1.setX(center.x()+2*r);
    rightPt2.setX(center.x()+r);
    painter->drawLine(leftPt1,leftPt2);
    painter->drawLine(rightPt1,rightPt2);
    painter->drawEllipse(adjustRect(2));

    //
    QPointF pt1 = center;
    QPointF pt2 = center;
    // to get the real 100 % radius, without recomputing
    pt1.setY(center.y()+r);
    pt2.setY(center.y()+4*r);
    pen.setColor(Qt::gray);
    painter->setPen(pen);
    painter->drawLine(pt1,pt2);

    // trapezium
    painter->setPen(Qt::gray);
    painter->setBrush(Qt::gray);
    QPolygonF trapPoly;
    QPointF tmpPt = center;
    tmpPt.setX(center.x()-r);
    tmpPt.setY(center.y()+4*r);
    trapPoly.append(tmpPt);
    tmpRct = itemRect();
    trapPoly.append(getPoint(290,tmpRct));
    trapPoly.append(getPoint(250,tmpRct));
    tmpPt = center;
    tmpPt.setX(center.x()+r);
    tmpPt.setY(center.y()+4*r);
    trapPoly.append(tmpPt);
    painter->drawPolygon(trapPoly);
    painter->drawChord(tmpRct,-16*70,-16*40);
}

